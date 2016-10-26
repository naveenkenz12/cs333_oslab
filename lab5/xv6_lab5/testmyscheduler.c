//new_adding_start
#include "types.h"
#include "user.h"
#include "syscall.h"
#include "memlayout.h"

#include "param.h"
#include "stat.h"
#include "fs.h"
#include "fcntl.h"
#include "traps.h"
#include "memlayout.h"

int 
main(int argc , char * argv[])
{
	printf(1,"\n\nCPU BOUND PROCESS\n\n");
	
	int i;
	for(i=0;i<5;i++)
	{
		int x=fork();
		if(x==0)
		{
			int prio = setprio(10+20*i);
			printf(1,"Child %d priority = %d\n",i,prio);
			volatile long int j=0;
			long int k=0;
			int start_time=uptime();
			printf(1,"child %d start time = %d\n",i,start_time);
			for(j=0;j<1000000000;j++)k++;
			int end_time=uptime();
			printf(1,"child %d end time = %d\n",i,end_time);
			printf(1,"child %d total time = %d\n",i,end_time-start_time);
			exit();
		}
	}
	wait();
	wait();
	wait();
	wait();
	wait();
	
	printf(1,"\n\nI/O BOUND PROCESS\n\n");
	for(i=5;i<10;i++)
	{
		int f = fork();
		if(f==0)
		{
			int prio=setprio(10+20*(i-5));
			printf(1,"child %d priority = %d\n",i,prio);
			char buff[2048];
			int start_time=uptime();
			printf(1,"child %d start time = %d\n",i,start_time);
			volatile long int j;
			for(j=0;j<20000;j++)
			{
				int fd = open("foo0.txt", O_RDONLY);
				int n = 0;
				do
				{
					n = read(fd, buff, 2048);
				}
				while(n>0);
			}
			int end_time=uptime();
			printf(1,"child %d end time = %d\n",i,end_time);
			printf(1,"child %d total time = %d\n",i,end_time-start_time);
			exit();
		}
	}
	wait();
	wait();
	wait();
	wait();
	wait();

	exit();
}

//new_adding_end