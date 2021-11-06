# xv6-risc

## Specification 1: Syscall tracing 
1. All syscalls pass through the function `syscall` in `syscall.c`. Hence this would be a very good place to use to implement the strace syscall.
2. We add a variable `strace_mask` to the `proc` datastructure which records which syscalls must be printed by strace. 
3. We then add the strace syscall which simply sets the mask to the value of the argument passed to it.
4. Now in `syscall`, we simply check if any of the bits in the mask have been set and if it has been set then we know we have to print it.
5. Now, the syscall itself might modify the arguments passed. Specifically, it might overwrite `a_0`. Hence we save the arguments passed to the trapframe beforehand.
6. Because there is no easy way to know how many arguments each syscall accepts we hardcode them in an array.
7. After the syscall has executed just print the required syscall traces with the saved values.
8. Add the stubs to `usys.pl` so it generates the required asm for the instruction and add the declaration to `user.h` so the compiler can recognize it.
9. And we're done. 

# Specification 2: Scheduling
1. We modify the makefile so we can easily switch between schedulers. It accepts arguments of the form `make qemu SCHEDULER=S` where `S` can be
	a. ROUNDROBIN
	b. FCFS
	c. PBS
	d. MLFQ
   The default option is ROUNDROBIN.
2. We add the waitx syscalls so we can test our schedulers easily.

## FCFS
1. The idea is pretty simple, we run a loop through the process table and select the process with the minimum creation time.
2. To record creation time, we create a variable in the proc structure and initialize it with the value of the global `ticks` variable in `allocproc`.
3. In the scheduler loop, we acquire the process lock before checking if it is the best process. If it is the current best process, we do not release the lock. If we do, another CPU might mess with it.
4. Release the lock only when we find a better pick. 
5. Once we have our pick, ensure that there was at least one process there to schedule. If all is well an we have our pick, context switch to the process, thereby executing it.
6. Next, disable `yield()` in `trap.c` due to timer interrupts. This disables process preemption. This can be done with `ifdef` preprocessor macros.

## PBS
1. Again, we follow a strategy pretty similar to FCFS. In `allocprocess`, we give all processes a static priority of 60.
2. We also add variables to log the number of times a process was scheduled to run. 
3. Next, we write a function `update_time` which runs in every timer interrupt. It simply iterates through the entire process list and updates the `run_time`, `sleep_time`, etc and other variables counting time the process has spent in various states.
4. We can use a switch statement here to make the compiler optimize it to a jump table, which should be slightly faster.
5. Once we have all the variables tracked, we just implement the logic for the given formulas in the scheduler and make it pick based on the calculated DP value.
6. Like we did in spec 1, we implement a new syscall which can modify the value of the static_priority value a process has.
7. We implement this syscall and then add a simple user program to test it. Tested via `procdump` and works on `init`, `sh`, etc.

## MLFQ
1. This is much more complicated. We create a new file `queue.c` and implement the functions of a simple queue datastructure that works on static memory. `kalloc` only alloc's blocks in chunks of 4096 bytes which is not ideal and pretty inefficient for dynamic allocation. Hence we use static allocation.
2. Once all the basic, `push_back`, `pop_front` and `remove_queue` functions have been implemented, we implement the `queueInfo` struct to keep track of the queue description and declare the variables as extern so it can be used and accessed across the project.
3. Now in the MLFQ scheduler, we begin by checking for any processes in RUNNABLE state. If found and they're not already in the queue, we add them to the priority 0 queue.
4. Now, in the scheduler, we go in order from the priority 0 queue to the priority 4 queue and execute processes in it in order. If a process was found when iterating in this order, we immediately execute it.
5. Now, in the clock_interrupt function, we increment the ticks occupied by one process and if it exceeds its quota then we yield the cpu. Otherwise we let it continue.
6. One important check in the interrupt function is to make sure no process of higher priority has entered the queue. If such a process has entered the queue, we immediately yield and preempt the process out of the queue.
7. If the process relinquishes control of the CPU for any reason without using its entire timeslice, then we put it back in the same queue.
8. We implement this as follows. Before execution it is popped from queue, in `trap.c` if we find a better priority process to preempt or the process is giving up control then we push it back into the same priority queue and yield cpu. Otherwise it has finished its time slice and we push it into the next queue. 
9. After this is done, we implement aging in a similar fashion. Before pushing stuff into the queue in scheduler, check if any process has wait time > AGETICKS. If yes, move it up one priority. We can keep track of wait time again in `update_time` as implemented before.
10. `AGETICKS` is defined in `params.h` and we implement moving up a priority by removing from original queue and pushing it up a queue. 

# Procdump

1. Procdump is simple to implement. We just modify the original procdump function with `#ifdef` statements to print whatever we want from the `proc` struct in a format that we like for each of the different schedulers.

# Analysis

Upon running the schedulertest function on all the different schedulers, we got the following results.

> MLFQ:  Average rtime 14,  wtime 144	| Single CPU
> PBS:   Average rtime 15,  wtime 106	| 3 CPUs
> FCFS:  Average rtime 31,  wtime 37	| 3 CPUs
> RR:    Average rtime 12,  wtime 112	| 3 CPUs

Notice that even with all the overhead from moving things around in the queue and managing multiple queues at the same time while working on just ONE cpu, MLFQ does considerably well. 
FCFS has the least wait time, this is mainly because the process is executed as soon as it comes into the queue and the scheduler cannot preemptively execute anything.
PBS and Round Robin do reasonably well and post great results as well. The extra picking based on priority in the PBS scheduler seems to have reduced wait time considerably while taking minimum hit in runtime. This is especially good as we reduce the amount of time a process has to idle potentially.

