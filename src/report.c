#include <stdio.h>
#include "report.h"

void generate_report(const char *filename, process_t *processes, int n, 
                     metrics_t *metrics_arr, const char **alg_names, int num_algorithms) {
    FILE *f = fopen(filename, "w");
    if (!f) { perror("fopen"); return; }

    fprintf(f, "# Scheduler Performance Report\n\n");

    fprintf(f, "## Process Set\n\n");
    fprintf(f, "| PID | Arrival | Burst | Priority |\n");
    fprintf(f, "|-----|---------|-------|----------|\n");
    for (int i = 0; i < n; ++i) {
        fprintf(f, "| %d   | %d       | %d     | %d        |\n",
                processes[i].pid,
                processes[i].arrival_time,
                processes[i].burst_time,
                processes[i].priority);
    }
    fprintf(f, "\n");

    fprintf(f, "## Algorithm Comparison\n\n");
    fprintf(f, "| Algorithm | Avg TAT | Avg WT | Avg RT | Throughput |\n");
    fprintf(f, "|-----------|---------|--------|--------|------------|\n");
    for (int i = 0; i < num_algorithms; ++i) {
        fprintf(f, "| %s | %.2f   | %.2f   | %.2f   | %.2f       |\n",
                alg_names[i],
                metrics_arr[i].avg_turnaround_time,
                metrics_arr[i].avg_waiting_time,
                metrics_arr[i].avg_response_time,
                metrics_arr[i].throughput);
    }
    fprintf(f, "\n");

    // Determine best algorithm (lowest Avg TAT)
    int best_idx = 0;
    for (int i = 1; i < num_algorithms; ++i) {
        if (metrics_arr[i].avg_turnaround_time < metrics_arr[best_idx].avg_turnaround_time) {
            best_idx = i;
        }
    }
    fprintf(f, "## Best Algorithm for This Workload\n");
    fprintf(f, "**%s** - Lowest average turnaround time and waiting time\n\n", alg_names[best_idx]);

    fprintf(f, "## Recommendations\n");
    fprintf(f, "- Interactive processes: Use MLFQ or RR\n");
    fprintf(f, "- Batch jobs: Use SJF or STCF\n");
    fprintf(f, "- Mixed workload: Use MLFQ with appropriate tuning\n");

    fclose(f);
}

