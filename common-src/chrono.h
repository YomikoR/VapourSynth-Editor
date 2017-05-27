#ifndef CHRONO_H_INCLUDED
#define CHRONO_H_INCLUDED

#include <chrono>

typedef std::chrono::high_resolution_clock::time_point hr_time_point;
typedef std::chrono::high_resolution_clock hr_clock;
typedef std::chrono::duration<double, std::ratio<1, 1> > double_duration;

template <typename T> double duration_to_double(const T & a_duration)
{
	return std::chrono::duration_cast<double_duration>(a_duration).count();
}

#endif // CHRONO_H_INCLUDED
