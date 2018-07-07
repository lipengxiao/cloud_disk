#ifndef _MPTHREAD_H
#define _MPTHREAD_H
#include<iostream>
#include<map>
#include<event.h>
#include "contral.h"
#include<string.h>
typedef class mpthread
{
	public:
		mpthread(int sock_1);
		~mpthread(){}
	private:
		int _sock_1;//socketpair_1
		struct event_base*_base;
		map<int,struct event*> _event_map;/*保存事件的map*/
		map<int,struct event*> _hb_map;/*保存文件描述符对应的超时事件*/
		Contral *Mcontral;//控制台
		friend void *pth_run(void*arg);
		friend void sock_1_cb(int fd,short event,void*arg);
		friend void cli_cb(int fd,short event,void*arg);
		friend void heart_beat(int fd,short event,void *arg);
}Mpthread,*Pmpthread;

#endif
