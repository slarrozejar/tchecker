#!/bin/bash

# This file is a part of the TChecker project.
#
# See files AUTHORS and LICENSE for copyright details.

# Check parameters

K=10

function usage() {
    echo "Usage: $0 N";
    echo "       N number of processes";
}

if [ $# -eq 1 ]; then
    N=$1
else
    usage
    exit 1
fi

# Model

echo "#clock:size:name
#int:size:min:max:init:name
#process:name
#event:name
#location:process:name{attributes}
#edge:process:source:target:event:{attributes}
#sync:events
#   where
#   attributes is a colon-separated list of key:value
#   events is a colon-separated list of process@event
"

echo "system:simple_parallel_2actions_${N}
"

# Events

for pid in `seq 1 $N`; do
    echo "event:a$pid
event:b$pid"
done
echo "event:c
"

# Processes

for pid in `seq 1 $N`; do
    echo "# Process $pid
process:P$pid
clock:1:x$pid
location:P$pid:A{initial:}
location:P$pid:B{}
location:P$pid:C{}
location:P$pid:D{}
edge:P$pid:A:B:a${pid}{provided:x$pid>=$pid}
edge:P$pid:B:C:b${pid}{provided:x$pid>=$pid}
edge:P$pid:C:D:c{}
"
done

# Synchronisations

echo -n "sync"
for pid in `seq 1 $N`; do
    echo -n ":P${pid}@c"
done
echo ""
