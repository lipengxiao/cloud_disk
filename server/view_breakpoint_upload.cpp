#include"breakpoint_transport.h"
#include"view_breakpoint_upload.h"
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

void view_breakpoint_upload::process(Json::Value val,int cli_fd)
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
	
	char filename[64] = {0};
	strcpy(filename,val["filename"].asString().c_str());
	char cmd[128] = "select * from breaktrans where(flag = 'upload' and filename='";
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
		char msg[] = "This file is never uploaded ,try upload it.";
		int str_len = strlen(msg);
		char sendbuff[64] = {0};
		sprintf(sendbuff,"%d#",str_len);
		strcat(sendbuff,msg);
		send(_cli_fd,sendbuff,buff_size,0);
		return;
	}
	else
	{
		/*开始进行上传的断点续传*/
		int fd = open(filename,O_WRONLY);
		if(fd == -1)
		{
			cerr<<"open file fail."<<endl;
			return;
		}
		long breakpoint = atol(mp_row[2]);
		char filename[32] = {0};
		strcpy(filename,mp_row[1]);
		int pos = lseek(fd,breakpoint,SEEK_SET);
		if(pos == -1)
		{
			cerr<<"lseek fail."<<endl;
			return;
		}
		int num = 0;
		char buff[1024] = {0};
		
		while((num = recv(_cli_fd,buff,1023,0)) >= 0)
		{
			if(num == 0)
			{
				/*文件传输完毕，开始校检文件*/
				char fileMD5[128] = {0};
				create_MD5(filename,fileMD5,sizeof(fileMD5));/*获取文件的MD5，用于文件校验*/
				char sendbuff[1024];
				sprintf(sendbuff,"%d#",strlen(fileMD5));
				strcat(sendbuff,fileMD5);
				send(_cli_fd,sendbuff,buff_size,0);
				memset(sendbuff,0,sizeof(sendbuff));
				int res = recv(_cli_fd,sendbuff,1023,0);
				if(res <= 0)
				{
					cerr<<"client err"<<endl;
					return;
				}
				int size;
				sscanf(sendbuff,"#d",&size);
				char *nap = strchr(sendbuff,'#');
				cout<<nap+1<<endl;
				break;
			}
			/*从数据包中将文件内容提取出来写入文件*/
			int size ;
			sscanf(buff,"%d",&size);
			char msg[1024] = "";
			char*nap = strchr(buff,'#');
			strcpy(msg,nap+1);
			write(fd,msg,strlen(msg));
			breakpoint += strlen(msg);
		}
		if(num == -1)
		{
			/*客户端挂了，重新记录断点信息*/
			char name[24] = {0};
			strcpy(name,val["name"].asString().c_str());
			breakpoint_trans("upload",name,filename,breakpoint);
			cout<<_cli_fd<<" is over and already save info."<<endl;
			close(fd);
			return;
		}
		close(fd);
	}
}
