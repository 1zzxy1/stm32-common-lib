#include "MultiTimer.h"
#include <stdio.h>

static MultiTimer* timerList = NULL;
static PlatformTicksFunction_t platformTicksFunction = NULL;

int multiTimerInstall(PlatformTicksFunction_t ticksFunc) {
    if (ticksFunc == NULL) {
        return -1; // Indicate error if ticksFunc is NULL
    }
    platformTicksFunction = ticksFunc;
    return 0;
}

static void removeTimer(MultiTimer* timer) {
    MultiTimer** current = &timerList;
    while (*current) {
        if (*current == timer) {
            *current = timer->next;
            break;
        }
        current = &(*current)->next;
    }
}

int multiTimerStart(MultiTimer* timer, uint64_t timing, MultiTimerCallback_t callback, void* userData) {
    if (!timer || !callback || platformTicksFunction == NULL) {
        return -1; // Return error if any parameter is invalid
    }

    removeTimer(timer); // Centralize removal logic

    timer->deadline = platformTicksFunction() + timing;
    timer->callback = callback;
    timer->userData = userData;

    MultiTimer** current = &timerList;
    while (*current && ((*current)->deadline < timer->deadline)) {
        current = &(*current)->next;
    }
    timer->next = *current;
    *current = timer;

    return 0;
}

int multiTimerStop(MultiTimer* timer) {
    removeTimer(timer); // Use centralized removal function
    return 0;
}

int multiTimerYield(void) {
    if (platformTicksFunction == NULL) {
        return -1; // Indicate error if platformTicksFunction is NULL
    }
    uint64_t currentTicks = platformTicksFunction();
    while (timerList && (currentTicks >= timerList->deadline)) {
        MultiTimer* timer = timerList;
        timerList = timer->next; // Remove expired timer

        if (timer->callback) {
            timer->callback(timer, timer->userData); // Execute callback
        }
    }
    return timerList ? (int)(timerList->deadline - currentTicks) : 0;
}
