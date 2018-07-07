#ifndef _BREAKPOINT_TRANS_H
#define _BREAKPOINT_TRANS_H
#include "public.h"
#include "view_download.h"
#include "view.h"
#include<json/json.h>
#include<errno.h>
#include<mysql/mysql.h>
#include<string.h>
#include<sys/socket.h>
#include<stdio.h>
#include"create_MD5.h"
#include<sys/stat.h>

using namespace std;


/*客户端异常终止时，服务器将文件断点信息记录到数据库中，用于客户端再次请求时进行断点续传*/
void breakpoint_trans(const char* flag,char*user,char*filename,long breakpoint);

#endif
