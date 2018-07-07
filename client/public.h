#ifndef _PUBLIC_H
#define _PUBLIC_H
enum MSG
{
	MSG_TYPE_REGISTER,/*注册*/
	MSG_TYPE_LOGIN,/*登录*/
	MSG_TYPE_UPLOAD,/*上传文件*/
	MSG_TYPE_DOWNLOAD,/*下载文件*/
	MSG_TYPE_EXIT,/*退出*/
	MSG_TYPE_B_DOWNLOAD,/*断点续传-下载*/
	MSG_TYPE_B_UPLOAD,/*断点续传-上传*/
	MSG_TYPE_CHECKFILE /*查询用户可下载的文件*/
};
#define FILE_MAX_SIZE 10485760  /*10MB*/
/*10M file*/
#define PATH "./filedir/"

#define buff_size 1024 /*sendbuff & recvbuff size*/

#endif
