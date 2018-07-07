#include"public.h"
#include"view_download.h"
#include"view_checkfile.h"
#include<string.h>
#include<mysql/mysql.h>
#include<iostream>
#include<sys/socket.h>
#include<json/json.h>
#include<errno.h>
#include<stdio.h>
using namespace std;

void view_checkfile::process(Json::Value val,int cli_fd)
{
	_cli_fd = cli_fd;
	char name[16] = "";
	strcpy(name,val["name"].asString().c_str());
	char usrbuff[buff_size] = "";
	char flag[16] = "";
	strcpy(flag,val["flag"].asString().c_str());
	user_file(name,usrbuff,buff_size,flag);
	/*****************************************/
	char sendbuff[buff_size] = "";
	sprintf(sendbuff,"%d#",strlen(usrbuff));
	strcat(sendbuff,usrbuff);
	int res = send(_cli_fd,sendbuff,buff_size,0);
	if(res <= 0)
	{
		cerr<<"send err;"<<errno<<endl;
		return;
	}
}
