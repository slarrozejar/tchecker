# RR

## Description 
por3 is  a partial order reduction for client/server systems. Such a system contains
 only local actions and synchronizations of one client with the server.
  $\left\{ 0, ..., n \right\}$ represent the indices of processes of the system.
 A state $q$ has $n$ components where $q_i$ denotes the local 
state of process $i$ when the system is in state $q$. 
$E(q_i)$ refers to the set of enabled actions in state 
$q_i$ for process $i$. A state $q_i$ is *purely local* if $E(q_i)$ contains only
 local actions of process $i$.

 A read action is a communication that is independent with all other communications 
 in that state. Read actions begin by the character '!'. Write actions correspond to 
 the remaining communications. 

This reduction consists in a *source* function with *memory*. 
The *source* function indicates the actions to explore while 
the memory $m$ is updated by the function *mem*. $b$ 
will denote the action taken from state $q$.

$$
	M=\left\{ 0,\dots,n \right\}
$$
Intuitively memory indicates the last process that has done the read action.

$$
\begin{aligned}
	&source(q,0,E)=&
	\begin{cases}
		out(q_i) \cap E & \text{ if } q_i \text{is a local state}\\
		\bigcup \left\{ out(q_i)\cap E : q_i \text{ is a write, or }q_i \text{ is a read and } i\geq m \right\}& \text{when no local state in } q
	\end{cases}\\
	&mem(m,b)=&
	\begin{cases}
		m & \text{ if } b \text{ is a local action}\\
		i & \text{ if } b is a read action of process } i\\
		0 & \text{ if } b \text{ is a write action}
	\end{cases}
\end{aligned}
$$