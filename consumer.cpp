#include <unistd.h>
#include <iostream>
#include <semaphore.h>

#include "consumer.h"
#include "queue.h"
#include "log.h"
#include "seating.h"

extern RequestQueue requestQueue;
extern int totalRequests; // Set via -s argument or default
extern sem_t barrier;     // Used to release the main thread
extern int tx_sleep_ms;
extern int rev9_sleep_ms;

void* consumer(void* arg) {
    intptr_t raw_value = reinterpret_cast<intptr_t>(arg);
    ConsumerType ctype = static_cast<ConsumerType>(raw_value);    

    int sleep_time = 0;
    if (ctype == TX){
        sleep_time = tx_sleep_ms;
    }
    else if (ctype == Rev9){
        sleep_time = rev9_sleep_ms;
    }

    while (true) {
        pthread_mutex_lock(&requestQueue.mutex);

        // Wait if queue is empty and consuming isn't finished
        bool queue_is_empty = requestQueue.queue.empty();
        bool more_requests_expected = (requestQueue.totalConsumed < totalRequests);

        // Wait for available requests or until all are consumed
        while (queue_is_empty && more_requests_expected) {
            pthread_cond_wait(&requestQueue.not_empty, &requestQueue.mutex);

            queue_is_empty = requestQueue.queue.empty();
            more_requests_expected = (requestQueue.totalConsumed < totalRequests);
        }

        // Exit if no requests remain and everything has been handled
        if (requestQueue.queue.empty() && requestQueue.totalConsumed >= totalRequests) {
            pthread_mutex_unlock(&requestQueue.mutex);
            return nullptr;
        }

        // Consume the next request
        RequestType req = requestQueue.queue.front();
        requestQueue.queue.pop();
        requestQueue.inQueue[req]--;
        requestQueue.consumed[ctype][req]++;
        requestQueue.totalConsumed++;

        output_request_removed(ctype, req, requestQueue.consumed[ctype], requestQueue.inQueue);

        // Signal barrier if this was the final request
        if (requestQueue.totalConsumed >= totalRequests && requestQueue.queue.empty()) {
            sem_post(&barrier);
        }

        pthread_cond_signal(&requestQueue.not_full);
        pthread_mutex_unlock(&requestQueue.mutex);

        if (sleep_time > 0) {
            usleep(sleep_time * 1000); // Convert milliseconds to microseconds
        }
    }
}
