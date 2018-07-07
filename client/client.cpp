#include"ls_file.h"
#include"create_MD5.h"
#include"client.h"
#include"public.h"
#include"view_cli.h"
#include<iostream>
#include<string>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<json/json.h>
#include<errno.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<signal.h>
using namespace std;
//_sockfd  msg
Client::Client(char*ip,unsigned short port)
{
	login_flag = false;
	_sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(_sockfd == -1)
	{
		cerr<<"socket err"<<endl;
		return;
	}

	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	saddr.sin_addr.s_addr = inet_addr(ip);
	int res = connect(_sockfd,(struct sockaddr*)&saddr,sizeof(saddr));
	if(res == -1)
	{
		cerr<<"connect fail:errno"<<errno<<endl;
		exit(0);;
	}
	
}

void Client::start_login_register(int flag)
{
	char name[32]={0};
	cout<<"请输入用户名:"<<endl;
	while(1)
	{
		cin>>name;
		fflush(stdin);
		if(strlen(name) > 20)
		{
			cout<<"用户名太长，请重新输入!"<<endl;
			continue;
		}
		break;
	}
	cout<<"请输入密码:"<<endl;
	char pw[32] = {0};
	while(1)
	{
		cin>>pw;
		fflush(stdin);
		if(strlen(pw) > 20)
		{
			cout<<"密码太长,请重新输入(8~12位)!"<<endl;
			continue;
		}
		break;
	}
	if(flag == 0)//register
	{
		cout<<"register json"<<endl;
		Json::Value val;
		val["TYPE"] = MSG_TYPE_REGISTER;
		val["name"] = name;
		val["pw"] = pw;
		if(-1 == send(_sockfd,val.toStyledString().c_str(),strlen(val.toStyledString().c_str()),0))
		{
			cerr<<"send fail;errno:"<<errno<<endl;
			return;
		}
	}
	if(flag == 1)//login
	{
		Json::Value val;
		val["TYPE"] = MSG_TYPE_LOGIN;
		val["name"] = name;
		val["pw"] = pw;
		_name = name;
		if(-1 == send(_sockfd,val.toStyledString().c_str(),strlen(val.toStyledString().c_str()),0))
		{
			cerr<<"send fail;errno:"<<errno<<endl;
			return;
		}
	}
}
//注册功能方法
void Client::register_func()
{
	cout<<"*注 册*"<<endl;
	start_login_register(0);//argv=0,register
	char buff[1024] = {0};
	cout<<"注册中 ..."<<endl;
	if(recv(_sockfd,buff,1024,0) > 0)
	{
		if(strstr(buff,"heartbeating") != NULL)
		{
			memset(buff,0,sizeof(buff));
			recv(_sockfd,buff,buff_size,0);
		}
		char*nap = strchr(buff,'#');
		cout<<nap+1<<endl;
	}
}


//上传文件
void Client:: uploadfile()
{
	if(login_flag == false)
	{
		cout<<"请先登录！"<<endl;
		return;
	}
	upload_show();/*上传视图*/
	bool flag = true;
	while(flag)
	{
		cout<<"请选择功能:"<<endl;
		char choice[2] = {0};
		cin>>choice;
		fflush(stdin);
		if(strlen(choice) > 1)
		{
			continue;
		}
		if(choice[0] > '9' || choice[0] < '0')
		{
			continue;
		}
		int num = atoi(&choice[0]);
		
		switch(num)
		{
			case 1:/*继续上传*/
					uploadfile_continue();
					flag = false;
					break;
			case 2:/*上传文件*/
					uploadfile_new();
					flag = false;
					break;
			case 3://exit
					flag = false;
					break;
			dafult:
				flag = true;
		}
		memset(choice,0,sizeof(choice));
	}
}

void Client:: uploadfile_continue()
{
	
}

