This program is a simulation in which a number of threads access a group of
shared buffers for both reading and writing purposes. Initially this will be done without the benefit of
synchronization tools ( semaphores ) to illustrate the problems of race conditions, and then with the
synchronization tools to solve the problems. 

To run the program:
1. type in "make"
(type in values for the following varaibles: nBUffers nWorkers sleepMin Sleepmax RandSeed lock)
2. type in "./friendship nBuffers nWorkers [ sleepMin sleepMax ] [ randSeed ] [ -lock | -nolock ].
EXAMPLE: "./friendship 11 10 .1 .2 0 -unlock"
3. When running simulation with "lock" please be patient. It might take a bit of time depending on # of Buffers and Workers.
