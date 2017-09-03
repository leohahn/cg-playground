#ifndef WATCHER_H
#define WATCHER_H

#include <string>
#include "lt_core.hpp"

struct WatcherEvent
{
    std::string name;
    i32         inotify_mask;
};

void         *watcher_start(void *arg);
void          watcher_stop(void);
WatcherEvent *watcher_peek_event(void);
void          watcher_event_peeked(void);


#endif // WATCHER_H
