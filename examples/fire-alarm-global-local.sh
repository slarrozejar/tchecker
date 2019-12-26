#!/bin/bash

# This file is a part of the TChecker project.
#
# See files AUTHORS and LICENSE for copyright details.

# Generates a TChecker model for the simplified Fire Alarm model in
# Figure 1 of:
# "Timed Automata with Disjoint Activity", Marco Muniz, Bernd Westphal
# and Andreas Podelski, FORMATS 2012.

# Checks command line arguments
if [ $# -eq 1 ];
then
    NSENSORS=$1                  # Number of sensors (processes)
    WINSIZE=10                   # Window size
    DURATION=$(($NSENSORS * 50)) # Cycle duration
else
    if [ $# -eq 3 ];
    then
	NSENSORS=$1 # Number of sensors (processes)
	WINSIZE=$2  # Window size
        DURATION=$3 # Cycle duration
    else
	echo "Usage: $0 <# sensors>";
	echo "       $0 <# sensors> <window size> <cycle duration>";
	echo " ";
	echo "where:    <# sensors>      is the number of sensor processes";
	echo "          <window size>    is the delay allocated for ack.";
	echo "          <cycle duration> is the duration of the whole cycle";
	echo " ";
	echo "Default values: window size=10, cycle duration=50*<# sensors>";
	exit 0;
    fi
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

echo "system:fire_alarm_${NSENSORS}_${WINSIZE}_${DURATION}
"


# Events

echo "# Events
event:tau
event:alive
event:ack
event:reset
event:local
event:e
"

# Sensor processes

for pid in `seq 1 $NSENSORS`; do
    WINSTART=$((2 * $pid * $WINSIZE - $WINSIZE))
    WINEND=$((2 * $pid * $WINSIZE))
    WINSEND=$(($WINSTART + 5))

    echo "# Sensor processes
process:sensor${pid}
clock:1:x${pid}
location:sensor${pid}:ini{initial: : invariant: x${pid}<=$WINSTART}
location:sensor${pid}:wait{invariant: x${pid}<=$WINSEND}
location:sensor${pid}:sent{invariant: x${pid}<=$WINEND}
location:sensor${pid}:fin{invariant: x${pid}<=$DURATION}
location:sensor${pid}:rst{invariant: x${pid}<=$DURATION}
edge:sensor${pid}:ini:wait:tau{provided: x${pid}>=$WINSTART}
edge:sensor${pid}:wait:sent:alive{}
edge:sensor${pid}:sent:fin:ack{}
edge:sensor${pid}:sent:fin:tau{provided: x${pid}>=$WINEND}
edge:sensor${pid}:fin:rst:reset{provided: x${pid}>=$DURATION}
edge:sensor${pid}:rst:ini:local{provided: x${pid}>=$DURATION : do: x${pid}=0}
edge:sensor${pid}:ini:ini:e
edge:sensor${pid}:wait:wait:e
edge:sensor${pid}:sent:sent:e
edge:sensor${pid}:fin:fin:e
"
done

# Central process

echo "# Central process
process:C
location:C:I{initial:}
edge:C:I:I:ack{}
edge:C:I:I:alive{}
edge:C:I:I:e{}
"

# Synchronizations

echo "# Synchronizations"

for pid in `seq 1 $NSENSORS`; do
    echo -n "sync:sensor${pid}@alive:C@alive"
    for p in `seq 1 $NSENSORS`; do
	if [ "$p" -ne "$pid" ]; then
	    echo -n ":sensor${p}@e"
	fi
    done
    echo ""
    echo -n "sync:sensor${pid}@ack:C@ack"
    for p in `seq 1 $NSENSORS`; do
	if [ "$p" -ne "$pid" ]; then
	    echo -n ":sensor${p}@e"
	fi
    done
    echo ""
    echo -n "sync:sensor${pid}@tau:C@e"
    for p in `seq 1 $NSENSORS`; do
	if [ "$p" -ne "$pid" ]; then
	    echo -n ":sensor${p}@e"
	fi
    done
    echo ""
done
echo -n "sync:C@e"
for pid in `seq 1 $NSENSORS`; do
    echo -n ":sensor${pid}@reset"
done
echo ""
