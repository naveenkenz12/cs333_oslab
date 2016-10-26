#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <errno.h>
#include <time.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

char server_ip[100];				//ip address of server
char server_port[5];				//port of server

int rpid=-1;						//id of running process
int rpid2=-1;						//id of running process
int bgp;							//number of background process
int bpid=0;							//group id for background for process

int pl_array[10000];				//id of process running in parallel
int pl_process=0;					//number of process running in parallel


char **tokenize(char *line)
{
	char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
	char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
	int i, tokenIndex = 0, tokenNo = 0;

	for(i =0; i < strlen(line); i++)
	{
		char readChar = line[i];

		if (readChar == ' ' || readChar == '\n' || readChar == '\t')
		{
			token[tokenIndex] = '\0';
			if (tokenIndex != 0)
			{
				tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
				strcpy(tokens[tokenNo++], token);
				tokenIndex = 0; 
			}
		}
		else
		{
			token[tokenIndex++] = readChar;
		}
  	}
 	free(token);
	tokens[tokenNo] = NULL ;
	return tokens;
}

void sigintHandler(int sig_num)		//signal handler
{
	if(sig_num == SIGCHLD)			//notify when background process finishes
	{	
		
    	int r;
    	r=waitpid(-bpid,0,WNOHANG);
      	while((r=waitpid(-1,0,WNOHANG))>0)
      	{
        	printf("Background process %d finished\n",r);
        	r=waitpid(-bpid,0,WNOHANG);
        	bgp--;
      	}
	}
	else							//signal gandler for sigint(ctrl+c)
	{
		if(rpid!=-1)				//if process running kill it
		{
			kill(-rpid,SIGINT);
			printf("killed %d\n",rpid);
			rpid=-1;
		}
		if(rpid2!=-1)				//in case of | 2 process will be running, rpid, rpid2, kill both the process
		{
			kill(-rpid2,SIGINT);
			printf("killed %d\n",rpid2);
			rpid=-1;
		}
		if(pl_process>0)			//kill all parallel process if num of parallel process > 0
		{
			for(int k=0;k<pl_process;k++)
			{
				kill(-pl_array[k],SIGINT);
				printf("killed %d\n",pl_array[k]);
			}
			pl_process=0;			//reset count of parallel process running to 0
		}
	}
}	



