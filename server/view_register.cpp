#include"public.h"
#include"view_register.h"
#include<iostream>
#include<string>
#include<json/json.h>
#include<errno.h>
#include<mysql/mysql.h>
#include<sys/socket.h>
#include<string.h>
#include<stdio.h>
using namespace std;

void view_register::process(Json::Value val,int cli_fd)
{
	_cli_fd = cli_fd;
	MYSQL *mpcon = mysql_init((MYSQL*)0);
	MYSQL_RES *mp_res;
	MYSQL_ROW mp_row;
	//connect mysql dbms
	if(!mysql_real_connect(mpcon,"127.0.0.1","root","123456",NULL,3306,NULL,0))
	{
		cerr<<"sql connect fail;errno: "<<errno<<endl;
		return;
	}
	//connect db
	if(mysql_select_db(mpcon,"cloud_disk"))
	{
		cerr<<"select fail: errno: "<<errno<<endl;
		return;
	}
	char name[128] = {0};
	strcpy(name,val["name"].asString().c_str());
	char cmd[256] = "select * from user where name='";
	char buff[3] = "';";
	strcat(cmd,name);
	strcat(cmd,buff);
	//visit user table
	if(mysql_real_query(mpcon,cmd,strlen(cmd)))
	{
		cerr<<"name query fail;errno: "<<errno<<endl;
		return;
	}
	mp_res = mysql_store_result(mpcon);
	mp_row = mysql_fetch_row(mp_res);
	if(mp_row != 0)
	{
		char msg[] = "register fail!";
		int str_len = strlen(msg);
		char buff[32] = {0};
		sprintf(buff,"%d#",str_len);
		strcat(buff,msg);/*str_len#register...*/
		send(_cli_fd,buff,buff_size,0);
		return;
	}
	/*start register*/
	char cmd_insert[128] = "insert into user values('";
	char pw[24] = {0};
	strcpy(pw,val["pw"].asString().c_str());
	strcat(cmd_insert,name);
	strcat(cmd_insert,"','");
	strcat(cmd_insert,pw);
	strcat(cmd_insert,"');");
	if(mysql_real_query(mpcon,cmd_insert,strlen(cmd_insert)))
	{
		cerr<<"insert query fail;errno:"<<errno<<endl;
		return;
	}
	char msg[] = "register success!";
	int str_len = strlen(msg);
	char sendbuff[32] = {0};
	sprintf(sendbuff,"%d#",str_len);
	strcat(sendbuff,msg);/*str_len#register...*/
	send(_cli_fd,sendbuff,buff_size,0);
}
