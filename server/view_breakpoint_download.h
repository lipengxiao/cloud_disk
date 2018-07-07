#ifndef _VIEW_BREAKPOINT_TRANS_H
#define _VIEW_BREAKPOINT_TRANS_H
#include"view.h"
using namespace std;
class view_breakpoint_download:public view
{
	public:
		void process(Json::Value val,int cli_fd);
		
	private:
		int _cli_fd;
};

#endif
