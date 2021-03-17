# POR1

## Description 
por1 is  a partial order reduction for client/server systems. Such a system contains only local actions and synchronizations of one client with the server. $\left\{ 0, ..., n \right\}$ represent the indices of processes of the system, 
where $0$ is the index of the server. A state $q$ has $n$ components where $q_i$ denotes the local 
state of process $i$ when the system is in state $q$. 
$out(q_i)$ refers to the set of enabled actions in state 
$q_i$ for process $i$. A state $q_i$ is *purely local* if $out(q_i)$ contains only local actions of process $i$. The set of enabled actions in state $q$ is denoted by $E$.

This reduction consists in a *source* function with *memory*. 
The *source* function indicates the actions to explore while 
the memory $m$ is updated by the function *memory*. $b$ 
will denote the action taken from state $q$.

Functions *source* and *memory* are defined as follows:

$$
    source(q,0,E) = \left\{
        \begin{array}{ll}
            out(q_i) \cap E & \text{ if i smallest index s.t } q_i \text{ purely local, } out(q_i) \cap E \neq \emptyset\\
            E & \text{ otherwise}
        \end{array}
    \right.
$$
$
    source(q,m,E) = \begin{array}{ll} 
     out(q_m) \cap E & \text{ for } m>0 \\
     \end{array}
$
$$
    mem(q,m,b) = \left\{
        \begin{array}{ll}
            0 & \text{ if $b$ is an action of the server }\\
            m & \text{ if } b \in out(q_m) \text{ and } q_m \text{ is purely local }\\
            i & \text{ if } b \text{ local action of process $i$ and not the above}
        \end{array}
    \right.
$$
 