/*
 * scheduler.c
 *
 * Main program: loads workload file, runs chosen algorithm, computes metrics,
 * optionally invokes ncurses GUI, and can write a report.
 *
 * Usage examples:
 *   ./scheduler workloads/workload1.txt fifo
 *   ./scheduler workloads/workload1.txt rr 3
 *   ./scheduler workloads/workload1.txt mlfq 3 "4,8,16" 50
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "scheduler.h"
#include "algorithms.h"
#include "metrics.h"
#include "report.h"  // generate_report

/* read workload file; format: lines with "arrival burst priority" */
int load_workload(const char *path, process_t **out_processes) {
    FILE *f = fopen(path, "r");
    if (!f) { perror("fopen"); return -1; }
    int capacity = 32;
    process_t *list = malloc(sizeof(process_t) * capacity);
    int count = 0;
    int pid_counter = 1;
    while (!feof(f)) {
        int arrival, burst, priority;
        if (fscanf(f, "%d %d %d", &arrival, &burst, &priority) == 3) {
            if (count >= capacity) {
                capacity *= 2;
                list = realloc(list, sizeof(process_t) * capacity);
            }
            list[count].pid = pid_counter++;
            list[count].arrival_time = arrival;
            list[count].burst_time = burst;
            list[count].priority = priority;
            count++;
        } else {
            char buf[256];
            if (!fgets(buf, sizeof(buf), f)) break;
        }
    }
    fclose(f);
    *out_processes = list;
    return count;
}

/* compute total_time from timeline */
int compute_total_time(timeline_event_t *timeline, int tlen) {
    int total = 0;
    for (int i = 0; i < tlen; ++i) total += timeline[i].duration;
    return total;
}

