# POR1

## Description 
por3 is  a partial order reduction for client/server systems. Such a system contains only local actions and synchronizations of one client with the server. $\left\{ 0, ..., n \right\}$ represent the indices of processes of the system, 
where $0$ is the index of the server. A state $q$ has $n$ components where $q_i$ denotes the local 
state of process $i$ when the system is in state $q$. 
$out(q_i)$ refers to the set of enabled actions in state 
$q_i$ for process $i$. A state $q_i$ is *purely local* if $out(q_i)$ contains only local actions of process $i$. The set of enabled actions in state $q$ is denoted by $E$.

This reduction consists in a *source* function with *memory*. 
The *source* function indicates the actions to explore while 
the memory $m$ is updated by the function *mem*. $b$ 
will denote the action taken from state $q$.

Functions *source* and *mem* are defined as follows:

$$
\begin{aligned}
		& source(q,m,E) = 
        \begin{cases}
            E \cap out(q_m) & \text{ if } q_m \text{ is purely local}\\
            E \cap (\bigcup_{j = 1}^n \left\{ sync(q_j)\right\} \cup out(q_m)) & \text{ otherwise}\\
        \end{cases}\\
		\\
		& mem(q,(L,S),b) = i \text{ with } b \in out(q_i)\\
		\end{aligned}
$$
