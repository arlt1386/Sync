# CS441/541 Synchronization Project

## Author(s):
Susanna Arlt and Leroy Ombogo


## Date:
11/25/25


## Description:
This program simulates democratic and republican voters that can only vote with other members of their party. It also includes independent voters that can vote with anyone. With default or given values for number of booths, demcrats, republicans and independents, this simulation prints out the status of each voter, arriving at the polling station, waiting for a booth, voting and leaving the station and a visual of each of the booths with whos inside. 

## How to build the software
Parse the arguments and assign each thread their identity. Create the threads and release them into the vote method. Make all the thread wait for eachother and then wait 2 seconds for the polling station to open. Threads will sleep for a random amount of time < 50000 microseconds, then try to get a booth. If it is a republican or demacrat it will wait on the turnstile for fairness. if it is the first thread it will hold all the others and wait until the polling booths are empty, this prevents the democrats and republicans from mixing. The thread will be assigned the first free booth and sleep for a random amount of time < 100000 microseconds. It will then leave the polling station one at a time freeing the booth it was in. If it is the last thread of its kind, it will unlock the lock to let the other party in. after all the threads are done they are joined, the semaphores are destroyed and allocated memory is freed.

## How to use the software
The parameters are: Number of booths with default of 10, number of democrats with default of 5, numer of republicans with default of 5, and number of independents with default of 5. All of these are optional, but program will assume the first is number of booths, seecond is number of democrats and so on.

## How the software was tested
Testing the max number of voters simulation can handle. Testing too many arguments. Testing negative arguments. Testing many voters and 2 booths. Testing different ratios of democrats to republicans to independents.


## Known bugs and problem areas
pthread_create will fail if number of provided voters is greater than 245 in manual mode. Using file redirection, the max number of voters is 211

## Special Section
1. Decribe your solution to the problem. wait for an empty polling room, count the number of voters (by party) with a mutex. lock the lock mutex once in the polling room. wait for an open booth and assign a booth one at a time using a booth mutex. all leave one by one with a mutex and free their booths with a mutex. if you're the last voter, unlock the door. 
2. How does this project avoid deadlock? It avoids deadlock by making sure to use mutexes whil accesing the voting booths and when counting each one of the voters.
3. How does your solution prevent Republican and Democrat voters from waiting on a voting booth at the same time? semaphore called lock to lock the room while one party is in there. This does not affectthe independents.
4. How is your solution fair? the turnstile semaphore allows for one thread at a time ot get a chance to get the lock. this allows order to who eneter the polling room. Is it fair that the independents can cut in front of the party affiliated because they're the ones who don't want to vote next to the opposite party. What about a late republican? The late republican should be able to vote because 
5. How does your solution prevent starvation? It prevents starvation by including a turnsile. This only allows on thread inside of the party separate counters at once. This is important to prevent race condition and allows order and fairness by letting whoever came first to have an opportunity to get the lock. No thread will wait for an indefinate amount of time to get the keys to the polling room (the lock semaphore).
6. How do you balance fairness with resource utilization and throughput? How would you measure this? We did to mostly resource utilization. That way, threads go through faster and throughput is higher dispite some thread having to wait longer.
