#include"contral.h"
#include"view_register.h"
#include"view_login.h"
#include"view_upload.h"
#include"view_download.h"
#include"view_exit.h"
#include"public.h"
#include"view_breakpoint_upload.h"
#include"view_breakpoint_download.h"
#include"view_checkfile.h"
#include<errno.h>
using namespace std;

contral::contral()
{
	_map.insert(make_pair(MSG_TYPE_REGISTER,new view_register()));
	_map.insert(make_pair(MSG_TYPE_LOGIN,new view_login()));
	_map.insert(make_pair(MSG_TYPE_UPLOAD,new view_upload()));
	_map.insert(make_pair(MSG_TYPE_DOWNLOAD,new view_download()));
	_map.insert(make_pair(MSG_TYPE_EXIT,new view_exit()));
	_map.insert(make_pair(MSG_TYPE_B_DOWNLOAD,new view_breakpoint_download()));
	_map.insert(make_pair(MSG_TYPE_B_UPLOAD,new view_breakpoint_upload()));
	_map.insert(make_pair(MSG_TYPE_CHECKFILE,new view_checkfile()));
}
void contral::process(char*buff,int cli_fd)
{
	/*json解析*/
	Json::Value val;
	Json::Reader read;
	if(-1 == read.parse(buff,val))
	{
		cerr<<"read fail;errno: "<<errno<<endl;
		return ;
	}

	map<int,view*>::iterator it = _map.find(val["TYPE"].asInt());
	if(it != _map.end())
	{
		it->second->process(val,cli_fd);
	}
	else
	{
		cerr<<"find fail view;errno: "<<errno<<endl;
		return ;
	}
}
