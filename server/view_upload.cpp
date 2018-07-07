#include "breakpoint_transport.h"
#include "view_upload.h"
#include "public.h"
#include "view.h"
#include "create_MD5.h"
#include<string.h>
#include<json/json.h>
#include<errno.h>
#include<mysql/mysql.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<stdio.h>

using namespace std;
void view_upload::process(Json::Value val,int cli_fd)
{
	_cli_fd = cli_fd;
	/*
	 *文件上传：
	 秒传：根据客户端上传的MD5在file_info中查询是否存在，如果存在就无需再次上传，将该用户拥有该文件的
		   信息存放在file_info表中，然后回复客户端，上传成功。
	正常上传：服务端需要接收目标文件的MD5和文件大小，然后服务器开始接收数据，接收完成之后校验文件是否传输完毕
			  如果失败，要求重新上传，这里考虑一个效率问题，如果文件很大，几个G，已经上传了99%，然后失败了，那么需要重新上传？
			  我的想法是，文件如果很大，采取分文件传输，客户端向服务器发送给一部分文件的MD5和文件大小，如果过程中出现失败，只
			  需要重新上传该部分，在上传完毕之后，将文件合一，进行校验。而对于小文件来说就没有分文件的必要了，一切为了效率。
	*/
	MYSQL*mpcon = mysql_init((MYSQL*)0);
	MYSQL_RES*mp_res;
	MYSQL_ROW mp_row;
	if(!mysql_real_connect(mpcon,"127.0.0.1","root","123456",NULL,3306,NULL,0))
	{
		cerr<<"sql connect fail;errno "<<errno<<endl;
		return;
	}
	if(mysql_select_db(mpcon,"cloud_disk"))
	{
		cerr<<"select database fail;errno: "<<errno<<endl;
		return;
	}
	
	char filemd5[64] = {0};
	char name[32] = {0};
	char filename[32] = {0};
	strcpy(filemd5,val["md5"].asString().c_str());
	strcpy(name,val["name"].asString().c_str());
	strcpy(filename,val["filename"].asString().c_str());
	char cmd[128] = "select * from file_info where(md5='";
	strcat(cmd,filemd5);
	strcat(cmd,"');");
	if(mysql_real_query(mpcon,cmd,strlen(cmd)))
	{
		cerr<<"query err;errno: "<<errno<<endl;
		return;
	}
	mp_res = mysql_store_result(mpcon);
	mp_row = mysql_fetch_row(mp_res);
	if(mp_row != 0)
	{
		/*秒传*/
		char msg[] = "upload success!";
		int str_len = strlen(msg);
		char sendbuff[64] = {0};
		sprintf(sendbuff,"%d#",strlen);
		strcat(sendbuff,msg);
		send(_cli_fd,sendbuff,buff_size,0);
	}
	else
	{
		char msg[] = "upload start!";
		int str_len = strlen(msg);
		char sendbuff[64] = {0};
		sprintf(sendbuff,"%d#",strlen);
		strcat(sendbuff,msg);
		send(_cli_fd,sendbuff,buff_size,0);
		
		cout<<"start upload"<<endl;
		/*正常上传*/
		long file_size = val["filesize"].asInt();
		char recvbuff[buff_size] = {0};
		int num = 0;
		/*open file*/
		char file[buff_size] = PATH;
		strcat(file,filename);
		int fw = open(file,O_WRONLY | O_CREAT,0600);
		if(fw == -1)
		{
			cerr<<"file create or open fail."<<endl;
			return;
		}
		/**********************/
		cout<<"start write file"<<endl;
		int count = 0;
		while(1)
		{
			num = recv(_cli_fd,recvbuff,buff_size,0);
			if(num < 0)
			{
				cerr<<"client connection err;errno:"<<errno<<endl;
				close(fw);
				return;
			}
			if(num == 0)
			{
				close(fw);
				break;
			}
			int size;
			sscanf(recvbuff,"%d",&size);
			if(size <= 0)
			{
				close(fw);
				break;
			}
			char *nap = strchr(recvbuff,'#');
			write(fw,nap+1,size);
			count += size;
			memset(recvbuff,0,buff_size);
		}
		cout<<"end write"<<endl;
		if(count < file_size)
		{
			/*传输中断，启动断点信息保存*/
			cout<<"client close pipe,save breakpoint."<<endl;
			breakpoint_trans("upload",name,filename,count);
			return;
		}
		if(count >= file_size)
		{
			/*校验文件*/
			char md5_buff[32] = {0};
			create_MD5(file,md5_buff,32);
			cout<<"filename's md5:"<<md5_buff<<endl;
			cout<<"filemd5:"<<filemd5<<endl;
			if(strcmp(md5_buff,filemd5) == 0)
			{
				char msg[] = "upload over!";
				int str_len = strlen(msg);
				char sendbuff[64] = {0};
				sprintf(sendbuff,"%d#",str_len);
				strcat(sendbuff,msg);
				send(_cli_fd,sendbuff,buff_size,0);
			}
			else
			{
				char msg[] = "upload fail! try again!";
				int str_len = strlen(msg);
				char sendbuff[64] = {0};
				sprintf(sendbuff,"%d#",str_len);
				strcat(sendbuff,msg);
				send(_cli_fd,sendbuff,buff_size,0);
				return;
			}
		}
	}
	char cmd_insert[128] = "insert into file_info value('";
	strcat(cmd_insert,filemd5);
	strcat(cmd_insert,"','");
	strcat(cmd_insert,name);
	strcat(cmd_insert,"','");
	strcat(cmd_insert,filename);
	strcat(cmd_insert,"');");
	if(mysql_real_query(mpcon,cmd_insert,strlen(cmd_insert)))
	{
		cerr<<"query fail;errno"<<errno<<endl;
		return;
	}
}
