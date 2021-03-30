# POR2

## Description 
por2 is  a partial order reduction for client/server systems. Such a system contains only local actions and synchronizations of one client with the server. $\left\{ 0, ..., n \right\}$ represent the indices of processes of the system, 
where $0$ is the index of the server. A state $q$ has $n$ components where $q_i$ denotes the local 
state of process $i$ when the system is in state $q$. 
$out(q_i)$ refers to the set of enabled actions in state 
$q_i$ for process $i$. The set of enabled actions in state $q$ is denoted by $E$. $local(q_i)$ denotes the set of local actions 
enabled in $q_j$ and $sync(q_i)$ denotes the set of syncronizations of client $i$ with the server enabled in $q_i$.

This reduction consists in a *source* function with *memory*. 
The *source* function indicates the actions to explore while 
the memory $m$ is updated by the function *mem*. $b$ 
will denote the action taken from state $q$. Function *mem* can take one or two parameters $L$ and $S$. $L$ represents the set of 
processes that moved in the last local phase while $S$ is the set of processes that moved in the last synchronization phase. 

Functions *source* and *mem* are defined as follows:

$$
\begin{aligned}
& source(q,(L,S),E) = 
    \begin{cases}
         E \cap (\bigcup \left\{ local(q_j) : j \ge max(L), j \in S \right\} \cup \bigcup \left\{ sync(q_j), j \in L \right\})     
    \end{cases}\\
\\
& source(q,S,E) = 
    \begin{cases}
         E \cap (\bigcup \left\{ sync(q_j), j = 1, \dots, n \right\} \cup \bigcup \left\{ local(q_j), j \in S \right\} ) 
    \end{cases}\\
\\
& mem(q,(L,S),b) =
    \begin{cases}
        (L \cup \left\{ i \right\}, S) & \text{ if } b \text{ local action of process } i\\
        \left\{ i \right\} & \text{ if } b \text{ synchronization of process } i\\
    \end{cases}\\
\\
& mem(q,S,b) =
    \begin{cases}
        S \cup \left\{ i \right\} & \text{ if } b \text{ synchronization of process } i\\
        (\left\{ i \right\}, S) & \text{ if } b \text{ local action of process } i\\
    \end{cases}
\end{aligned}
$$