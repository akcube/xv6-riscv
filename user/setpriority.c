#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]){
	if(argc < 2){
	  fprintf(2, "usage: setpriority priority pid\n");
  	  exit(1);
	}

	int priority = atoi(argv[1]);
	int pid = atoi(argv[2]);
	
	set_priority(priority, pid);

	exit(0);
}