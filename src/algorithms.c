/*
 * algorithms.c
 *
 * Implementations for FIFO, SJF, STCF, RR and MLFQ scheduling.
 *
 * Nota: all algorithms mutate the processes array (set remaining_time, start_time, completion_time, finished)
 * and append timeline events into the provided timeline array. timeline_len is set to number of events appended.
 *
 * This code favors clarity over extreme optimization.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "algorithms.h"

/* helper: reset process bookkeeping */
static void reset_processes(process_t *processes, int n) {
    for (int i = 0; i < n; ++i) {
        processes[i].remaining_time = processes[i].burst_time;
        processes[i].start_time = -1;
        processes[i].completion_time = -1;
        processes[i].turnaround_time = 0;
        processes[i].waiting_time = 0;
        processes[i].response_time = -1;
        processes[i].finished = 0;
    }
}

/* helper: find first arrival time among processes (min arrival) */
static int first_arrival(process_t *processes, int n) {
    int mn = INT_MAX;
    for (int i = 0; i < n; ++i) if (processes[i].arrival_time < mn) mn = processes[i].arrival_time;
    return (mn == INT_MAX) ? 0 : mn;
}

/* Append timeline event */
static void push_event(timeline_event_t *timeline, int *tlen, int time, int pid, int duration) {
    timeline[*tlen].time = time;
    timeline[*tlen].pid = pid;
    timeline[*tlen].duration = duration;
    (*tlen)++;
}

/* FIFO: non-preemptive, run by arrival order. Ties by pid. */
void schedule_fifo(process_t *processes, int n, timeline_event_t *timeline, int *timeline_len) {
    reset_processes(processes, n);
    *timeline_len = 0;
    // Create an index array sorted by arrival_time then pid
    int *idx = malloc(sizeof(int)*n);
    for (int i = 0; i < n; ++i) idx[i] = i;
    // simple stable sort by arrival_time, then pid
    for (int i = 0; i < n-1; ++i)
        for (int j = i+1; j < n; ++j) {
            if (processes[idx[i]].arrival_time > processes[idx[j]].arrival_time ||
               (processes[idx[i]].arrival_time == processes[idx[j]].arrival_time &&
                processes[idx[i]].pid > processes[idx[j]].pid)) {
                int t = idx[i]; idx[i] = idx[j]; idx[j] = t;
            }
        }
    int time = first_arrival(processes, n);
    for (int k = 0; k < n; ++k) {
        process_t *p = &processes[idx[k]];
        if (time < p->arrival_time) {
            // CPU idle until arrival
            push_event(timeline, timeline_len, time, -1, p->arrival_time - time);
            time = p->arrival_time;
        }
        // start
        if (p->start_time == -1) p->start_time = time;
        push_event(timeline, timeline_len, time, p->pid, p->burst_time);
        time += p->burst_time;
        p->remaining_time = 0;
        p->completion_time = time;
        p->finished = 1;
    }
    free(idx);
}

/* SJF: non-preemptive. At each scheduling point pick arrived process with smallest burst_time. */
void schedule_sjf(process_t *processes, int n, timeline_event_t *timeline, int *timeline_len) {
    reset_processes(processes, n);
    *timeline_len = 0;
    int completed = 0;
    int time = first_arrival(processes, n);
    while (completed < n) {
        // find candidate among arrived & not finished with min burst_time (tie by arrival then pid)
        int best = -1;
        for (int i = 0; i < n; ++i) {
            if (!processes[i].finished && processes[i].arrival_time <= time) {
                if (best == -1 ||
                    processes[i].burst_time < processes[best].burst_time ||
                    (processes[i].burst_time == processes[best].burst_time && processes[i].arrival_time < processes[best].arrival_time)) {
                    best = i;
                }
            }
        }
        if (best == -1) {
            // idle until next arrival
            int next_arr = INT_MAX;
            for (int i = 0; i < n; ++i) if (!processes[i].finished && processes[i].arrival_time < next_arr) next_arr = processes[i].arrival_time;
            push_event(timeline, timeline_len, time, -1, next_arr - time);
            time = next_arr;
            continue;
        }
        process_t *p = &processes[best];
        if (p->start_time == -1) p->start_time = time;
        push_event(timeline, timeline_len, time, p->pid, p->burst_time);
        time += p->burst_time;
        p->remaining_time = 0;
        p->completion_time = time;
        p->finished = 1;
        completed++;
    }
}

