#include "queue.h"

RequestQueue requestQueue;

bool RequestQueue::is_full(RequestType type) const {
    unsigned int total = inQueue[GeneralTable] + inQueue[VIPRoom];
    return (total >= 18 || (type == VIPRoom && inQueue[VIPRoom] >= 5));
}
