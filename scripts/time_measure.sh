#!/usr/bin/env bash

# USAGE: time_measure.sh IMPL SCENE [NUMBER]

asm_rt() {
    ./ray_tracer -a -s "$1" >/dev/null 2>&1
}

cpp_rt() {
    "$HOME/projects/ray-tracing-iow-cpp/iow" "$1" >/dev/null 2>&1
}

mongi_rt() {
    "$HOME/source/ray_tracer/ray_tracer" "$1" 1 >/dev/null 2>&1
}

export TIMEFORMAT="%3U,%3R"

impl="$1"
scene="$2"
number="${3:-10}"

totalctime=0
totalwtime=0
for i in $(seq 1 "$number"); do
    bothtimes="$({ time "${impl}_rt" "$scene"; } 2>&1)"

    ctime="${bothtimes%,*}"
    ctime="$(echo "$ctime * 1000" | bc)"
    ctime="${ctime%.*}"

    wtime="${bothtimes#*,}"
    wtime="$(echo "$wtime * 1000" | bc)"
    wtime="${wtime%.*}"

    totalctime=$((totalctime + ctime))
    totalwtime=$((totalwtime + wtime))
done

avgctime="$(echo "scale=2; $totalctime/$number" | bc)"
avgwtime="$(echo "scale=2; $totalwtime/$number" | bc)"
printf "cpu: %.2f ms\nwall: %.2f ms\n" "$avgctime" "$avgwtime"
