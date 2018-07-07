#include"create_MD5.h"
#include"ls_file.h"
#include"public.h"
#include<sys/types.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<string.h>
using namespace std;

void getfile(char*buff,int size_buff)
{
	if(size_buff < 1)
	{
		return;
	}
	int pipefd[2];
	pipe(pipefd);
	pid_t pid = fork();
	if(pid == 0)
	{
		close(pipefd[0]);
		dup2(pipefd[1],1);
		dup2(pipefd[1],2);
		execlp("ls","ls",PATH,0);
		cerr<<"execlp err";
		close(pipefd[1]);
		exit(0);
	}
	close(pipefd[1]);
	wait(NULL);
	char *readbuff = NULL;
	read(pipefd[0],buff,size_buff);
	char*p = NULL;
	readbuff = strtok_r(buff," ",&p);
	strcpy(buff,readbuff);
	close(pipefd[0]);
}