void Client::uploadfile_new()
{
	cout<<"本地可以上传的文件有:"<<endl;
	char fileinfo[buff_size] = "";
	getfile(fileinfo,buff_size);
	cout<<fileinfo<<endl;
	cout<<"输入想上传的文件名:"<<endl;
	char filename[128]=PATH;
	char cname[32] = "";
	cin>>cname;
	strcat(filename,cname);
	char filemd5[buff_size] = "";
	create_MD5(filename,filemd5,buff_size);
	/********************************/
	struct stat st;
	lstat(filename,&st);
	long filesize = st.st_size;/*filesize*/
	/*********************************/
	/*json value*/
	Json::Value val;
	val["TYPE"] = MSG_TYPE_UPLOAD;
	val["name"] = _name;
	val["filename"] = cname;
	val["filesize"] = (int)filesize;
	val["md5"] = filemd5;
	/*********************/
	if(-1 == send(_sockfd,val.toStyledString().c_str(),strlen(val.toStyledString().c_str()),0))
	{
		cerr<<"send fail;errno:"<<errno<<endl;
		return;
	}
	/*********秒传*******************/
	char ackmsg[buff_size]={0};
	if(-1 == recv(_sockfd,ackmsg,buff_size,0))
	{
		cerr<<"recv fail;errno:"<<errno<<endl;
		return;
	}
	if(strstr(ackmsg,"heartbeating") != NULL)
	{
		memset(ackmsg,0,sizeof(ackmsg));
		recv(_sockfd,ackmsg,buff_size,0);
	}
	if(strstr(ackmsg,"success") != NULL)
	{
		cout<<"upload over!"<<endl;
		return;
	}
	/***********************/
	int fd = open(filename,O_RDONLY);
	if(fd == -1)
	{
		cerr<<"open file err;errno: "<<errno<<endl;
		exit(0);
	}
	/*************************/
	const int size = 1000;
	char readbuff[size]="";
	char sendbuff[buff_size] = "";
	int res;
	int num;
	while(1)
	{
		res = read(fd,readbuff,size);
		cout<<res<<endl;
		if(res <= 0)
		{
			sprintf(sendbuff,"%d#",res);
			num = send(_sockfd,sendbuff,buff_size,0);
			if(num <= 0)
			{
				cerr<<"connection err;errno:"<<errno<<endl;
				exit(0);
			}
			close(fd);
			break;
		}
		sprintf(sendbuff,"%d#",res);
		strcat(sendbuff,readbuff);
		/********************/
		num = send(_sockfd,sendbuff,buff_size,0);
		if(num < 0)
		{
			cerr<<"connection err;errno:"<<errno<<endl;
			exit(0);
		}
		memset(sendbuff,0,sizeof(sendbuff));
		memset(readbuff,0,sizeof(readbuff));
		sleep(1);
	}
	cout<<"trans over"<<endl;
	/**************传输完毕，校验文件*****/
	char recvbuff[buff_size] = "";
	res = recv(_sockfd,recvbuff,buff_size,0);
	if(res <= 0)
	{
		cerr<<"connection err;errno: "<<errno<<endl;
		return;
	}
	if(strstr(recvbuff,"heartbeating") != NULL)
	{
		memset(recvbuff,0,sizeof(recvbuff));
		res = recv(_sockfd,recvbuff,buff_size,0);
	}
	char*nap = strchr(recvbuff,'#');
	cout<<nap+1<<endl;
}

//下载文件
void Client::downloadfile()
{
	if(login_flag == false)
	{
		cout<<"请先登录！"<<endl;
		return;
	}
	download_show();/*下载视图*/
	bool flag = true;
	while(flag)
	{
		cout<<"请选择功能:"<<endl;
		char choice[2] = {0};
		cin>>choice;
		fflush(stdin);
		if(strlen(choice) > 1)
		{
			continue;
		}
		if(choice[0] > '9' || choice[0] < '0')
		{
			continue;
		}
		int num = atoi(&choice[0]);
		
		switch(num)
		{
			case 1:/**继续下载*/
					downloadfile_continue();
					flag = false;
					break;
			case 2:/*下载文件*/
					downloadfile_new();
					flag = false;
					break;
			case 3://exit
					flag = false;
					break;
			dafult:
				flag = true;
		}
		memset(choice,0,sizeof(choice));
	}
}

