#include<iostream>
#include<string>
#include<json/json.h>
#include<errno.h>
#include<mysql/mysql.h>
#include<string.h>
#include<sys/socket.h>
#include"view.h"
#include"view_login.h"
#include"public.h"
#include<stdio.h>
using namespace std;

void view_login::process(Json::Value val,int cli_fd)
{
	_cli_fd = cli_fd;
	MYSQL *mpcon = mysql_init((MYSQL*)0);
	MYSQL_RES *mp_res;
	MYSQL_ROW mp_row;
	/*connect DBMS*/
	if(!mysql_real_connect(mpcon,"127.0.0.1","root","123456",NULL,3306,NULL,0))
	{
		cerr<<"sql connect fail;errno: "<<errno<<endl;
		return;
	}
	/*connect sql DB*/
	if(mysql_select_db(mpcon,"cloud_disk"))
	{
		cerr<<"select fail;errno: "<<errno<<endl;
		return;
	}
	/*visit user table*/
	char name[32] = {0};
	strcpy(name,val["name"].asString().c_str());
	char cmd[128] = "select * from user where name='";
	char buff[3] = "';";
	strcat(cmd,name);
	strcat(cmd,buff);
	
	if(mysql_real_query(mpcon,cmd,strlen(cmd)))
	{
		cerr<<"usr fail;errno: "<<errno<<endl;
		return;
	}
	mp_res = mysql_store_result(mpcon);
	mp_row = mysql_fetch_row(mp_res);
	if(mp_row == 0)
	{
		char msg[] = "login fail, user name is invalid,try again!";
		int str_len = strlen(msg);
		char sendbuff[64] = {0};
		sprintf(sendbuff,"%d#",str_len);
		strcat(sendbuff,msg);
		send(_cli_fd,sendbuff,buff_size,0);
		return;
	}
	if(strcmp(mp_row[1],val["pw"].asString().c_str()) != 0)
	{
		char msg[] = "login fail, user pw is invalid,try again!";
		int str_len = strlen(msg);
		char sendbuff[64] = {0};
		sprintf(sendbuff,"%d#",str_len);
		strcat(sendbuff,msg);
		send(_cli_fd,sendbuff,buff_size,0);
		return;
	}
	/*visit online to test user is online,now?,if not should insert online with name & fd;
	 *if is,should send err.
	 * */
	memset(cmd,0,sizeof(cmd));
	strcpy(cmd,"select * from online where name='");
	strcat(cmd,name);
	strcat(cmd,"';");
	if(mysql_real_query(mpcon,cmd,strlen(cmd)))
	{
		cerr<<"online is err;errno: "<<errno<<endl;
		return;
	}
	mp_res = mysql_store_result(mpcon);
	mp_row = mysql_fetch_row(mp_res);
	if(mp_row != 0)
	{
		char msg[] = "login fail, user is online ,not login again!";
		int str_len = strlen(msg);
		char sendbuff[64] = {0};
		sprintf(sendbuff,"%d#",str_len);
		strcat(sendbuff,msg);
		send(_cli_fd,sendbuff,buff_size,0);
		return;
	}
	char login_buff[128] = "insert into online value('";
	strcat(login_buff,name);
	strcat(login_buff,"','");
	int login_buff_len = strlen(login_buff);
	sprintf(login_buff+login_buff_len,"%d",cli_fd);
	strcat(login_buff,"');");		
	if(mysql_real_query(mpcon,login_buff,strlen(login_buff)))
	{
		cerr<<"insert online is err;errno: "<<errno<<endl;
		return;
	}
	
	/*login in ,ok*/
	char msg[] = "login success!";
	int str_len = strlen(msg);
	char sendbuff[32] = {0};
	sprintf(sendbuff,"%d#",str_len);
	strcat(sendbuff,msg);
	send(_cli_fd,sendbuff,buff_size,0);
}
