//lab_6_modification_start
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

int glob;

int
main()
{
	printf(1,"Parent start: no. of free page = %d\n",getNumFreePages());
	if(fork()==0)
	{
		printf(1,"Child1: no. of free page just after fork = %d\n",getNumFreePages());
		glob = 15;
		printf(1,"Child1: no. of free page after some write = %d\n",getNumFreePages());
		exit();
	}
	if(fork()==0)
	{
		printf(1,"Child2: no. of free page just after fork = %d\n",getNumFreePages());
		glob = 25;
		printf(1,"Child2: no. of free page after some write = %d\n",getNumFreePages());
		exit();
	}
	wait();
	wait();
	printf(1,"Parent end: no. of free page = %d\n",getNumFreePages());
	exit();
	return 0;
}

//lab_6_modification_end