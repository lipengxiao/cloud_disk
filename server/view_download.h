#ifndef _VIEW_DOWNLOAD_H
#define _VIEW_DOWNLOAD_H
#include"view.h"
using namespace std;
class view_download:public view
{
	public:
		void process(Json::Value val,int cli_fd);
		
	private:
		int _cli_fd;
};
/*在数据表中查询该用户可以下载的文件信息*/
void user_file(char*name,char*buff,int size,char*flag);

#endif