/* STCF: preemptive shortest remaining time first */
void schedule_stcf(process_t *processes, int n, timeline_event_t *timeline, int *timeline_len) {
    reset_processes(processes, n);
    *timeline_len = 0;
    int time = first_arrival(processes, n);
    int completed = 0;
    int current_pid = -1;
    int current_idx = -1;
    int current_start = -1;
    while (completed < n) {
        // pick process with smallest remaining_time among arrived and not finished
        int best = -1;
        for (int i = 0; i < n; ++i) {
            if (!processes[i].finished && processes[i].arrival_time <= time) {
                if (best == -1 || processes[i].remaining_time < processes[best].remaining_time ||
                   (processes[i].remaining_time == processes[best].remaining_time && processes[i].arrival_time < processes[best].arrival_time)) {
                    best = i;
                }
            }
        }
        if (best == -1) {
            // idle until next arrival
            int next_arr = INT_MAX;
            for (int i = 0; i < n; ++i) if (!processes[i].finished && processes[i].arrival_time < next_arr) next_arr = processes[i].arrival_time;
            push_event(timeline, timeline_len, time, -1, next_arr - time);
            time = next_arr;
            continue;
        }
        // run best for 1 time unit (granularity = 1)
        process_t *p = &processes[best];
        if (p->start_time == -1) p->start_time = time;
        // If switching from another process, push the previous event
        if (current_idx != best) {
            // finish old event if exists
            if (current_idx != -1) {
                push_event(timeline, timeline_len, current_start, processes[current_idx].pid, time - current_start);
            }
            current_idx = best;
            current_pid = p->pid;
            current_start = time;
        }
        // execute 1 unit
        p->remaining_time -= 1;
        time += 1;
        if (p->remaining_time == 0) {
            // finish process
            p->completion_time = time;
            p->finished = 1;
            completed++;
            // finish current event
            push_event(timeline, timeline_len, current_start, p->pid, time - current_start);
            current_idx = -1;
            current_start = -1;
        }
    }
}

/* Round Robin: preemptive with quantum; simple ready queue by arrival order. */
void schedule_rr(process_t *processes, int n, int quantum, timeline_event_t *timeline, int *timeline_len) {
    reset_processes(processes, n);
    *timeline_len = 0;
    int time = first_arrival(processes, n);
    // simple queue of indices
    int *queue = malloc(sizeof(int)*n*2);
    int qhead = 0, qtail = 0;
    int added[n];
    memset(added, 0, sizeof(added));
    // seed queue with processes that arrive at initial time
    for (int i = 0; i < n; ++i) {
        if (processes[i].arrival_time <= time) {
            queue[qtail++] = i; added[i] = 1;
        }
    }
    while (1) {
        // if queue empty, advance to next arrival or break if all finished
        int all_done = 1;
        for (int i = 0; i < n; ++i) if (!processes[i].finished) { all_done = 0; break; }
        if (all_done) break;
        if (qhead == qtail) {
            // idle until next arrival
            int next_arr = INT_MAX;
            for (int i = 0; i < n; ++i) if (!processes[i].finished && processes[i].arrival_time < next_arr) next_arr = processes[i].arrival_time;
            push_event(timeline, timeline_len, time, -1, next_arr - time);
            time = next_arr;
            for (int i = 0; i < n; ++i) {
                if (!added[i] && processes[i].arrival_time <= time) { queue[qtail++] = i; added[i] = 1; }
            }
            continue;
        }
        int idx = queue[qhead++];
        process_t *p = &processes[idx];
        if (p->start_time == -1) p->start_time = time;
        int use = (p->remaining_time < quantum) ? p->remaining_time : quantum;
        push_event(timeline, timeline_len, time, p->pid, use);
        time += use;
        p->remaining_time -= use;
        // add newly arrived processes to queue
        for (int i = 0; i < n; ++i) {
            if (!added[i] && processes[i].arrival_time <= time) { queue[qtail++] = i; added[i] = 1; }
        }
        if (p->remaining_time > 0) {
            // requeue at tail
            queue[qtail++] = idx;
        } else {
            // finished
            p->completion_time = time;
            p->finished = 1;
        }
    }
    free(queue);
}

