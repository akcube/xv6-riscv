#include "param.h"

struct queueInfo {
	int size[5];
	int back[5];
	int max_ticks[5];
};

extern struct queueInfo queueInfo;
extern struct proc *sched_queue[5][NPROC];