void  main(void)
{
	signal(SIGINT, sigintHandler);	//signal handler call
	signal(SIGCHLD, sigintHandler);

	char line[MAX_INPUT_SIZE];            
	char **tokens;              
	int i;
	
	char hello[100];				//message in start of each line of terminal
	strcpy(hello, "Hello>");
	
	strcpy(server_port, "0000");	//initialize server port with 0000, 

	while(1)
	{       
		printf("%s",hello);     
		bzero(line, MAX_INPUT_SIZE);
		gets(line);           
		line[strlen(line)] = '\n'; 	//terminate with new line
		tokens = tokenize(line);
   				
		int len_tokens = 0;
		char command[30];

		for(i=0;tokens[i]!=NULL;i++)
		{
			if(i==0)strcpy(command, tokens[0]);
			len_tokens++;
		}
       	
		if(strcmp(command,"cd")==0)	//cd
		{
			if(len_tokens!=2)printf("Incorrect number of arguements\n");
			else
			{
				char new_dir[150];
				
				if(tokens[1][0]=='~')
				{
					strcpy(new_dir, getenv("HOME"));
					for(int i=1;tokens[1][i-1]!='\0';i++)tokens[1][i-1]=tokens[1][i];
					strcat(new_dir, tokens[1]);
				}
				else
				{
					strcpy(new_dir, tokens[1]);
				}
				//printf("%s\n",new_dir);
				int x=chdir(new_dir);
				//printf("%d",x);
				if(x==-1)printf("Error changing Directory\n");
				char cwd[1024];
			   	
				if(x!=-1){if(getcwd(cwd, sizeof(cwd)) != NULL)strcpy(hello,cwd);}
				if(x!=-1)strcat(hello,"@Hello>");
				
			}
		}
		else if(strcmp(command,"server")==0)		//serever detail saving
		{
			if(len_tokens!=3)printf("Incorrect number of arguements\n");
			else
			{
				strcpy(server_ip, tokens[1]);
				strcpy(server_port, tokens[2]);
			}
		}
		else if(strcmp(command,"getfl")==0)			//getfl
		{
			if(len_tokens==2)
			{
				if(len_tokens!=2)printf("Incorrect number of arguements\n");
				else if(strcmp(server_port, "0000")==0)printf("server details not present, run server <ip> <port>\n");
				else
				{
					int ret=fork();
					
					if(ret==-1)printf("Process creation failed");
					if(ret==0)
					{
						execl("./get-one-file-sig", "get-one-file-sig", tokens[1], server_ip, server_port, "display", (char *)0);
						exit(1);
					}
					if(ret>0)
					{
						rpid=ret;
						waitpid(ret,0,0);
						rpid=-1;
					}
				}
			}
			else if(strcmp(tokens[2],">")==0)			//output redirection to file
			{
				if(len_tokens!=4)printf("Incorrect number of arguements\n");
				else if(strcmp(server_port, "0000")==0)printf("server details not present, run server <ip> <port>\n");
				else
				{
					int ret=fork();
					
					if(ret==-1)printf("Process creation failed");
					if(ret==0)
					{
						int fd = open(tokens[3], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
						dup2(fd, 1);
						close(fd);
						execl("./get-one-file-sig", "get-one-file-sig", tokens[1], server_ip, server_port, "display", (char *)0);
						exit(1);
					}
					if(ret>0)
					{
						rpid=ret;
						waitpid(ret,0,0);
						rpid=-1;
					}
				}
			}
			else if(strcmp(tokens[2],"|")==0)			//piping
			{
				if(len_tokens<=3)printf("Incorrect number of arguements\n");
				else if(strcmp(server_port, "0000")==0)printf("server details not present, run server <ip> <port>\n");
				else
				{
					if(!strcmp(tokens[2],"|"))
					{
		            	pid_t ret1, ret2;
		            	int p[2];
		            	pipe(p);

		            	ret1 = fork();
		            	if(ret1==0)
		            	{
		              		dup2(p[1],1);
		              		close(p[0]);
		              		close(p[1]);
		              		execl("./get-one-file-sig","get-one-file-sig",tokens[1],server_ip,server_port,"display",(char *)0);
		              		exit(1);
		            	}
		            	ret2 = fork();
		            	if(ret2==0)
		            	{
		              		dup2(p[0],0);
		              		close(p[0]);
		              		close(p[1]);
		              		char** instr[MAX_NUM_TOKENS];
		              		//strcopy
		              		int j=3;
		              		while(tokens[j]!=NULL)
		              		{
		              			instr[j-3] = tokens[j];
		              			j++;
		              		}
		              		instr[j-3] = NULL;
		              		int value = execvp(tokens[3],instr);
		              		if(value == -1)
		              		{
		                		fprintf(stderr,"%s\n",strerror(errno) );
		              			exit(0);
		          	  		}
		              		exit(0);
		            	}
		            	close(p[0]);
		            	close(p[1]);
		        
		            	rpid=ret1;
		            	rpid2=ret2;
		          		waitpid(ret1,NULL,0);
		          		rpid=-1;
		            	
		          		waitpid(ret2,NULL,0);
		          		rpid2=-1;
		            	
		          		continue;
		          	}
				}
			}
		}
		else if(strcmp(command,"getsq")==0)				//sequential file download
		{
			if(len_tokens<2)printf("Incorrect number of arguements\n");
			else if(strcmp(server_port, "0000")==0)printf("server details not present, run server <ip> <port>\n");
			else
			{
				int ret;
				for(int num_d =0;num_d<len_tokens-1;num_d++)
				{
					ret = fork();
					if(ret==-1)printf("Process creation failed");
					if(ret==0)
					{
						execl("./get-one-file-sig", "get-one-file-sig", tokens[num_d+1], server_ip, server_port, "nodisplay", (char *)0);
						exit(1);
					}
					if(ret>0)
					{
						rpid=ret;
						waitpid(ret,0,0);
						rpid=-1;
					}
				}
			}
		}
		else if(strcmp(command,"getpl")==0)				//parallel file download
		{
			pl_process=0;
			if(len_tokens<2)printf("Incorrect number of arguements\n");
			else if(strcmp(server_port, "0000")==0)printf("server details not present, run server <ip> <port>\n");
			else
			{
				int ret;
				for(int num_d =0;num_d<len_tokens-1;num_d++)
				{
					ret = fork();
					if(ret==-1)printf("Process creation failed");
					if(ret==0)
					{
						execl("./get-one-file-sig", "get-one-file-sig", tokens[num_d+1], server_ip, server_port, "nodisplay", (char *)0);
						//exit(1);
					}
					else
					{
						pl_array[pl_process]=ret;
						pl_process++;
					}
				}
				for(int num_d=0;num_d<len_tokens;num_d++){wait(NULL);pl_process--;}
			}
		}
		else if(strcmp(command,"getbg")==0)				//background file download
		{
			if(len_tokens!=2)printf("Incorrect number of arguements\n");
			else if(strcmp(server_port, "0000")==0)printf("server details not present, run server <ip> <port>\n");
			else
			{
				int ret=fork();
				if(ret==0)
				{
					execl("./get-one-file-sig", "get-one-file-sig", tokens[1], server_ip, server_port, "nodisplay", (char *)0);
					exit(1);
				}
				else
				{
					bgp++;
					bpid=ret;
					continue;
				}
			}
		}
		else if(strcmp(command,"exit")==0)				//exit
		{
			if (bpid!=0)kill(-bpid,SIGINT);				//kill all background process and exit
      		if (bgp >0)
      		{
      			int x;
        		while((x=waitpid(-1,0,WNOHANG))>0)
        		{
          			printf("Background process %d finished\n",x);
          			bgp--;
        		}
      		}
			exit(0);
		}
		else if(len_tokens>=1)							//default terminal command
		{
			int ret=fork();
			if(ret==-1)
			{
				printf("process creation failed");
			}
			if(ret==0)
			{
				execvp(tokens[0], tokens);
				exit(1);
			}
			if(ret>0)
			{
				rpid=ret;
				waitpid(ret,0,0);
				rpid=-1;
				continue;;
			}
		}
		
		for(i=0;tokens[i]!=NULL;i++)					// Freeing the allocated memory	
		{
			free(tokens[i]);
		}
		free(tokens);
	}
}

                
