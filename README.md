## Conveyor Belt - C implementation of this synchronization problem. 

### This program is an illustation of the following assembly line/conveyor belt setup:

* The Converyor belt has 3 slots.

```
        ---------
        |   0   |
        |       |
        ---------
        |   1   |
        |       |
        ---------
        |   2   |
        |       |
        ---------
```

* There are 2 workers, 1 on either side of each slot.

```
        ---------
        |   0   |
    W0  |       |   W1
        ---------
        |   1   |
    W2  |       |   W3
        ---------
        |   2   |
    W4  |       |   W5
        ---------
```

* The slots are initially empty.

```
        ---------
        |   E   |
    W0  |       |   W1
        ---------
        |   E   |
    W2  |       |   W3
        ---------
        |   E   |
    W4  |       |   W5
        ---------
```

* When the belt starts rolling, part A and part B of product P starts appearing on the slot 0.
    * Note: slot 0 can start with part A, part B or can be empty E with equal probability

```
        run 1                   run 2                   run 3                   run 4
        ---------               ---------               ---------               ---------
        |   B   |               |   E   |               |   A   |               |   A   |
    W0  |       |   W1      W0  |       |   W1      W0  |       |   W1      W0  |       |   W1
        ---------               ---------               ---------               ---------
        |   E   |               |   B   |               |   E   |               |   A   |
    W2  |       |   W3      W2  |       |   W3      W2  |       |   W3      W2  |       |   W3
        ---------               ---------               ---------               ---------
        |   E   |               |   E   |               |   B   |               |   E   |
    W4  |       |   W5      W4  |       |   W5      W4  |       |   W5      W4  |       |   W5
        ---------               ---------               ---------               ---------
```

* Worker, W<sub>i</sub>, can take the part A on slot S<sub>i/2</sub> if:
    * W<sub>i</sub> is holding part B 
    * W<sub>i</sub> is holding onto nothing, E, and it's counterpart, W<sub>i-1</sub>, (_if **i** in W<sub>i</sub> is odd_), or W<sub>i+1</sub>, (_if **i** in W<sub>i</sub> is even_) has no use for it (i.e. if the counterpart is holding onto nothing, E, or is already holding onto part A, or is holding onto product P, or is busy assembling product P from part A and part B which it already has)

```
                     |      run 1          |      run 1          |      run 2          |      run 2
      ---------      |      ---------      |      ---------      |      ---------      |      ---------
      |   E   |      |      |   A   |      |      |   E   |      |      |   A   |      |      |   E   |
W0(E) |       | W1(E)|W0(E) |       | W1(E)|W0(E) |       | W1(A)|W0(E) |       | W1(A)|W0(A) |       | W1(A)
      ---------      |      ---------      |      ---------      |      ---------      |      ---------
      |   E   |      |      |   E   |      |      |   E   |      |      |   E   |      |      |   E   |
W2(E) |       | W3(E)|W2(E) |       | W3(E)|W2(E) |       | W3(E)|W2(E) |       | W3(E)|W2(E) |       | W3(E)
      ---------      |      ---------      |      ---------      |      ---------      |      _________
      |   E   |      |      |   E   |      |      |   E   |      |      |   E   |      |      |   E   |
W4(E) |       | W5(E)|W4(E) |       | W5(E)|W4(E) |       | W5(E)|W4(E) |       | W5(E)|W4(E) |       | W5(E)
      ---------      |      ---------      |      ---------      |      ---------      |      ---------
```

* Once Worker, W<sub>i</sub> has both part A and part B in hand, it takes 3 runs or slots to roll by before it can finish assembling product P from those.
    * Note: The finished product is placed onto the 1st empty slot after 3 runs have gone by.

```
                       |      run 1            |      run 1            |      run 2            |      run 2
      ---------        |      ---------        |      ---------        |      ---------        |      ---------
      |   E   |        |      |   A   |        |      |   E   |        |      |   B   |        |      |   E   |
W0(E) |       | W1(E)  |W0(E) |       | W1(E)  |W0(E) |       | W1(A)  |W0(E) |       | W1(A)  |W0(E) |       | W1(A+B)
      ---------        |      ---------        |      ---------        |      ---------        |      ---------
      |   E   |        |      |   E   |        |      |   E   |        |      |   E   |        |      |   E   |
W2(E) |       | W3(E)  |W2(E) |       | W3(E)  |W2(E) |       | W3(E)  |W2(E) |       | W3(E)  |W2(E) |       | W3(E)
      ---------        |      ---------        |      ---------        |      ---------        |      _________
      |   E   |        |      |   E   |        |      |   E   |        |      |   E   |        |      |   E   |
W4(E) |       | W5(E)  |W4(E) |       | W5(E)  |W4(E) |       | W5(E)  |W4(E) |       | W5(E)  |W4(E) |       | W5(E)
      ---------        |      ---------        |      ---------        |      ---------        |      ---------
------------------------------------------------------------------------------------------------------------------------
      run 3            |      run 3            |      run 4            |      run 5            |      run 5
      ---------        |      ---------        |      ---------        |      ---------        |      ---------
      |   B   | Skip 1 |      |   E   | Skip 1 |      |   B   | Skip 2 |      |   B   | Skip 3 |      |   B   | Skip 3
W0(E) |       | W1(A+B)|W0(B) |       | W1(A+B)|W0(B) |       | W1(A+B)|W0(B) |       | W1(A+B)|W0(B) |       | W1(A+B)
      ---------        |      ---------        |      ---------        |      ---------        |      ---------
      |   E   |        |      |   E   |        |      |   E   |        |      |   B   |        |      |   E   |
W2(E) |       | W3(E)  |W2(E) |       | W3(E)  |W2(E) |       | W3(E)  |W2(E) |       | W3(E)  |W2(B) |       | W3(E)
      ---------        |      ---------        |      ---------        |      ---------        |      _________
      |   E   |        |      |   E   |        |      |   E   |        |      |   E   |        |      |   E   |
W4(E) |       | W5(E)  |W4(E) |       | W5(E)  |W4(E) |       | W5(E)  |W4(E) |       | W5(E)  |W4(E) |       | W5(E)
      ---------        |      ---------        |      ---------        |      ---------        |      ---------
------------------------------------------------------------------------------------------------------------------------
      run 6            |      run 6            |      run 7            |      run 8            |      run 9
      ---------        |      ---------        |      ---------        |      ---------        |      ---------
      |   E   |        |      |   P   |        |      |   E   |        |      |   E   |        |      |   E   |
W0(B) |       | W1(P)  |W0(B) |       | W1(E)  |W0(B) |       | W1(E)  |W0(B) |       | W1(E)  |W0(B) |       | W1(E)
      ---------        |      ---------        |      ---------        |      ---------        |      ---------
      |   B   |        |      |   E   |        |      |   P   |        |      |   E   |        |      |   E   |
W2(B) |       | W3(E)  |W2(B) |       | W3(B)  |W2(B) |       | W3(B)  |W2(B) |       | W3(B)  |W2(B) |       | W3(B)
      ---------        |      ---------        |      ---------        |      ---------        |      _________
      |   E   |        |      |   E   |        |      |   E   |        |      |   P   |        |      |   E   |
W4(E) |       | W5(E)  |W4(E) |       | W5(E)  |W4(E) |       | W5(E)  |W4(E) |       | W5(E)  |W4(E) |       | W5(E)
      ---------        |      ---------        |      ---------        |      ---------        |      ---------
                                                                                                          P
```

* Given this illustration:
    * design a conveyor belt, worker system
    * run it for a 100 times
    * output the number of finished products that come off the line, the number of part A and part B wasted.
