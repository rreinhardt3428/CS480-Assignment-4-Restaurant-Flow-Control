#ifndef CONSUMER_H
#define CONSUMER_H

#include <pthread.h>

// Thread function for consumer robots (T-X and Rev-9)
void* consumer(void* arg);

#endif // CONSUMER_H
