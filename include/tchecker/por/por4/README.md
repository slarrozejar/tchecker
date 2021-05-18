# POR4

## Description 

*por4* is a partial-order reduction technique without memory. A state
$q_i$ with $i \in \llbracket 1, n \rrbracket$ is 
\textit{pure local} if $out(q_i)$ contains only local actions of processus $i$. we denote $pure\_local(q)$ the pure local processes in state $q$.
We define 
$min\_plb(q)$ which computes the lowest 
number of outgoing transitions among all 
pure local states of $q$. $min\_plb(q) = \min \{|E(q_i)| \, | \, i \text{ is pure local}
\}$. We define $branching\_pid(q)$, the lowest pure local process which has the lowest number of outgoing transitions amongst the pure local processes. $branching\_pid(q) = 
\min \{j \in pure\_local(q) \,|\, |E(q_j)| = min\_plb(q)\}$.

Function $\src_4$ is defined as follows:


$$
   source_4(q) = 
    \begin{cases}
        E(q_i) & \text{ if } q_i \text{ is pure local and } i = branching\_pid(q)\\
        E(q) \text{ otherwise }\\
    \end{cases}
$$