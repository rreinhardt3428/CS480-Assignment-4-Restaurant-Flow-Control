// Ethan Kent CS480
// REDID: 826843661
// Roger Reinhardt
// REDID: 826470808

#include "queue.h"

RequestQueue requestQueue; // instance of the shared request used by the producers and consumers

const int QUEUE_CAPACITY = 18;
const int VIP_LIMIT = 5;

bool RequestQueue::isFull(RequestType type) const { // checking if the queue is full based upon certain constraints
    unsigned int vipCount = inQueue[VIPRoom];
    unsigned int total = inQueue[GeneralTable] + inQueue[VIPRoom]; // computing the total number of requests waiting in the queue


    bool atGlobalCapacity = false;
    if (total >= QUEUE_CAPACITY) {
        atGlobalCapacity = true; // checking if the queue has hit the max capacity of 18
    }

    bool vipIsOverLimit = false;
    if (type == VIPRoom) {
        vipIsOverLimit = (vipCount >= VIP_LIMIT); // checking if the vip queue has hit its cap of 5
    }
    return (atGlobalCapacity || vipIsOverLimit);
}