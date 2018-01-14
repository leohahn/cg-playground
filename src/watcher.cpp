#include "watcher.hpp"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/inotify.h>
#include <sys/time.h>
#include <sys/types.h>
#include <pthread.h>
#include <memory.h>
#include <linux/limits.h>
#include <errno.h>

#include "lt_core.hpp"
#include "lt_utils.hpp"
#include "gl_resources.hpp"

#ifdef __linux__
#define LEN_NAME PATH_MAX
#else
// @FIXME(leo): This should not be harcoded for other operating systems.
#define LEN_NAME         64
#endif

#define EVENT_SIZE       (sizeof(struct inotify_event))
#define MAX_EVENTS       1024
#define BUF_LEN          (MAX_EVENTS * (EVENT_SIZE + LEN_NAME))
#define EVENT_BUFFER_LEN 50

using std::string;

struct EventBuffer
{
    isize                head;
    isize                tail;
    isize                maxlen;
    WatcherEvent        *events;
};

lt_global_variable lt::Logger logger("watcher");
// Event buffer
lt_global_variable EventBuffer     g_event_buffer = {};
lt_global_variable pthread_mutex_t g_mutex_event_buffer;
// Running state
lt_global_variable bool            g_running      = false;
lt_global_variable pthread_mutex_t g_mutex_running;

lt_internal void
initialize_event_buffer(EventBuffer *eb, isize maxlen)
{
    eb->head = 0;
    eb->tail = 0;
    eb->maxlen = maxlen;
    eb->events = (WatcherEvent*)malloc(sizeof(WatcherEvent) * maxlen);

    for (isize i = 0; i < maxlen; i++)
        eb->events[i].inotify_mask = -1;
}

lt_internal void
free_event_buffer(EventBuffer *eb)
{
    LT_Free(eb->events);
}

lt_internal void
set_running(bool r)
{
    pthread_mutex_lock(&g_mutex_running);
    g_running = r;
    pthread_mutex_unlock(&g_mutex_running);
}

lt_internal void
push_event(EventBuffer *buf, const struct inotify_event *ie)
{
    pthread_mutex_lock(&g_mutex_event_buffer);

    isize next_head = buf->head + 1;
    if (next_head >= buf->maxlen)
        next_head = 0;

    if (next_head == buf->tail)
        LT_Fail("Circular buffer was overrun\n");

    buf->events[buf->head].inotify_mask = ie->mask;
    buf->events[buf->head].name = string(ie->name);
    buf->head = next_head;

    pthread_mutex_unlock(&g_mutex_event_buffer);
}

lt_internal void
consume_event(EventBuffer *buf)
{
    pthread_mutex_lock(&g_mutex_event_buffer);

    LT_Assert(buf->head != buf->tail);

    isize next_tail = buf->tail + 1;
    if (next_tail >= buf->maxlen)
        next_tail = 0;

    buf->events[buf->tail].inotify_mask = -1;
    buf->events[buf->tail].name = "";
    buf->tail = next_tail;

    pthread_mutex_unlock(&g_mutex_event_buffer);
}

WatcherEvent *
watcher_peek_event(void)
{
    pthread_mutex_lock(&g_mutex_event_buffer);

    if (g_event_buffer.head == g_event_buffer.tail)
    {
        pthread_mutex_unlock(&g_mutex_event_buffer);
        return nullptr;
    }

    WatcherEvent *e = &g_event_buffer.events[g_event_buffer.tail];

    pthread_mutex_unlock(&g_mutex_event_buffer);
    return e;
}

void watcher_event_peeked(void) { consume_event(&g_event_buffer); }

void
watcher_stop(void)
{
    logger.log("Stopping watcher.");
    set_running(false);
}

// @ThreadEntry
void *
watcher_start(void *arg)
{
    LT_Unused(arg);

    const isize MAX_NUM_EVENTS = 10;

    // Initialize the mutexes.
    pthread_mutex_init(&g_mutex_running, nullptr);
    pthread_mutex_init(&g_mutex_event_buffer, nullptr);
    // Initialize the event buffer with maximum number of events.
    initialize_event_buffer(&g_event_buffer, MAX_NUM_EVENTS);

    i32 fd = inotify_init1(IN_NONBLOCK);

    if (fd < 0)
    {
        pthread_mutex_destroy(&g_mutex_running);
        pthread_mutex_destroy(&g_mutex_event_buffer);
        LT_Fail("Failed starting inotify.\n");
    }

    i32 wd = inotify_add_watch((i32)fd, RESOURCES_PATH, IN_MODIFY|IN_CREATE|IN_DELETE);

    if (wd < 0)
    {
        pthread_mutex_destroy(&g_mutex_running);
        pthread_mutex_destroy(&g_mutex_event_buffer);
        LT_Fail("Could not add watch to %s\n", RESOURCES_PATH);
    }

    char buf[BUF_LEN] = {};

    set_running(true);

    logger.log("Watching folder ", RESOURCES_PATH);

    fd_set read_fds;

    while (g_running)
    {
        memset(buf, 0, BUF_LEN);

        // NOTE: We need to reset the read set again every loop, since select() modifies it.
        FD_ZERO(&read_fds);
        FD_SET(fd, &read_fds);

        // NOTE: We need to set the timeout every loop, since select() modifies it on linux.
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;

        i32 retval = select(fd+1, &read_fds, nullptr, nullptr, &timeout);

        // We ignore if retval == 0, since it means there was nothing to read.
        if (retval != 0)
        {
            isize len = read(fd, buf, BUF_LEN);

            if (len < 0)
            {
                perror("Error reading for inotify event");
                continue;
            }

            LT_Assert(len < (isize)BUF_LEN);

            isize i = 0;
            while (i < len)
            {
                struct inotify_event *event = (struct inotify_event *)&buf[i];

                if (event->len)
                {
                    // Push event to the circular buffer.
                    push_event(&g_event_buffer, event);
                }

                i += EVENT_SIZE + event->len;
            }
        }
        else if (retval == -1)
        {
            perror("error select(): ");
        }
    }
    logger.log("Finished watching folder ", RESOURCES_PATH);

    // Cleanup resources.
    inotify_rm_watch(fd, wd);
    close(fd);
    free_event_buffer(&g_event_buffer);

    pthread_mutex_destroy(&g_mutex_running);
    pthread_mutex_destroy(&g_mutex_event_buffer);
    pthread_exit(nullptr);
}
