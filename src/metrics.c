/*
 * metrics.c
 *
 * Implements calculate_metrics which computes average turnaround, waiting,
 * response time, CPU utilization, throughput, and Jain's fairness index.
 */

#include <stdio.h>
#include <math.h>
#include "metrics.h"

void calculate_metrics(process_t *processes, int n, int total_time, metrics_t *metrics) {
    double sum_tat = 0.0, sum_wt = 0.0, sum_rt = 0.0;
    double busy = 0.0;
    int completed = 0;
    double sum_x = 0.0, sum_x2 = 0.0;
    for (int i = 0; i < n; ++i) {
        process_t *p = &processes[i];
        if (p->completion_time >= 0) {
            completed++;
            p->turnaround_time = p->completion_time - p->arrival_time;
            p->waiting_time = p->turnaround_time - p->burst_time;
            if (p->start_time >= 0) p->response_time = p->start_time - p->arrival_time;
            else p->response_time = -1;
            sum_tat += p->turnaround_time;
            sum_wt += p->waiting_time;
            sum_rt += (p->response_time >=0 ? p->response_time : 0);
            busy += p->burst_time;
            sum_x += p->turnaround_time;
            sum_x2 += (p->turnaround_time * p->turnaround_time);
        } else {
            // If not completed, still count partial busy time
            busy += (p->burst_time - p->remaining_time);
            // For fairness we skip incomplete processes (but usually tests will complete all)
        }
    }
    metrics->avg_turnaround_time = (completed>0) ? (sum_tat / completed) : 0.0;
    metrics->avg_waiting_time = (completed>0) ? (sum_wt / completed) : 0.0;
    metrics->avg_response_time = (completed>0) ? (sum_rt / completed) : 0.0;
    metrics->cpu_utilization = (total_time>0) ? (busy / total_time * 100.0) : 0.0;
    metrics->throughput = (total_time>0) ? ((double)completed / (double)total_time) : 0.0;
    if (n>0 && sum_x2 > 0.0) {
        metrics->fairness_index = (sum_x * sum_x) / ( (double)n * sum_x2 );
    } else {
        metrics->fairness_index = 0.0;
    }
}

