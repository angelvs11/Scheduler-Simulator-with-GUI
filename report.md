# Scheduler Performance Report

## Process Set

| PID | Arrival | Burst | Priority |
|-----|---------|-------|----------|
| 1   | 0       | 8     | 1        |
| 2   | 1       | 4     | 2        |
| 3   | 2       | 9     | 1        |
| 4   | 3       | 5     | 3        |
| 5   | 4       | 2     | 1        |
| 6   | 5       | 6     | 2        |
| 7   | 6       | 3     | 1        |
| 8   | 7       | 7     | 3        |
| 9   | 8       | 4     | 2        |
| 10   | 9       | 1     | 1        |

## Algorithm Comparison

| Algorithm | Avg TAT | Avg WT | Avg RT | Throughput |
|-----------|---------|--------|--------|------------|
| FIFO | 26.20   | 21.30   | 21.30   | 0.20       |
| SJF | 18.70   | 13.80   | 13.80   | 0.20       |
| STCF | 17.10   | 12.20   | 9.70   | 0.20       |
| RR | 30.70   | 25.80   | 10.60   | 0.20       |
| MLFQ | 29.30   | 24.40   | 12.20   | 0.20       |

## Best Algorithm for This Workload
**STCF** - Lowest average turnaround time and waiting time

## Recommendations
- Interactive processes: Use MLFQ or RR
- Batch jobs: Use SJF or STCF
- Mixed workload: Use MLFQ with appropriate tuning
