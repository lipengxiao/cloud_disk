#ifndef _VIEW_H_
#define _VIEW_H_
#include<json/json.h>

class view
{
	public:
		virtual void process(Json::Value val,int cli_fd) = 0;
};

#endif
