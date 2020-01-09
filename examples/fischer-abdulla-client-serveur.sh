#!/bin/bash

# This file is a part of the TChecker project.
#
# See files AUTHORS and LICENSE for copyright details.

#Inspired from:
#Parosh Aziz Abdulla, Johann Deneux, Pritha Mahata, Aletta Nylén: "Using Forward
#Reachability Analysis for Verification of Timed Petri Nets.", Nordic Journal of
#Computing, 2007

# Check parameters

K=10

function usage() {
    echo "Usage: $0 N";
    echo "       $0 N K";
    echo "       N number of processes";
    echo "       K delay for mutex (default: $K)"
}

if [ $# -eq 1 ]; then
    N=$1
elif [ $# -eq 2 ]; then
    N=$1
    K=$2
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

echo "system:fischer_abdulla_client_serveur_${N}_$K
"

# Events

echo "event:tau"

for pid in `seq 0 $N`; do
    echo "event:id_to_$pid
event:id_is_$pid"
done
echo ""

# Processes

for pid in `seq 1 $N`; do
    echo "# Process $pid
process:P$pid
clock:1:x$pid
location:P$pid:A
location:P$pid:req{invariant: x$pid<=$K}
location:P$pid:wait{initial:}
location:P$pid:cs{labels: cs$pid}
edge:P$pid:A:req:id_is_0{do: x$pid=0}
edge:P$pid:req:wait:id_to_$pid{do: x$pid=0}
edge:P$pid:wait:cs:id_is_$pid{provided: x$pid > $K}
edge:P$pid:wait:A:tau
edge:P$pid:cs:A:id_to_0
"
done

# ID shared variable

echo "process:ID
location:ID:id0{initial:}"

for id in `seq 1 $N`; do
    echo "location:ID:id${id}"
done

for id in `seq 0 $N`; do
    echo "edge:ID:id${id}:id${id}:id_is_${id}"
    for id2 in `seq 0 $N`; do
        echo "edge:ID:id${id}:id${id2}:id_to_${id2}"
    done
done
echo ""

# Synchronisations

for pid in `seq 1 $N`; do
    echo "sync:P$pid@id_is_0:ID@id_is_0
sync:P$pid@id_to_0:ID@id_to_0
sync:P$pid@id_is_$pid:ID@id_is_$pid
sync:P$pid@id_to_$pid:ID@id_to_$pid"
done
echo ""
