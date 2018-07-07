#include "public.h"
#include "view_download.h"
#include "view.h"
#include "breakpoint_transport.h"
#include<json/json.h>
#include<errno.h>
#include<mysql/mysql.h>
#include<string.h>
#include<sys/socket.h>
#include<stdio.h>
#include"create_MD5.h"
#include<sys/stat.h>
using namespace std;

void view_download::process(Json::Value val,int cli_fd)
{
	char name[16] = "";
	strcpy(name,val["name"].asString().c_str());
	/*********************************************/
	_cli_fd = cli_fd;
	MYSQL*mpcon = mysql_init((MYSQL*)0);
	MYSQL_RES *mp_res;
	MYSQL_ROW mp_row;

	if(!mysql_real_connect(mpcon,"127.0.0.1","root","123456",NULL,3306,NULL,0))
	{
		cerr<<"sql conncet fail;errno: "<<errno<<endl;
		return;
	}
	if(mysql_select_db(mpcon,"cloud_disk"))
	{
		cerr<<"select database fail;errno: "<<errno<<endl;
		return;
	}
	char filename[64] = {0};
	strcpy(filename,val["filename"].asString().c_str());
	/*visit file_info table to find this file is or not at ser.*/
	char cmd[128] = "select * from file_info where (filename='";
	strcat(cmd,filename);
	strcat(cmd,"' and name='");
	strcat(cmd,val["name"].asString().c_str());
	strcat(cmd,"');");
	if(mysql_real_query(mpcon,cmd,strlen(cmd)))/*query err*/
	{
		cerr<<"file_info fail;errno: "<<errno<<endl;
		return;
	}
	mp_res = mysql_store_result(mpcon);
	mp_row = mysql_fetch_row(mp_res);
	if(mp_row == 0)
	{
		char msg[] = "This file is not exist,please check your input!";
		int str_len = strlen(msg);
		char sendbuff[64] = {0};
		sprintf(sendbuff,"%d#",str_len);
		strcat(sendbuff,msg);
		send(_cli_fd,sendbuff,buff_size,0);
		return;
	}
	/*start send*/
	char filemd5[64] = {0};
	strcpy(filemd5,mp_row[0]);
	char file_name[buff_size] = PATH;
	strcat(file_name,filename);
	int fd = open(file_name,O_RDONLY);
	if(-1 == fd)
	{
		cerr<<"file is not exsit.please check filesystem!"<<endl;
		return;
	}
	struct stat st;
	
	lstat(file_name,&st);
	long file_size = st.st_size;
	cout<<st.st_size<<endl;
	if(file_size > FILE_MAX_SIZE)
	{
		/*large file send*/
	}
	char sendmsg[128] = {0};
	sprintf(sendmsg,"%d#",file_size);
	strcat(sendmsg,filemd5);
	cout<<sendmsg<<endl;
	send(_cli_fd,sendmsg,buff_size,0);/*send file_size and md5 to cli*/
	/*********************************************************************/
	char readbuff[1024] = {0};
	int num = 0;
	int rt = 0;
	long breakpoint = 0;
	while((num = read(fd,readbuff,1000)) >= 0)
	{
		if(num == 0)
		{
			char sendbuff[1024] = {0};
			sprintf(sendbuff,"%d#",num);
			cout<<num<<endl;
			strcat(sendbuff,readbuff);
			rt = send(_cli_fd,sendbuff,buff_size,0);	
			break;
		}
		char sendbuff[1024] = {0};
		sprintf(sendbuff,"%d#",num);
		cout<<num<<endl;
		strcat(sendbuff,readbuff);
		rt = send(_cli_fd,sendbuff,buff_size,0);	
		if(rt == -1)
		{
			/*客户端异常终止，启用断点续传保存断点信息*/
			breakpoint_trans("download",name,filename,breakpoint);
			cout<<"client "<<_cli_fd<<" is over,already save info."<<endl;
			close(fd);
			return;
		}
		breakpoint += rt;
		memset(readbuff,0,strlen(readbuff));
	}
	/***************************************************/
	cout<<"crc start"<<endl;
	char crc[buff_size] = "";
	int crc_res = recv(_cli_fd,crc,buff_size,0);
	if(crc_res <= 0)
	{
		cerr<<"client is over errno:"<<errno<<endl;
		close(fd);
		return;
	}
	if(strcmp(crc,filemd5) == 0)
	{
		char sendbuff[1024] = {0};
		const char *str = "download success";
		sprintf(sendbuff,"%d#",strlen(str));
		strcat(sendbuff,str);
		rt = send(_cli_fd,readbuff,buff_size,0);	
	}
	else
	{
		char sendbuff[1024] = {0};
		const char *str = "download fail";
		sprintf(sendbuff,"%d#",strlen(str));
		strcat(sendbuff,str);
		rt = send(_cli_fd,readbuff,buff_size,0);	
	}
	cout<<"file: "<<filename<<" send over!"<<endl;
	close(fd);
}
