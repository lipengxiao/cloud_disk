#ifndef _TCPSERVER_H
#define _TCPSERVER_H

#include<iostream>
#include<vector>
#include<map>
#include<sys/epoll.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<event.h>
#include<unistd.h>
#include<time.h>
#include<functional>
#include<algorithm>
#include<string.h>
using namespace std;

class Sockpair
{
	public:
		Sockpair(int sockpair0,int sockpair1);
		int _sockpair0;
		int _sockpair1;
};

typedef class Tcpserver
{
	public:
		Tcpserver(char*ip,unsigned short port,int pth_num);
		~Tcpserver(){}
		void run();
		void create_socket_pair();
		void create_pth();
	private:
		struct event_base*_base;//libevent
		int _listenfd;//listenfd
		int _pth_num;/*线程个数*/
		vector<Sockpair> _sockpair_base;
		map<int,int> _pth_num_map;/*线程监听数量的map表*/
		friend void sock_0_cb(int fd,short event,void*arg);
		friend void listen_cb(int fd,short event,void*arg);
}*Ptcpserver;

#endif
