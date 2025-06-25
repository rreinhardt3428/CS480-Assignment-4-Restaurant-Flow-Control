#include "queue.h"

RequestQueue requestQueue; // instance of the shared request used by the producers and consumers

bool RequestQueue::isFull(RequestType type) const { // checking if the queue is full based upon certain constraints
    unsigned int total = inQueue[GeneralTable] + inQueue[VIPRoom]; // computing the total number of requests waiting in the queue
    bool atGlobalCapacity = (total >= 18); // checking if the queue has hit the max capacity of 18
    bool vipIsOverLimit = (type == VIPRoom && inQueue[VIPRoom] >= 5); // checking if the vip queue has hit its cap of 5

    return (atGlobalCapacity || vipIsOverLimit);
}
