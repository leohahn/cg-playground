#ifndef __MACROS_HPP__
#define __MACROS_HPP__

//
// NOTE, HACK: These macros are ugly, remove them as soon as possible.
//
#define BEGIN_REGION(x)												   \
	lt_local_persist f64 _local_counter_##x = get_time_milliseconds(); \
	lt_local_persist u32 _count_##x = 0;							   \
	lt_local_persist u32 _accum_region_##x = 0;						   \
	const u64 _begin_region_##x = lt::rdtsc()

#define END_REGION(x)													\
	_count_##x++;														\
	_accum_region_##x += lt::rdtsc() - _begin_region_##x;				\
	if ((get_time_milliseconds() - _local_counter_##x) >= 1000.0)		\
	{																	\
		dgui::State::instance().performance_regions[(x)] = _accum_region_##x / _count_##x; \
		_accum_region_##x = 0;											\
		_local_counter_##x = get_time_milliseconds();					\
		_count_##x = 0;													\
	} (void) 0

#endif // __MACROS_HPP__
