#ifndef _VIEW_LOGIN_H
#define _VIEW_LOGIN_H
#include<string>
#include"view.h"
using namespace std;
class view_login:public view
{
	public:
		void process(Json::Value val,int cli_fd);
	private:
		string reason;
		int _cli_fd;
};

#endif
