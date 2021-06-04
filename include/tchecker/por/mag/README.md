# CS

## Description 
mag is  a partial order reduction for client/server systems. Such a system contains only
 local actions and synchronizations of one client with the server. $\left\{ 0, ..., n \right\}$ 
 represent the indices of processes of the system. A state $q$ has $n$ components where $q_i$ denotes the local 
state of process $i$ when the system is in state $q$. 
$out(q_i)$ refers to the set of enabled actions in state 
$q_i$ for process $i$. 

Models are composed of magnetic and non magnetic states, where non magnetic states 
names begin by caracter *!*. Initial and final states are magnetic and on every path 
between a pair of synchronizations there is a magnetic state. 

This reduction consists in a *source* function that indicates the actions to explore.

Functions *source* is defined as follows:

$$
source(q) = 
    \begin{cases}
         out(q) if all locations in q are magnetic\\
        out(q_i) if q_i is the unique non magnetic location in q\\        
    \end{cases}\\
$$
