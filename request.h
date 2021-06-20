#ifndef __REQUEST_H__

#include "myQueue.h"

void requestHandle(int fd, thread_stats_t* thread_stats, struct timeval arrival_time, struct timeval dispatch_time);

#endif
