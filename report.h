#ifndef REPORT_H
#define REPORT_H

#include "scheduler.h"
#include "metrics.h"

/* 
 * Generates a Markdown report comparing multiple algorithms.
 * filename: output file name (e.g., "report.md")
 * processes: array of processes (for process table)
 * n: number of processes
 * metrics_arr: array of metrics, one per algorithm
 * alg_names: array of algorithm names
 * num_algorithms: number of algorithms
 */
void generate_report(const char *filename, process_t *processes, int n, 
                     metrics_t *metrics_arr, const char **alg_names, int num_algorithms);

#endif

