#ifndef _VIEW_CHECKFILE_H
#define _VIEW_CHECKFILE_H
#include<string>
#include"view.h"
using namespace std;
class view_checkfile:public view
{
	public:
		void process(Json::Value val,int cli_fd);
	private:
		int _cli_fd;
};

#endif
