#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]){
	if(argc < 3){
	  fprintf(2, "usage: strace mask command [args ...]\n");
  	  exit(1);
	}

	char *command = argv[2];
	int mask = atoi(argv[1]);
	
	trace(mask);
	exec(command, &argv[2]);

	exit(0);
}