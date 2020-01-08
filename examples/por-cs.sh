#!/bin/bash

# This file is a part of the TChecker project.
#
# See files AUTHORS and LICENSE for copyright details.

# Check parameters

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

echo "system:por_cs
"

# Events

echo "event:a"
echo "event:c"
echo ""

# Clients

for pid in `seq 1 $N`; do
    echo "# Client $pid
process:C$pid
clock:1:x$pid
location:C$pid:A{initial:}
location:C$pid:B{}
location:C$pid:C{}
edge:C$pid:A:B:a{do: x$pid=0}
edge:C$pid:B:C:c
"
done

# Server

echo "# Server
process:S
location:S:A{initial:}
edge:S:A:A:c
"

# Synchronisations

for pid in `seq 1 $N`; do
    echo "sync:C$pid@c:S@c"
done
echo ""
