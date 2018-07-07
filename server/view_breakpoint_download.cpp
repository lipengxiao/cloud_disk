#include"breakpoint_transport.h"
#include"view_breakpoint_download.h"
#include"public.h"
#include"view.h"
#include"create_MD5.h"
#include<json/json.h>
#include<errno.h>
#include<mysql/mysql.h>
#include<string.h>
#include<stdio.h>
#include<sys/stat.h>
#include<stdlib.h>

using namespace std;

void view_breakpoint_download::process(Json::Value val,int cli_fd)
{
	_cli_fd = cli_fd;
	MYSQL*mpcon = mysql_init((MYSQL*)0);
	MYSQL_RES *mp_res;
	MYSQL_ROW mp_row;
	if(!mysql_real_connect(mpcon,"127.0.0.1","root","123456",NULL,3306,NULL,0))
	{
		cerr<<"sql connect fail;errno: "<<errno<<endl;
		return;
	}
	if(mysql_select_db(mpcon,"breaktrans"))
	{
		cerr<<"select database fail;errno: "<<errno<<endl;
		return;
	}
	/****************************************************************/
	char filename[64] = {0};
	strcpy(filename,val["filename"].asString().c_str());
	char cmd[128] = "select * from breaktrans where(flag='download' and filename='";
	strcat(cmd,filename);
	strcat(cmd,"' and name ='");
	strcat(cmd,val["name"].asString().c_str());
	strcat(cmd,"');");
	if(mysql_real_query(mpcon,cmd,strlen(cmd)))
	{
		cerr<<"breaktrans query err"<<endl;
		return;
	}
	mp_res = mysql_store_result(mpcon);
	mp_row = mysql_fetch_row(mp_res);
	if(mp_row == 0)
	{
		char msg[] = "This file is never downloaded ,try download it.";
		int str_len = strlen(msg);
		char sendbuff[64] = {0};
		sprintf(sendbuff,"%d#",str_len);
		strcat(sendbuff,msg);
		send(_cli_fd,sendbuff,buff_size,0);
		return;
	}
	else
	{
		/*开始断点续传，那么原来的断点信息就应该删除*/
		char del_cmd[100] = "delete from breaktrans where flag = 'download' and name='";
		char name[32] = "";
		strcpy(name,val["name"].asString().c_str());
		strcat(del_cmd,name);
		strcat(del_cmd,"'and filename = '");
		strcat(del_cmd,filename);
		strcat(del_cmd,"';");
		if(mysql_real_query(mpcon,del_cmd,strlen(del_cmd)))
		{
			cerr<<"breaktrans query err"<<endl;
			return;
		}
		/*****************************************/
		/*开始进行下载的断点续传*/
		int fd = open(filename,O_RDONLY);
		if(fd == -1)
		{
			cerr<<"open file fail."<<endl;
			return;
		}
		long breakpoint = atol(mp_row[2]);
		char filename[1024] = {0};
		strcpy(filename,mp_row[1]);
		char fileMD5[128] = {0};
		create_MD5(filename,fileMD5,sizeof(fileMD5));
		/*根据数据库中的断点信息，对文件进行偏移，找到上次断开传输时的位置*/
		int pos = lseek(fd,breakpoint,SEEK_SET);
		if(pos == -1)
		{
			cerr<<"lseek fail."<<endl;
			return;
		}
		int num = 0;
		char buff[1024] = {0};
		char sendbuff[1024] = {0};
		while((num = read(fd,buff,1000)) >= 0)
		{
			if(num == 0)
			{
				close(fd);
				break;
			}
			sprintf(sendbuff,"%d#",num);
			strcat(sendbuff,buff);
			pos = send(_cli_fd,sendbuff,buff_size,0);
			breakpoint += num;
			if(pos == -1)
			{
				/*客户端挂了，重新记录断点信息*/
				char name[24] = {0};
				strcpy(name,val["name"].asString().c_str());
				breakpoint_trans("download",name,filename,breakpoint);
				cout<<_cli_fd<<" is over and already save info."<<endl;
				close(fd);
				return;
			}
			memset(sendbuff,0,sizeof(sendbuff));
			memset(buff,0,sizeof(buff));
		}
		cout<<"download file over!"<<endl;
		/*开始校验文件*/
		char msg[1024] = "";
		int res = recv(_cli_fd,msg,1023,0);
		if(res <=0)
		{
			cerr<<"client require err"<<endl;
			return;
		}
		char md5[128] = {0};
		char*nap = strchr(msg,'#');
		strcpy(md5,nap+1);
		if(strcmp(md5,fileMD5) == 0)
		{
			send(_cli_fd,"2#ok",buff_size,0);
		}
		else
		{
			send(_cli_fd,"2#no",buff_size,0);
		}
	}
}
