#!/bin/bash
pid=$(pgrep -f ./fib)
while true; do
    ps -o pid,tid,%mem,rss,comm -T -p "$pid"
    sleep 1
done