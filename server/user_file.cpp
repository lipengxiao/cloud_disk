#include"view_download.h"
#include<errno.h>
#include<mysql/mysql.h>
#include<string.h>
#include<iostream>
using namespace std;

void user_file(char*name,char*buff,int size,char*flag)
{
	if(buff == NULL || size ==0 || name == NULL)
	{
		return;
	}
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

	char cmd[128] = "";
	if(strcmp(flag,"download") == 0)
	{
		strcpy(cmd,"select filename from file_info where name='");
	}
	if(strcmp(flag,"breakdownload") == 0)
	{
		strcpy(cmd,"select filename from breaktrans where name='");
	}
	strcat(cmd,name);
	strcat(cmd,"';");
	if(mysql_real_query(mpcon,cmd,strlen(cmd)))/*query err*/
	{
		cerr<<"file_info fail;errno: "<<errno<<endl;
		return;
	}
	mp_res = mysql_store_result(mpcon);
	unsigned int num = mysql_num_fields(mp_res);
	while((mp_row = mysql_fetch_row(mp_res)))
	{
		unsigned long *len;
		len = mysql_fetch_lengths(mp_res);
		for(int i=0;i<num;++i)
		{
			strcat(buff,mp_row[i]);
			strcat(buff," ");
		}
	}
	cout<<buff<<endl;
}
