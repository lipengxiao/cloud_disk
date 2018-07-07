#include"view_exit.h"
#include<string.h>
#include<mysql/mysql.h>
#include<iostream>
#include<sys/socket.h>
#include<json/json.h>
#include<errno.h>
#include<stdio.h>
using namespace std;

void view_exit::process(Json::Value val,int cli_fd)
{
	cout<<cli_fd<<" client over!"<<endl;
	_cli_fd = cli_fd;
	MYSQL*mpcon = mysql_init((MYSQL*)0);
	MYSQL_RES *mp_res;
	MYSQL_ROW mp_row;
	if(!mysql_real_connect(mpcon,"127.0.0.1","root","123456",NULL,3306,NULL,0))
	{
		cerr<<"SQL connect fail ."<<endl;
		return;
	}
	if(mysql_select_db(mpcon,"cloud_disk"))
	{
		cerr<<"select fail !"<<endl;
		return;
	}
	char cmd[256] = "delete from online where fd='";
	int cmd_len = strlen(cmd);
	sprintf(cmd+cmd_len,"%d",cli_fd);
	strcat(cmd,"';");
	if(mysql_real_query(mpcon,cmd,strlen(cmd)))
	{
		cerr<<"online fail!"<<endl;
		return;
	}
	close(cli_fd);
}
