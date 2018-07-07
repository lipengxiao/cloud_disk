#include "breakpoint_transport.h"
#include<iostream>

using namespace std;

void breakpoint_trans(const char*flag,char*user,char*filename,long breakpoint)
{
	MYSQL*mpcon = mysql_init((MYSQL*)0);
	MYSQL_RES *mp_res;
	MYSQL_ROW mp_row;
	if(!mysql_real_connect(mpcon,"127.0.0.1","root","123456",NULL,3306,NULL,0))
	{
		cerr<<"sql connect fail;errno: "<<errno<<endl;
		return;
	}
	/**********************************************/
	if(mysql_select_db(mpcon,"cloud_disk"))
	{
		cerr<<"select database err;errno:"<<errno<<endl;
		return;
	}
	/**************************************************/
	char cmd[128] = "insert into breaktrans values('";
	if(strcmp(flag,"upload") == 0)
	{
		strcat(cmd,"upload','");
	}
	if(strcmp(flag,"download") == 0)
	{
		strcat(cmd,"download','");
	}
	strcat(cmd,user);
	strcat(cmd,"','");
	strcat(cmd,filename);
	strcat(cmd,"','");
	char bp[16] = {0};
	sprintf(bp,"%ld",breakpoint);
	strcat(cmd,bp);
	strcat(cmd,"');");
	/************************************************/
	cout<<"breaktrans cmd:"<<cmd<<endl;
	if(mysql_real_query(mpcon,cmd,strlen(cmd)))
	{
		cerr<<"query fail;errno: "<<errno<<endl;
		return;
	}
	cout<<"breakpoint info save ok"<<endl;
}