void checkfile(int _sockfd,const char*name,const char*flag);
void Client::downloadfile_continue()
{ 
	checkfile(_sockfd,_name.c_str(),"breakdownload");
	/**************************************************/
	char _file[buff_size] = "";
	int res;
	res = recv(_sockfd,_file,buff_size,0);
	if(res <= 0)
	{
		cout<<"connection is err"<<endl;
		return;
	}
	if(strstr(_file,"heartbeating") != NULL)
	{
		memset(_file,0,sizeof(_file));
		recv(_sockfd,_file,buff_size,0);
	}
	char*nap = strchr(_file,'#');
	int file_size;
	sscanf(_file,"%d",&file_size);
	if(file_size == 0)
	{
		cout<<"没有可以继续下载的文件!"<<endl;
		return;
	}
	cout<<"可以继续下载的文件有:"<<endl;
	char msg[buff_size] = "";
	strncpy(msg,nap+1,file_size);
	cout<<msg<<endl;
	/******************************************************/
	char filename[32] = "";
	cout<<"选择要下载的文件:"<<endl;
	cin>>filename;
	char*ptr = strstr(msg,filename);
	if(ptr == NULL)
	{
		cout<<"文件名无效!"<<endl;
		return;
	}
	Json::Value val;
	val["TYPE"] = MSG_TYPE_B_DOWNLOAD;
	val["name"] = _name;
	val["filename"] = filename;
	if(-1 == send(_sockfd,val.toStyledString().c_str(),strlen(val.toStyledString().c_str()),0))
	{
		cerr<<"send fail;errno:"<<errno<<endl;
		return;
	}
	/*************************************************************/
	char file_name[buff_size] = PATH; 
	strcat(file_name,filename);
	int fw = open(file_name,O_APPEND);
	if(fw == -1)
	{
		cerr<<"open file fail."<<endl;
		return;
	}
	char info[buff_size] = "";
	while(1)
	{
		res = recv(_sockfd,info,buff_size,0);
		if(res <= 0)
		{
			cerr<<"connection is err;erron:"<<errno<<endl;
			close(fw);
			return;
		}
		if(strstr(info,"heartbeating") != NULL)
		{
			memset(info,0,sizeof(info));
			continue;
		}
		int size;
		sscanf(info,"%d",&size);
		if(size == 0)
		{
			close(fw);
			break;
		}
		nap = strchr(info,'#');
		char word[buff_size] = "";
		strncpy(word,nap+1,size);
		write(fw,word,size);
		memset(info,0,sizeof(info));
	}
	char md5[32] = "";
	create_MD5(file_name,md5,32);
	md5[32] = '\0';
	char sendbuff[buff_size];
	sprintf(sendbuff,"%d#",32);
	strcat(sendbuff,md5);
	send(_sockfd,sendbuff,buff_size,0);
	memset(sendbuff,0,sizeof(sendbuff));
	res = recv(_sockfd,sendbuff,buff_size,0);
	if(strstr(sendbuff,"heartbeating") != NULL)
	{
		memset(sendbuff,0,sizeof(sendbuff));
		recv(_sockfd,sendbuff,buff_size,0);
	}
	nap = strchr(sendbuff,'#');
	if(nap == NULL)
	{
		cerr<<"recv err"<<errno<<endl;
		return;
	}
	if(strcmp(sendbuff,"ok") == 0)
	{
		cout<<"下载成功!"<<endl;
	}
	else
	{
		cout<<"下载失败!"<<endl;
	}
}

void checkfile(int _sockfd,const char*name,const char*flag)
{
	Json::Value val;
	val["TYPE"] = MSG_TYPE_CHECKFILE;
	val["name"] = name;
	val["flag"] = flag;
	if(-1 == send(_sockfd,val.toStyledString().c_str(),strlen(val.toStyledString().c_str()),0))
	{
		cerr<<"connection err;errno:"<<errno<<endl;
		return;
	}
}

void Client::downloadfile_new()
{
	checkfile(_sockfd,_name.c_str(),"download");
	/******************************************/
	cout<<"你可以下载的文件有:"<<endl;
	char _file[buff_size] = "";
	int res;
	res = recv(_sockfd,_file,buff_size,0);
	if(res <= 0)
	{
		cout<<"connection is err"<<endl;
		return;
	}
	if(strstr(_file,"heartbeating") != NULL)
	{
		memset(_file,0,sizeof(_file));
		recv(_sockfd,_file,buff_size,0);
	}
	char*nap = strchr(_file,'#');
	int file_size ;
	sscanf(_file,"%d",&file_size);
	if(file_size == 0)
	{
		cout<<"你没有可以下载的文件，请先上传！"<<endl;
		return;
	}
	char msg[buff_size] = "";
	strncpy(msg,nap+1,file_size);
	cout<<msg<<endl;
	/************************************/
	char filename[32] = "";
	cout<<"选择下载的文件:"<<endl;
	cin>>filename;
	char *ptr = strstr(msg,filename);
	if(ptr == NULL)
	{
		cout<<"文件名无效"<<endl;
		return;
	}
	Json::Value val;
	val["TYPE"] = MSG_TYPE_DOWNLOAD;
	val["name"] = _name;
	val["filename"] = filename;
	if(-1 == send(_sockfd,val.toStyledString().c_str(),strlen(val.toStyledString().c_str()),0))
	{
		cerr<<"send fail;errno:"<<errno<<endl;
		return;
	}
	/**********************************************/
	char recvbuff[buff_size] = "";
	res = recv(_sockfd,recvbuff,buff_size,0);
	if(res <= 0)
	{
		cerr<<"recv err;errno:"<<errno<<endl;
		return;
	}
	if(strstr(recvbuff,"heartbeating") != NULL)
	{
		memset(recvbuff,0,sizeof(recvbuff));
		recv(_sockfd,recvbuff,buff_size,0);
	}
	nap = strchr(recvbuff,'#');
	sscanf(recvbuff,"%d",&file_size);/*file size*/
	char md5[33] = "";
	strncpy(md5,nap+1,32);/*file md5*/
	md5[33] = '\0';
	/****************************************************/
	char file[buff_size] = PATH;
	strcat(file,filename);
	int fw = open(file,O_WRONLY | O_CREAT,0600);
	if(fw == -1)
	{
		cerr<<"file create or open fail."<<endl;
		return;
	}
	int count = 0;
	int recv_num = 0;
	char info[buff_size] = "";
	cout<<"正在下载..."<<endl;
	while(1)
	{
		recv_num = recv(_sockfd,info,buff_size,0);
		if(recv_num < 0)
		{
			cerr<<"connection break;errno"<<errno<<endl;
			close(fw);
			return;
		}
		if(strstr(info,"heartbeating") != NULL)
		{
			memset(info,0,sizeof(info));
			continue;
		}
		if(recv_num == 0)
		{
			close(fw);
			return;
		}
		int size;
		sscanf(info,"%d",&size);
		if(size == 0)
		{
			close(fw);
			break;
		}
		nap = strchr(info,'#');
		write(fw,nap+1,size);
		count += size;
		memset(info,0,sizeof(info));
	}
	cout<<"trans over,start crc"<<endl;
	if(count >= file_size)
	{
		/*校验文件*/
		char md5_buff[32] = "";
		create_MD5(file,md5_buff,32);
		md5_buff[32] = '\0';
		if(strcmp(md5_buff,md5) == 0)
		{
			char msg[] = "download over!";
			int msg_len = strlen(msg);
			char _sendbuff[buff_size] = "";
			sprintf(_sendbuff,"%d#",msg_len);
			strcat(_sendbuff,msg);
			send(_sockfd,_sendbuff,buff_size,0);
			cout<<"下载成功!"<<endl;
		}
		else
		{
			char msg[] = "download fail!";
			int msg_len = strlen(msg);
			char _sendbuff[buff_size] = "";
			sprintf(_sendbuff,"%d#",msg_len);
			strcat(_sendbuff,msg);
			send(_sockfd,_sendbuff,buff_size,0);
			cout<<"下载失败!"<<endl;
		}
	}
}

