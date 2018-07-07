#include "tcpserver.h"
#include "mpthread.h"
#include<arpa/inet.h>
using namespace std;

 Sockpair::Sockpair(int sockpair0,int sockpair1):_sockpair0(sockpair0),_sockpair1(sockpair1){}

Tcpserver::Tcpserver(char*ip,unsigned short port,int pth_num)
{
	int listenfd = socket(AF_INET,SOCK_STREAM,0);
	if(listenfd == -1)
	{
		cout<<"socket err"<<endl;
		return;
	}
	struct sockaddr_in saddr;
	memset(&saddr,0,sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	saddr.sin_addr.s_addr = inet_addr(ip);

	int res = bind(listenfd,(struct sockaddr*)&saddr,sizeof(saddr));
	if(res == -1)
	{
		return;
	}
	listen(listenfd,5);
	_listenfd = listenfd;
	_pth_num = pth_num;
	/*初始化libevent*/
	_base = event_base_new();
}

void Tcpserver::create_pth()
{
	for(int i=0;i<_pth_num;++i)
	{
		new Mpthread(_sockpair_base[i]._sockpair1);
	}
}

void listen_cb(int fd,short event,void*arg)
{
	Ptcpserver mthis = (Ptcpserver)arg;
	int listenfd = mthis->_listenfd;
	struct sockaddr_in caddr;
	int len = sizeof(caddr);
	//accept
	int cli_fd = accept(listenfd,(struct sockaddr*)&caddr,(socklen_t*)&len);
	if(cli_fd < 0)
	{
		return;
	}
	cout<<"client "<<cli_fd<<" connected!"<<endl;
	map<int,int>::iterator it = mthis->_pth_num_map.begin();
	int num = it->second;
	map<int,int>::iterator tmp = it;
	while(it != mthis->_pth_num_map.end())
	{
		if(it->second < num)
		{
			num = it->second;
			tmp = it;
		}
		++it;
	}
	/*通知子线程处理这个客户端连接*/
	int res = send(tmp->first,&cli_fd,sizeof(cli_fd),0);
	if(res < 0)
	{
		cout<<"send thread err"<<endl;
		return;
	}
}

void Tcpserver::run()
{
	create_socket_pair();
	create_pth();
	struct event*ev = event_new(_base,_listenfd,EV_READ|EV_PERSIST,listen_cb,(void*)this);
	event_add(ev,NULL);
	event_base_dispatch(_base);
}

void sock_0_cb(int fd,short event,void*arg)
{
	map<int,int> *_pth_num_map = (map<int,int>*)arg;

	int num =0;
	if((recv(fd,&num,sizeof(num),0)) == -1)
	{
		cerr<<"recv err"<<endl;
		return;
	}
	/*update map*/
	(*_pth_num_map)[fd] = num;
}

void Tcpserver::create_socket_pair()
{
	for(int i=0;i<_pth_num;++i)
	{
		int _pipe[2];
		if((socketpair(AF_UNIX,SOCK_STREAM,0,_pipe)) == -1)
		{
			cerr<<"socketpair err"<<endl;
			return;
		}

		_sockpair_base.push_back(Sockpair(_pipe[0],_pipe[1]));
		_pth_num_map.insert(make_pair(_pipe[0],0));

		struct event*ev = event_new(_base,_pipe[0],EV_READ | EV_PERSIST,sock_0_cb,(void*)&_pth_num_map);
		event_add(ev,NULL);
	}
}
