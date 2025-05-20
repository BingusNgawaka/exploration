# A winning strategy for Determinant Tic-Tac-Toe on an nxn board

## Introduction
The rules of Determinant Tic-Tac-Toe are simple. There are two players, call them $P_0$ and $P_1$. $P_0$ and $P_1$ take turns placing 0s and 1s respectively into an nxn matrix. The goal of $P_0$ is to make the resulting matrix's determinant zero, and $P_1$'s goal is to make it non zero. Presented in our Linear Algebra tutorial with $n=3$, most quickly convinced themselves that $P_0$ always has a winning strategy. I'm concerned more so with the case of $n \geq 4$ and also whether $P_0$ going first or second matters.

## Claim
My claim is that $P_0$ can always win for $n >= 4$ regardless of who goes first. The proof is relatively intuitive and consists of breaking down the problem into that of proving the forced linear dependence of the rows of B which of course implies that the determinant of B will be 0.

## Definitions and Terminology
Let's define our board as an nxn matrix of 0s and 1s.
```math
    B \in M_n(\Set{0, 1})
```


We also want to view our board as a column of it's rows.
```math
    B = \begin{bmatrix}
        R_1 \\
        \vdots \\
        R_n \\
    \end{bmatrix}
```
```math
    R_i = (r_1, \dots, r_n)
```


## Vector Filling Game
Let's define a new game (that might seem a little contrived at first) that we will use in the proof of our winning strategy for Determinant Tic-Tac-Toe.

Consider two vectors of length k, $P$ and $Q$. This game is very similar in vain to our Tic-Tac-Toe. Each player takes turns placing a 1 or 0 respectively into any position j on either vector. $P_0$'s goal is to fill $P$ and $Q$ such that $P + Q = (1, \dots, 1)$. That is, to make sure that $P + Q$ is equal to the vector of length k filled with ones (called the "k-ones vector" from here onwards). $P$'s goal is the opposite, that is, make $P + Q$ equal to anything *but* the k-ones vector.
Notice that if $P_0$ wins, $p_i + q_i = 1$. This will be important for our big proof later on.

### Theorem 1
For 2 vectors of length $k \geq 1$ $P$ and $Q$, regardless of who moves first, $P_0$ can ensure that $p_i + q_i = 1$ for all $i \in \Set{1, \dots, k}$.

### Proof
Let $P(k)$ be the statement $P_0$ can force $P + Q$ = k-ones vector regardless of who starts.


For k = 1, the outcome is trivial. $P_0$ will win regardless as there are only 2 positions to place anything.


Assume $P(r)$ holds. Now, in the $r+1$ length game consider either player placing their element into a vector. This has the effect of "isolating" this position in both vectors. That is, if $P_0$ places a 0 in the jth position of vector $P$, then $P_1$ can either match with a 1 in the $Q$ vector which reduces the game to that of length $r$, or they can place a 1 elsewhere, ignoring that position which also reduces the game to that of length $r$. Note that since there are $2k$ positions to play in, $P_0$ can never be forced to place a second 0 in that initial position in the other vector as that would require an odd number of play spaces to "swap the parity" of $P_0$ and $P_1$'s turns. Similar logic can be used to prove the case when $P_1$ goes first.


Therefore, by mathematical induction, $P(k)$ holds for all $k \geq 1$.


## Connecting back to Tic-Tac-Toe
Hopefully that very hand-wavy argument was good enough for you. But how and why does this vector filling game matter? Esentially, we want to reduce our Tic-Tac-Toe game into a series of these smaller vector filling games. This then allows us to prove the linear dependence of the rows. So let's do that.

### Theorem 2
For a matrix with an even number of rows $P_0$ has a winning strategy regardless of who starts.

### Proof
Suppose the number of rows $l$ is of the form $2k$ for some $k$ such that $l \geq 4$. Consider pairs of rows of the form:
```math
    {Pair}_i = (R_i, R_{i+1}) \text{ for some odd } i
```
For example, $(R_3, R_4)$.


Notice now that we can construct $k$ many of these pairs. Since $l \geq 4 \implies k \geq 2$. This means that we will always have at least 2 of these pairs.


It is now obvious that the game can be reduced into a series of these vector filling games. $P_0$'s strategy then consists of mirroring $P_1$'s placement in a given pair and since n is even, $P_0$ and $P_1$ have an equal number of turns, which means that $P_0$ can never be forced into placing more than $\frac{n}{2}$ 0's into any given row.


Now, by theorem 1, we can see that given any of the pairs we constructed before, their sum will always equal the vector consisting of all 1s. Since we have at least 2 pairs, say ${Pair}_i$ and ${Pair}_j$ :
```math
    R_i + R_{i+1} = R_j + R_{j+1} \implies R_i = R_j + R_{j+1} - R_{i+1}
```

Which implies that $R_i$ is a linear combination of the other rows, meaning that the rows of B are linearly *dependent* which, for square matrices, means that the determinant is 0.


Thus, for all even $n \geq 4$, $P_0$ has a winning strategy.


### Theorem 3
For odd $n \geq 4$, $P_0$ has a winning strategy regardless of who starts.

### Proof
The key idea here is to "ignore" the bottom row, which makes the problem equivalent to our previous case of an even number of rows. If $P_1$ places a 1 in the bottom row, then $P_0$ can place a 0 in the bottom row as well. If this repeats, the bottom row will eventually be filled and the parity of turns will shift as there are an odd number of columns. For example, if $P_0$ started and both players filled this "odd" row, then $P_1$ would end up going first in the (n-1)xn submatrix. We know by theorem 2 that regardless of who starts in this even-rowed submatrix, $P_0$ has a strategy to force at least two rows to be linearly dependent.


Since $P_0$ can force linearly dependent rows in the (n-1)xn submatrix, the determinant of the whole nxn matrix will be 0. Meaning $P_0$ has a winning strategy regardless of who starts.


## Done!
This is all very hand-wavy as said before but hopefully enough to get a general idea for the proof. I also was halfway through writing up theorem 3 before realising that the rows of a rectangular matrix being linearly dependent don't imply anything about it's determinant because it't *not defined*... duh. So if theorem 2 seems weird it's because it was originally written for the case of an nxn matrix where n is even. Realistically it shouldve been split up into another theorem which shows that given an nxk matrix with n being even $P_0$ can force it to have linearly dependent rows etc but this ended up being way more than I thought it would be anyways so, sorry for the lack of rigor. Also not sure whether these are theorems, lemmas or propositions but oh well.