/* optional: print timeline for debug */
static void print_timeline(timeline_event_t *timeline, int tlen) {
    printf("Timeline events:\n");
    for (int i = 0; i < tlen; ++i) {
        printf("  time=%d pid=%d dur=%d\n", timeline[i].time, timeline[i].pid, timeline[i].duration);
    }
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s <workload_file> <algorithm> [params...]\n", argv[0]);
        printf("Algorithms: fifo, sjf, stcf, rr <quantum>, mlfq <num_q> <comma_quants> <boost_interval>\n");
        return 1;
    }

    const char *workload = argv[1];
    const char *alg = argv[2];

    process_t *processes = NULL;
    int n = load_workload(workload, &processes);
    if (n <= 0) { fprintf(stderr, "No processes loaded.\n"); return 2; }

    // allocate timeline
    int max_events = 10000;
    timeline_event_t *timeline = malloc(sizeof(timeline_event_t) * max_events);
    int tlen = 0;

    metrics_t metrics; // for the selected algorithm

    // copy of processes for safety
    process_t *orig = malloc(sizeof(process_t)*n);
    memcpy(orig, processes, sizeof(process_t)*n);

    // run selected algorithm
    if (strcmp(alg, "fifo") == 0) {
        schedule_fifo(processes, n, timeline, &tlen);
    } else if (strcmp(alg, "sjf") == 0) {
        schedule_sjf(processes, n, timeline, &tlen);
    } else if (strcmp(alg, "stcf") == 0) {
        schedule_stcf(processes, n, timeline, &tlen);
    } else if (strcmp(alg, "rr") == 0) {
        if (argc < 4) { fprintf(stderr, "rr requires quantum param\n"); return 3; }
        int q = atoi(argv[3]);
        if (q <= 0) { fprintf(stderr, "quantum must be > 0\n"); return 3; }
        schedule_rr(processes, n, q, timeline, &tlen);
    } else if (strcmp(alg, "mlfq") == 0) {
        if (argc < 6) { fprintf(stderr, "mlfq requires num_queues quantums_csv boost_interval\n"); return 3; }
        int numq = atoi(argv[3]);
        char *csv = argv[4];
        int boost = atoi(argv[5]);
        int *quantums = malloc(sizeof(int)*numq);
        char *tmp = strdup(csv);
        char *tok = strtok(tmp, ",");
        int i = 0;
        while (tok && i < numq) {
            quantums[i++] = atoi(tok);
            tok = strtok(NULL, ",");
        }
        free(tmp);
        if (i != numq) { fprintf(stderr, "quantums count mismatch\n"); return 4; }
        mlfq_config_t cfg;
        cfg.num_queues = numq;
        cfg.quantums = quantums;
        cfg.boost_interval = boost;
        schedule_mlfq(processes, n, &cfg, timeline, &tlen);
        free(quantums);
    } else {
        fprintf(stderr, "Unknown algorithm '%s'\n", alg);
        return 4;
    }

    // calculate metrics
    int total_time = compute_total_time(timeline, tlen);
    calculate_metrics(processes, n, total_time, &metrics);

    // textual output
    printf("Algorithm: %s\n", alg);
    printf("Processes:\n");
    for (int i = 0; i < n; ++i) {
        printf("PID %d: arrival=%d burst=%d priority=%d start=%d completion=%d tat=%d wt=%d rt=%d\n",
               processes[i].pid, processes[i].arrival_time, processes[i].burst_time, processes[i].priority,
               processes[i].start_time, processes[i].completion_time,
               processes[i].turnaround_time, processes[i].waiting_time, processes[i].response_time);
    }
    print_timeline(timeline, tlen);
    printf("\nMetrics:\n");
    printf("Avg Turnaround Time: %.2f\n", metrics.avg_turnaround_time);
    printf("Avg Waiting Time:    %.2f\n", metrics.avg_waiting_time);
    printf("Avg Response Time:   %.2f\n", metrics.avg_response_time);
    printf("CPU Utilization:     %.2f%%\n", metrics.cpu_utilization);
    printf("Throughput:          %.4f\n", metrics.throughput);
    printf("Fairness Index:      %.4f\n", metrics.fairness_index);

    // optional GUI
    printf("\nLaunch ncurses GUI? (y/N): ");
    int c = getchar();
    if (c == 'y' || c == 'Y') {
        extern void render_gui(process_t *processes, int n, timeline_event_t *timeline, int tlen,
                               metrics_t *metrics, const char *algorithm_name, int quantum);
        int q = 0;
        if (strcmp(alg,"rr")==0 && argc>=4) q = atoi(argv[3]);
        render_gui(processes, n, timeline, tlen, &metrics, alg, q);
    }

    // generate comparison report for all algorithms
    {
        metrics_t all_metrics[5];
        const char *alg_names[] = {"FIFO","SJF","STCF","RR","MLFQ"};
        for (int i=0; i<5; i++) {
            process_t *copy = malloc(sizeof(process_t)*n);
            memcpy(copy, processes, sizeof(process_t)*n);
            int tlen_copy = 0;
            timeline_event_t *timeline_copy = malloc(sizeof(timeline_event_t)*max_events);
            switch(i) {
                case 0: schedule_fifo(copy,n,timeline_copy,&tlen_copy); break;
                case 1: schedule_sjf(copy,n,timeline_copy,&tlen_copy); break;
                case 2: schedule_stcf(copy,n,timeline_copy,&tlen_copy); break;
                case 3: schedule_rr(copy,n,3,timeline_copy,&tlen_copy); break; // quantum=3
                case 4: {
                    mlfq_config_t cfg = {3, (int[]){4,8,16}, 50};
                    schedule_mlfq(copy,n,&cfg,timeline_copy,&tlen_copy);
                    break;
                }
            }
            metrics_t m;
            calculate_metrics(copy,n,compute_total_time(timeline_copy,tlen_copy),&m);
            all_metrics[i] = m;
            free(copy);
            free(timeline_copy);
        }
        generate_report("report.md", processes, n, all_metrics, alg_names, 5);
        printf("\nReport generated: report.md\n");
    }

    free(processes);
    free(orig);
    free(timeline);
    return 0;
}

