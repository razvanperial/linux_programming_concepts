# Linux programming concepts and principles

This repository contains problems that make use of concepts and principles used by Unix based operating systems to provide programming abstractions that enable 
an efficient and robust execution of application programs.

## Here is a list of the problems presented and their different use cases:
### env.c
- this programm implements some of the functionality of the standard env program. The
syntax of the command line arguments is the following:
  > `./env [OPTION]... [NAME=VALUE]... [COMMAND [ARG]...]`
  
- If called without any arguments, env prints the current environment to the standard output.
- If called with a sequence of “name=value” pairs and no further arguments, the program adds
the “name=value” pairs to the environment and the prints the environment to the standard
output.
- If called with a command and optional arguments, env executes the command with the given
arguments.
- If called with a sequence of “name=value” pairs followed by a command and optional arguments, the program adds the “name=value” pairs to the environment and executes the command with the given arguments in the modified environment.
- If called with the option -v, the program writes a trace of what it is doing to the standard error.
- If called with the option -u name, the program removes the variable name from the environment.
- Some example invocations: 
```
$ env                       # print the current environment
$ env foo=bar               # add foo=bar and print the environment
$ env -u foo                # remove foo and print the environment
$ env date                  # execute the program date
$ env TZ=GMT date           # add TZ=GMT and execute the program date
$ env -u TZ date            # remove TZ and execute the program date
$ env -u x a=b b=c date     # remove x, add a and b, execute date
```
--- 

## xargs.c
- A simplified version of the Unix xargs utility
- It reads lines from the standard input and constructs argument lists for a command to be executed
- It continues constructing argument lists and executing commands until the end of the standard input has been reached.
- The command to execute is specified as part of the xargs arguments or if none are provided, then ```/bin/echo``` is used.
- If ```-n``` is passed as an option, its argument represents the (maximum) number of input lines added to the constructed argument lists
- If ```-t``` is passed as an option, the programm shows the argument list (on stderr) before the command is executed
- Some example invocations: 
```
$ echo "hello world" | xargs
hello world
$ seq 0 10 | xargs
0 1 2 3 4 5 6 7 8 9 10
$ seq 0 10 | xargs -t
/bin/echo 0 1 2 3 4 5 6 7 8 9 10
0 1 2 3 4 5 6 7 8 9 10
$ seq 0 10 | xargs -n 3
0 1 2
3 4 5
6 7 8
9 10
$ seq 0 10 | xargs -n 3 -t
/bin/echo 0 1 2
0 1 2
/bin/echo 3 4 5
3 4 5
/bin/echo 6 7 8
6 7 8
/bin/echo 9 10
9 10
$ seq 1 4 | xargs -t -n 1 printf "foo-%02d\n"
printf foo-%02d\n 1
foo-01
printf foo-%02d\n 2
foo-02
printf foo-%02d\n 3
foo-03
printf foo-%02d\n 4
foo-04
```
---
### perfect.c (pthread)
- The programm finds perfect numbers in a range for numbers, using the ```pthread``` system call. 
- A perfect number is a positive integer that is equal to the sum of its positive divisors, excluding the
number itself. For example, 6 has the positive divisors { 1, 2, 3 } and 1 + 2 + 3 = 6.
- The default number range is [1, 10000]
- The program accepts the ```-s``` option to set the lower bound and the ```-e``` option to set the higher bound.
- programm also accepts ```-t```, which can be used to define how many concurrent threads should be
used to execute the search.
- If the ```-t``` option is not present, then a single thread is used to carry out the search
- The ```-v``` option writes trace information to the standard error (for debugging purposes).
- One example invocation: 
```
./perfect -t 2 -v
perfect: t0 searching [1,5000]
perfect: t1 searching [5001,10000]
6
28
496
8128
perfect: t0 finishing
perfect: t1 finishing
```
---
### prisoners.c (pthread and mutex lock and unlock)
- A C implementation of the 100 prisoners problem. To find out how this problem sounds like, check this [link](https://en.wikipedia.org/wiki/100_prisoners_problem).
- The program uses the ```pthread``` system call, and ```pthread_mutex_lock``` and ```pthread_mutex_unlock```
- The programm implements the prisoners as concurrent threads. It symulates 4 different types of games. Two for the random strategy and two for the sequence strategy.
- We also have 2 different locking strategies: 
1. Using a global lock for all drawers: A prisoner first obtains the global lock and then executes
the search for his number. When done, the prisoner releases the global lock.
2. Using a separate lock for each drawer: Prisoners inspect drawers concurrently but they have
to obtain a lock for a specific drawer before opening it and they release the lock after closing
the drawer. 
- The command line option ```-n``` determines how many games to simulate. The default is 100 games.
- The command line option ```-s``` seeds the random number generator with a non-static value.
- One example invocation:
```
$ ./prisoner
method random_global 0/100 wins = 0.00% 191.161 ms
method random_drawer 0/100 wins = 0.00% 173.900 ms
method strategy_global 26/100 wins = 26.00% 219.138 ms
method strategy_drawer 35/100 wins = 35.00% 186.296 ms
```


