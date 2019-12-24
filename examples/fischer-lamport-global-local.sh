#!/bin/bash

# This file is a part of the TChecker project.
#
# See files AUTHORS and LICENSE for copyright details.

# The model is inspired from the paper 
#"A Fast Mutual Exclusion Algorithm",by Leslie Lamport. 
#Appeared in ACM Transactions on Computer Systems, Volume 5, Number 1,
# February 1987, Pages 1–11.

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

# Labels
labels="cs1"
for pid in `seq 2 $N`; do
    labels="${labels},cs${pid}"
done
echo "#labels=${labels}"

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

echo "system:fischer_global_local_${N}_$K
"

# Events

echo "event:tau"

for pid in `seq 0 $N`; do
    echo "event:x_to_$pid
event:X_TO_$pid
event:y_to_$pid
event:Y_TO_$pid
"
done

echo ""

# Processes

for pid in `seq 1 $N`; do
    echo "# Process $pid
process:P$pid
clock:1:z$pid
int:1:0:$N:0:x$pid
int:1:0:$N:0:y$pid
location:P${pid}:start{initial:}	
location:P${pid}:set{}
location:P${pid}:check1{committed:}
location:P${pid}:req{}
location:P${pid}:wait{}
location:P${pid}:check2{committed:}
location:P${pid}:delay{}
location:P${pid}:cs{labels:cs${pid}:invariant:z${pid}<=${K}}

edge:P${pid}:start:set:X_TO_$pid{do: x$pid=$pid}
edge:P${pid}:set:check1:tau{}
edge:P${pid}:check1:req:tau{provided:y$pid == 0}
edge:P${pid}:check1:start:tau{provided:y$pid != 0}
edge:P${pid}:req:wait:Y_TO_$pid{do: y$pid=$pid}
edge:P${pid}:wait:check2:tau{}
edge:P${pid}:check2:cs:tau{provided:x$pid == $pid : do : z${pid}=0}
edge:P${pid}:check2:delay:tau{provided:x$pid != $pid : do : z${pid}=0}
edge:P${pid}:delay:start:tau{provided: y$pid != $pid && z${pid}>${K}}
edge:P${pid}:delay:cs:tau{provided: y$pid == $pid &&  z${pid}>${K}}
edge:P${pid}:cs:start:Y_TO_0{do: y$pid=0}"

    for id in `seq 0 $N`; do
	echo "edge:P$pid:set:set:x_to_$id{do: x$pid=$id}
edge:P$pid:check1:check1:x_to_$id{do: x$pid=$id}
edge:P$pid:req:req:x_to_$id{do: x$pid=$id}
edge:P$pid:wait:wait:x_to_$id{do: x$pid=$id}
edge:P$pid:check2:check2:x_to_$id{do: x$pid=$id}
edge:P$pid:delay:delay:x_to_$id{do: x$pid=$id}
edge:P$pid:cs:cs:x_to_$id{do: x$pid=$id}

edge:P$pid:start:start:y_to_$id{do: y$pid=$id}
edge:P$pid:set:set:y_to_$id{do: y$pid=$id}
edge:P$pid:check1:check1:y_to_$id{do: y$pid=$id}
edge:P$pid:wait:wait:y_to_$id{do: y$pid=$id}
edge:P$pid:check2:check2:y_to_$id{do: y$pid=$id}
edge:P$pid:delay:delay:y_to_$id{do: y$pid=$id}
"	
	if [ "$id" -ne "$pid" ]; then
	    echo "edge:P$pid:start:start:x_to_$id{do: x$pid=$id}
edge:P$pid:req:req:y_to_$id{do: y$pid=$id}"

	fi
	if [ "$id" -ne "0" ]; then
	    echo "edge:P$pid:cs:cs:y_to_$id{do:y$pid=$id}"
	fi
    done
echo ""
done

# Synchronisations

for id in `seq 0 $N`; do
    for p in `seq 1 $N`; do
	echo -n "sync:P$p@X_TO_$id";
	for q in `seq 1 $N`; do
	    if [ "$p" -ne "$q" ]; then
		echo -n ":P$q@x_to_$id"
	    fi
	done;
	echo ""
    done
done

for id in `seq 0 $N`; do
    for p in `seq 1 $N`; do
	echo -n "sync:P$p@Y_TO_$id";
	for q in `seq 1 $N`; do
	    if [ "$p" -ne "$q" ]; then
		echo -n ":P$q@y_to_$id"
	    fi
	done;
	echo ""
    done
done

