#ifndef _VIEW_REGISTER_H
#define _VIEW_REGISTER_H
#include<string>
#include"view.h"
using namespace std;
class view_register:public view
{
	public:
		void process(Json::Value val,int cli_fd);
	private:
		string reason;
		int _cli_fd;
};

#endif
