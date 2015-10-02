#pragma once

#ifdef DEBUG
#define LOG(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#else
#define LOG(fmt, ...)
#endif
