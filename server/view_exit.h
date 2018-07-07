#ifndef _VIEW_EXIT_H_
#define _VIEW_EXIT_H
#include<string>
#include"view.h"
using namespace std;
class view_exit:public view
{
	public:
		void process(Json::Value val,int cli_fd);
	private:
		string reason;
		int _cli_fd;
};

#endif