/* MLFQ: multi-level feedback queue with num_queues (0 highest), quantums array, and optional boost interval.
   Simplified behavior:
   - All processes start in top queue (0).
   - When a process uses full quantum at level i, it is demoted to i+1 (bounded at last).
   - If it yields earlier (completes or is preempted because quantum ended?), since we run per-quantum slices, we demote only when it fully used the quantum.
   - Boost: every boost_interval time units, all processes (not finished) move back to queue 0.
   Implementation uses per-level FIFO queues.
*/
void schedule_mlfq(process_t *processes, int n, mlfq_config_t *config, timeline_event_t *timeline, int *timeline_len) {
    reset_processes(processes, n);
    *timeline_len = 0;
    int numq = config->num_queues;
    int **q = malloc(sizeof(int*)*numq);
    int *qcap = malloc(sizeof(int)*numq);
    int *qhead = malloc(sizeof(int)*numq);
    int *qtail = malloc(sizeof(int)*numq);
    for (int i = 0; i < numq; ++i) {
        qcap[i] = n + 5;
        q[i] = malloc(sizeof(int)*qcap[i]);
        qhead[i] = qtail[i] = 0;
    }
    int time = first_arrival(processes, n);
    int added[n];
    memset(added, 0, sizeof(added));
    int completed = 0;
    int last_boost = time;
    // add arrivals at start
    for (int i = 0; i < n; ++i) if (processes[i].arrival_time <= time) { q[0][qtail[0]++] = i; added[i]=1; }
    while (completed < n) {
        if (config->boost_interval > 0 && time - last_boost >= config->boost_interval) {
            // boost: move everyone to queue 0 preserving order by scanning queues
            for (int level = 1; level < numq; ++level) {
                while (qhead[level] < qtail[level]) {
                    int pid = q[level][qhead[level]++];
                    q[0][qtail[0]++] = pid;
                }
                qhead[level] = qtail[level] = 0;
            }
            last_boost = time;
        }
        // find highest non-empty queue
        int level = -1;
        for (int i = 0; i < numq; ++i) if (qhead[i] < qtail[i]) { level = i; break; }
        if (level == -1) {
            // no ready processes; advance to next arrival
            int next_arr = INT_MAX;
            for (int i = 0; i < n; ++i) if (!processes[i].finished && processes[i].arrival_time < next_arr) next_arr = processes[i].arrival_time;
            if (next_arr==INT_MAX) break;
            push_event(timeline, timeline_len, time, -1, next_arr - time);
            time = next_arr;
            for (int i = 0; i < n; ++i) if (!added[i] && processes[i].arrival_time <= time) { q[0][qtail[0]++] = i; added[i]=1; }
            continue;
        }
        // pop from queue[level]
        int idx = q[level][qhead[level]++];
        process_t *p = &processes[idx];
        if (p->start_time == -1) p->start_time = time;
        int quantum = config->quantums[level];
        int use = (p->remaining_time < quantum) ? p->remaining_time : quantum;
        push_event(timeline, timeline_len, time, p->pid, use);
        time += use;
        p->remaining_time -= use;
        // new arrivals appended to highest queue
        for (int i = 0; i < n; ++i) if (!added[i] && processes[i].arrival_time <= time) { q[0][qtail[0]++] = i; added[i]=1; }
        if (p->remaining_time == 0) {
            p->completion_time = time;
            p->finished = 1;
            completed++;
        } else {
            // used full quantum? demote; else (if used < quantum) keep same level (but this should not happen because use < quantum only when finishing)
            if (use >= quantum) {
                int new_level = level + 1;
                if (new_level >= numq) new_level = numq - 1;
                q[new_level][qtail[new_level]++] = idx;
            } else {
                q[level][qtail[level]++] = idx;
            }
        }
    }
    for (int i = 0; i < numq; ++i) free(q[i]);
    free(q); free(qcap); free(qhead); free(qtail);
}

