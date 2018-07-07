#include"mpthread.h"
#include<json/json.h>
#include"view_exit.h"
#include"view.h"
#include<pthread.h>
#include<sys/time.h>
#include<signal.h>
using namespace std;

void *pth_run(void *arg);
/*设置自定义的信号，心跳包*/
/*
void heart_beat(int)
{
	cout<<"heart beat"<<endl;
}
void init_sigaction()
{
	struct sigaction act;
	act.sa_handler = heart_beat;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	sigaction(SIGPROF,&act,NULL);
}
*/
void init_timer()
{
	struct itimerval val;
	val.it_value.tv_sec = 10;//10 seconds
	val.it_value.tv_usec = 0;//0 microseconds

	val.it_interval = val.it_value;
	setitimer(ITIMER_PROF,&val,NULL);
}

Mpthread::mpthread(int sock_1)
{
	Mcontral = new Contral;
	_sock_1 = sock_1;
	pthread_t id;
	if(pthread_create(&id,NULL,pth_run,this) != 0)
	{
		cerr<<"pthread_create err"<<endl;
		return;
	}
	/*********************************************/
	/*注册心跳包信号*/
//	init_sigaction();
//	init_timer();
}
void cli_cb(int fd,short event,void *arg)
{
	Pmpthread mthis = (Pmpthread)arg;
	char buff[1024] = {0};
	int recv_num = recv(fd,buff,sizeof(buff)-1,0);
	if(recv_num > 0)
	{
		mthis->Mcontral->process(buff,fd);
	}
	else if(recv_num == 0)
	{
		/*client is close*/
		cout<<"client "<<fd<<" is closed"<<endl;
		event_del(mthis->_event_map[fd]);
		mthis->_event_map.erase(fd);
		event_del(mthis->_hb_map[fd]);
		mthis->_hb_map.erase(fd);
		view_exit ex;/*正常退出*/
		Json::Value val;
		ex.process(val,fd);
		return;
	}
	else
	{
		cout<<"client "<<fd<<" is err closed"<<endl;
		view_exit ex;/*异常退出 ???*/
		Json::Value val;
		ex.process(val,fd);
		return;
	}
}

typedef struct HB{
	int fd;
	Pmpthread mthis;
	HB(int _fd = 0,Pmpthread _mthis = Pmpthread()):fd(_fd),mthis(_mthis){}
}*PHB;


void heart_beat(int fd,short event,void *arg)
{
	PHB pcur = (PHB)arg;
	int cli_fd = pcur->fd;
	Pmpthread mthis = pcur->mthis;

	char sendbuff[buff_size] = "";
	char msg[] = "heartbeating";
	sprintf(sendbuff,"%d#",msg);
	strcat(sendbuff,msg);
	int res = 1;
	res = send(cli_fd,sendbuff,buff_size,0);
	if(res <= 0)
	{
		event_del(mthis->_event_map[fd]);
		mthis->_event_map.erase(fd);
		event_del(mthis->_hb_map[fd]);
		mthis->_hb_map.erase(fd);
		return;
	}
	cout<<"heart_beating!"<<"client "<<cli_fd<<"is ok!"<<endl;
}


void sock_1_cb(int fd,short event,void *arg)
{
	Pmpthread mthis = (Pmpthread)arg;
	int cli_fd;
	if(recv(fd,&cli_fd,sizeof(cli_fd),0) == -1)
	{
		cerr<<"recv cli_fd err"<<endl;
		return ;
	}
	/*将cli_fd添加到libevent->cli_cb()*/
	struct event*ev = event_new(mthis->_base,cli_fd,EV_READ|EV_PERSIST,cli_cb,(void*)mthis);
	event_add(ev,NULL);
	mthis->_event_map.insert(make_pair(cli_fd,ev));
	int num = mthis->_event_map.size();/*统计当前线程管理的客户端的数目*/
	send(fd,&num,sizeof(num),0);/*将num发送给主线程接收各个线程任务量的双向管道*/
	/*****************************************************************************/
	/*注册心跳检测事件*/
	PHB hb_packet  = new HB(cli_fd,mthis);
	timeval tv = {10,0};
	struct event *hb = event_new(mthis->_base,cli_fd,EV_TIMEOUT|EV_PERSIST,heart_beat,(void*)hb_packet);
	event_add(hb,&tv);
	mthis->_hb_map[cli_fd] = hb;
}

void*pth_run(void*arg)
{
	Pmpthread mthis = (Pmpthread)arg;
	mthis->_base = event_init();

	struct event*ev = event_new(mthis->_base,mthis->_sock_1,EV_READ|EV_PERSIST,sock_1_cb,mthis);
	event_add(ev,NULL);
	event_base_dispatch(mthis->_base);
}
