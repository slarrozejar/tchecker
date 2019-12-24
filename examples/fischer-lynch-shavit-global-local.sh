#!/bin/bash

# This file is a part of the TChecker project.
#
# See files AUTHORS and LICENSE for copyright details.

# The model is inspired from  
#Parosh Aziz Abdulla, Johann Deneux, Pritha Mahata, Aletta Nylén: "Using Forward
#Reachability Analysis for Verification of Timed Petri Nets.", Nordic Journal of
#Computing, 2007


# Check parameters

K=10

function usage() {
    echo "Usage: $0 N [K]";
    echo "       N number of processes";
    echo "       K delay for mutex (default: ${K})"
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

echo "system:fischer_lynch_shavit_global_local_${N}_${K}
"

# Events

for i in `seq 0 $N`; do
    echo "event:X_TO_${i}
event:x_to_${i}
event:x_is_${i}
event:x_not_${i}"
done
echo "event:tau
event:y_is_false
event:y_is_true
event:y_to_true
event:y_to_false
event:Y_TO_TRUE
event:Y_TO_FALSE
event:y_not_true
event:y_not_false
"

# Processes

for pid in `seq 1 $N`; do
    echo "# Process ${pid}
process:P${pid}
int:1:0:$N:0:x$pid
int:1:0:1:0:y$pid
clock:1:z${pid}
location:P${pid}:start{initial:}	
location:P${pid}:set{}
location:P${pid}:req{}
location:P${pid}:check_bool{}
location:P${pid}:set_bool{}
location:P${pid}:check{}
location:P${pid}:cs{labels:cs${pid}}
location:P${pid}:W{}
location:P${pid}:reset{}

edge:P${pid}:start:set:x_is_0{provided: x${pid}==0 : do : z${pid}=0}
edge:P${pid}:set:req:X_TO_${pid}{provided: z${pid}<=${K} : do : z${pid}=0;x${pid}=${pid}}
edge:P${pid}:req:start:x_not_${pid}{provided: x${pid}!=${pid}}
edge:P${pid}:req:check_bool:x_is_${pid}{provided: x${pid}==${pid} && z${pid}>${K}}
edge:P${pid}:check_bool:start:y_not_false{provided: y${pid}!=0}
edge:P${pid}:check_bool:set_bool:y_is_false{provided: y${pid}==0 : do : z${pid}=0}
edge:P${pid}:set_bool:check:Y_TO_TRUE{provided: z${pid}<=${K} : do : z${pid}=0; y${pid}=1}
edge:P${pid}:check:start:x_not_${pid}{provided: x${pid}!=${pid}}
edge:P${pid}:check:cs:x_is_${pid}{provided: x${pid}==${pid}}
edge:P${pid}:cs:W:tau{do : z${pid}=0}
edge:P${pid}:W:reset:Y_TO_FALSE{provided: z${pid}<=${K} : do : z${pid}=0;y${pid}=0}
edge:P${pid}:reset:start:X_TO_0{provided: z${pid}<=${K} : do : x${pid}=0}
"

    for id in `seq 0 $N`; do
	echo "edge:P$pid:start:start:x_to_$id{do:x$pid=$id}
edge:P$pid:req:req:x_to_$id{do:x$pid=$id}
edge:P$pid:check_bool:check_bool:x_to_$id{do:x$pid=$id}
edge:P$pid:set_bool:set_bool:x_to_$id{do:x$pid=$id}
edge:P$pid:check:check:x_to_$id{do:x$pid=$id}
edge:P$pid:cs:cs:x_to_$id{do:x$pid=$id}
edge:P$pid:W:W:x_to_$id{do:x$pid=$id}"
	
	if [ "$id" -ne "$pid" ]; then
	    echo "edge:P$pid:set:set:x_to_$id{do:x$pid=$id}"
	fi
    echo ""
	if [ "$id" -ne "0" ]; then
	    echo "edge:P$pid:reset:reset:x_to_$id{do:x$pid=$id}"
	fi
    echo ""
    done
echo ""

echo "edge:P$pid:start:start:y_to_true{do:y$pid=1}
edge:P$pid:req:req:y_to_true{do:y$pid=1}
edge:P$pid:set:set:y_to_true{do:y$pid=1}
edge:P$pid:check_bool:check_bool:y_to_true{do:y$pid=1}
edge:P$pid:check:check:y_to_true{do:y$pid=1}
edge:P$pid:cs:cs:y_to_true{do:y$pid=1}
edge:P$pid:W:W:y_to_true{do:y$pid=1}
edge:P$pid:reset:reset:y_to_true{do:y$pid=1}

edge:P$pid:start:start:y_to_false{do:y$pid=0}
edge:P$pid:req:req:y_to_false{do:y$pid=0}
edge:P$pid:set:set:y_to_false{do:y$pid=0}
edge:P$pid:check_bool:check_bool:y_to_false{do:y$pid=0}
edge:P$pid:check:check:y_to_false{do:y$pid=0}
edge:P$pid:cs:cs:y_to_false{do:y$pid=0}
edge:P$pid:reset:reset:y_to_false{do:y$pid=0}
edge:P$pid:set_bool:set_bool:y_to_false{do:y$pid=0}"
echo ""
done
echo ""



# Synchronizations

echo "# Synchronizations"

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

for p in `seq 1 $N`; do
	echo -n "sync:P$p@Y_TO_TRUE";
	for q in `seq 1 $N`; do
	    if [ "$p" -ne "$q" ]; then
		echo -n ":P$q@y_to_true"
	    fi
	done;
echo ""
done

for p in `seq 1 $N`; do
	echo -n "sync:P$p@Y_TO_FALSE";
	for q in `seq 1 $N`; do
	    if [ "$p" -ne "$q" ]; then
		echo -n ":P$q@y_to_false"
	    fi
	done;
echo ""
done



