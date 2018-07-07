#include<iostream>
#include<errno.h>
#include<string.h>
#include<stdlib.h>
#include"client.h"
#include<signal.h>
#define max 20
using namespace std;

int main(int argc,char**argv)
{
	signal(SIGPIPE,SIG_IGN);
	if(argc < 3)
	{
		cerr<<"arg not enough;errno:"<<errno<<endl;
		exit(1);
	}
	//解析参数 ip  port  
	char ip[max]={0};
	strcpy(ip,argv[1]);
	unsigned short port = (unsigned short)atoi(argv[2]);
	//构造客户端对象
	Client cli(ip,port);
	//运行客户端
	while(1)
	{
		cli.run();
	}
	return 0;
}
