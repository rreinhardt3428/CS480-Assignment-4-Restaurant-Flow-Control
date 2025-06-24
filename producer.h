#ifndef PRODUCER_H
#define PRODUCER_H

#include <pthread.h>

// Thread function for producer robots (GeneralTable and VIPRoom)
void* producer(void* arg);

#endif // PRODUCER_H
