#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/scheduler.h"
#include "../include/metrics.h"
#include "../include/algorithms.h"

int main() {
    process_t processes[3] = {
        {1,0,5,1,5,0,0,0,0},
        {2,1,3,2,3,0,0,0,0},
        {3,2,8,1,8,0,0,0,0}
    };
    int n = 3;
    timeline_event_t timeline[100];
    int tlen = 0;
    schedule_stcf(processes,n,timeline,&tlen);

    metrics_t m;
    int total_time = compute_total_time(timeline,tlen);
    calculate_metrics(processes,n,total_time,&m);

    printf("STCF test:\n");
    printf("Avg TAT=%.2f, Avg WT=%.2f\n", m.avg_turnaround_time, m.avg_waiting_time);
    if (m.avg_turnaround_time == 7.33 && m.avg_waiting_time == 2.0) 
        printf("PASSED\n");
    else
        printf("FAILED\n");

    return 0;
}

