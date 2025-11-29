#!/bin/bash
for t in build/test_fifo build/test_sjf build/test_stcf build/test_rr build/test_mlfq; do
    echo "Running $t ..."
    $t
    echo ""
done