//登录功能方法
void Client::load_func()
{
	if(login_flag == true)
	{
		cout<<"You already logined."<<endl;
		return;
	}
	cout<<"*登 录*"<<endl;
	bool key = true;
	bool hb = false;
	while(key)
	{
		if(hb == false)
		{
			start_login_register(1);//argv=1,login
		}
		char msg[1024] = {0};
		if(recv(_sockfd,msg,1024,0) > 0)
		{
			char*nap = strchr(msg,'#');
			cout<<nap+1<<endl;
		}
		if(strstr(msg,"heartbeating") != NULL)
		{
			hb = true;
			memset(msg,0,sizeof(msg));
			continue;
		}
		if(strstr(msg,"success") != NULL)
		{
			login_flag = true;
			return;
		}
		if(strstr(msg,"fail") != 0)
		{
			return;
		}
		else
		{
			function();//登录成功后的视图
			key = false;
		}
	}
}

static int sockfd;
void fun(int sig)
{
	cout<<"系统意外终止!"<<endl;
	Json::Value val;
	val["TYPE"] = MSG_TYPE_EXIT;
	send(sockfd,val.toStyledString().c_str(),strlen(val.toStyledString().c_str()),0);
	close(sockfd);
	cout<<"退出成功!"<<endl;
	exit(1);
}

//退出功能方法
void Client::exit_func()
{
	cout<<"*退 出*"<<endl;
	sockfd = _sockfd;
	Json::Value val;
	val["TYPE"] = MSG_TYPE_EXIT;
	send(sockfd,val.toStyledString().c_str(),strlen(val.toStyledString().c_str()),0);
	close(sockfd);
	cout<<"退出成功!"<<endl;
	exit(1);
}

void Client::run()
{
	//创建第二个线程，用于监听服务器消息
	//此处调用视图完成功能呈现
	function();
	signal(SIGINT,fun);//客户端意外终止
	signal(SIGQUIT,fun);//与上面情况相同
	bool flag = true;
	while(flag)
	{
		cout<<"请选择功能:"<<endl;
		char choice[2] = {0};
		cin>>choice;
		fflush(stdin);
		if(strlen(choice) > 1)
		{
			continue;
		}
		if(choice[0] > '9' || choice[0] < '0')
		{
			continue;
		}
		int num = atoi(&choice[0]);
		
		switch(num)
		{
			case 1://register
					register_func();
					flag = false;
					break;
			case 2://upload
					load_func();
					flag = false;
					break;
			case 3://upload
					uploadfile();
					flag = false;
					break;
			case 4://download
					downloadfile();
					flag = false;
					break;
			case 5://exit
					exit_func();
					flag = false;
					break;
			dafult:
				flag = true;
		}
		memset(choice,0,sizeof(choice));
	}
}
