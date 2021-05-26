# POR1

## Description 
por3 is  a partial order reduction for client/server systems. Such a system contains only local actions and synchronizations of one client with the server. $\left\{ 0, ..., n \right\}$ represent the indices of processes of the system.
 A state $q$ has $n$ components where $q_i$ denotes the local 
state of process $i$ when the system is in state $q$. 
$E(q_i)$ refers to the set of enabled actions in state 
$q_i$ for process $i$. A state $q_i$ is *purely local* if $E(q_i)$ contains only local actions of process $i$.
 The set of local actions enabled in state $q$ is denoted by $E_l(q)$ and we note 
 $E_s(q)$ the set of synchronizations enabled in $q$. 
A state $q$ is *pure synchro* if $\forall i \in \llbracket 1, n 
\rrbracket,~E_l(q_i) = \emptyset \land E_s(q_i) \neq \emptyset$. A state 
$q_i$ is *mixed* if $E_l(q_i) \neq \emptyset \land E_s(q_i) \neq \emptyset$.
This reduction consists in a *source* function with *memory*. 
The *source* function indicates the actions to explore while 
the memory $m$ is updated by the function *mem*. $b$ 
will denote the action taken from state $q$.

We consider here extended sets of actions:

$$
    E_{ext}(q) = \set{ (a,0) | 
a \in E(q)} \cup \set{(a,j) | \exists i, q_i \xrightarrow{a} q_j' \land q_j' \text{
    is mixed}}
$$

Functions *source* and *mem* are defined as follows:

$$
    \label{eq:source_por5}
    \begin{aligned}
    & source_5(q,m) = 
    \begin{cases}
        E_{ext}(q_i) & \text{ if } \exists i \in \llbracket 1, n \rrbracket,~q_i 
        \text{ is pure local}\\
        E_{ext}(q) & \text{ if } q \text{ is pure synchro}\\
        E_{l_{ext}}(q_m) & \text{ if } m > 0\\
        E_{s_{ext}}(q) & otherwise\\
    \end{cases}\\
    \\
    & mem_5(q,(b,i)) = i\\
    \end{aligned}
$$