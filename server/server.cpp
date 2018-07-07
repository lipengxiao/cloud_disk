#include"tcpserver.h"
#include<iostream>

using namespace std;


int main(int argc,char*argv[])
{
	if(argc < 4)
	{
		cout<<"argv at least 4,ip,port,thread_num"<<endl;
		return 0;
	}
	char *ip = argv[1];
	int port = atoi(argv[2]);
	int thread_number = atoi(argv[3]);

	Tcpserver ser(ip,port,thread_number);
	cout<<"ser created"<<endl;
	ser.run();
	return 0;
}
