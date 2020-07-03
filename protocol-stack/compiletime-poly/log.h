#ifndef LOG_H
#define LOG_H

#ifndef BENCHMARK_PERFORMANCE
#include <iostream>
#define LOG(a) std::cout << a << std::endl
#else
#define LOG(a)
#endif

#endif
