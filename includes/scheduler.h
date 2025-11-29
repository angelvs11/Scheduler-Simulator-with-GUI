#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stddef.h>

typedef struct {
    int pid;                    // Process ID
    int arrival_time;           // When process arrives
    int burst_time;             // Total CPU time needed
    int priority;               // Priority (lower = higher priority)
    int remaining_time;         // Time left to execute
    int start_time;             // First time scheduled (-1 if never)
    int completion_time;        // When finished
    int turnaround_time;        // completion - arrival
    int waiting_time;           // turnaround - burst
    int response_time;          // start - arrival
    int finished;               // boolean flag
} process_t;

typedef struct {
    int time;                   // Time slice start
    int pid;                    // Process running (-1 for idle)
    int duration;               // How long it ran
} timeline_event_t;

#endif // SCHEDULER_H

