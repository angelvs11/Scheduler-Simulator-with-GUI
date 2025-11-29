#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler.h"
#include "metrics.h"

static void draw_horizontal_line(int y, int x_start, int x_end) {
    for (int x = x_start; x <= x_end; ++x) {
        mvaddch(y, x, '-');
    }
}

static void draw_box_ascii(int y, int x, int h, int w, const char *title) {
    mvaddch(y, x, '+');
    mvaddch(y, x + w - 1, '+');
    mvaddch(y + h - 1, x, '+');
    mvaddch(y + h - 1, x + w - 1, '+');

    for (int i = 1; i < w - 1; ++i) {
        mvaddch(y, x + i, '-');
        mvaddch(y + h - 1, x + i, '-');
    }
    for (int i = 1; i < h - 1; ++i) {
        mvaddch(y + i, x, '|');
        mvaddch(y + i, x + w - 1, '|');
    }

    if (title) {
        mvprintw(y, x + 2, "%s", title);
    }
}

void render_gui(process_t *processes, int n, timeline_event_t *timeline, int tlen, metrics_t *metrics, const char *algorithm_name, int quantum) {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    // Header
    mvprintw(0, (cols - 28)/2, "CPU Scheduler Simulator v1.0");
    draw_horizontal_line(1, 0, cols-1);

    // Process table
    int table_y = 2;
    int table_x = 2;
    int table_w = 40;
    int table_h = n + 4;
    draw_box_ascii(table_y, table_x, table_h, table_w, "Processes");
    mvprintw(table_y + 1, table_x + 1, "PID | Arrival | Burst | Priority");
    for (int i = 0; i < n; ++i) {
        mvprintw(table_y + 2 + i, table_x + 1, " %2d | %6d | %5d | %7d",
                 processes[i].pid, processes[i].arrival_time, processes[i].burst_time, processes[i].priority);
    }

    // Algorithm info
    int algo_y = table_y + table_h + 1;
    mvprintw(algo_y, table_x, "Algorithm: [%s]", algorithm_name);
    if (quantum > 0) {
        printw("  Quantum: [%d]", quantum);
    }

    // Gantt chart
    int gantt_y = algo_y + 2;
    int gantt_w = cols - 4;
    draw_box_ascii(gantt_y, table_x, 5, gantt_w, "Gantt Chart");

    int total_time = 0;
    for (int i = 0; i < tlen; ++i) total_time += timeline[i].duration;
    double scale = (total_time > 0) ? ((double)(gantt_w - 2) / total_time) : 1.0;

    int x = table_x + 1;
    int y = gantt_y + 1;
    int times_y = gantt_y + 3;
    int cur_time = 0;

    for (int i = 0; i < tlen; ++i) {
        int width = (int)(timeline[i].duration * scale + 0.5);
        if (width < 3) width = 3;

        char label[8];
        if (timeline[i].pid == -1) snprintf(label, sizeof(label), "IDLE");
        else snprintf(label, sizeof(label), "P%d", timeline[i].pid);

        // Bloque del Gantt
        mvprintw(y, x, "|%s", label);
        for (int k = strlen(label); k < width - 1; ++k) {
            mvaddch(y, x + k, '=');
        }
        mvaddch(y, x + width - 1, '|');

        // Tiempo debajo
        mvprintw(times_y, x, "%d", cur_time);
        cur_time += timeline[i].duration;

        x += width;
        if (x >= table_x + gantt_w - 1) break;
    }
    // Último tiempo al final del Gantt
    mvprintw(times_y, x, "%d", cur_time);

    // Metrics
    int metrics_y = gantt_y + 7;
    draw_box_ascii(metrics_y, table_x, 7, 40, "Metrics");
    mvprintw(metrics_y + 1, table_x + 1, "Avg Turnaround: %.2f", metrics->avg_turnaround_time);
    mvprintw(metrics_y + 2, table_x + 1, "Avg Waiting:    %.2f", metrics->avg_waiting_time);
    mvprintw(metrics_y + 3, table_x + 1, "Avg Response:   %.2f", metrics->avg_response_time);
    mvprintw(metrics_y + 4, table_x + 1, "CPU Utilization: %.2f%%", metrics->cpu_utilization);
    mvprintw(metrics_y + 5, table_x + 1, "Throughput: %.4f", metrics->throughput);

    // Footer
    mvprintw(rows - 2, table_x, "[R]un  [A]dd Process  [D]elete  [S]ave  [L]oad  [Q]uit");

    refresh();
    getch();
    endwin();
}

