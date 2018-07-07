#ifndef _CLIENT_H
#define _CLIENT_H
#include<iostream>
#include<json/json.h>
#include<string>
#include<pthread.h>
using namespace std;

class Client
{
	public:
		Client(char*ip,unsigned short port);
		~Client(){}
		void start_login_register(int flag);
		void run();
		void register_func();
		void load_func();
		void exit_func();
		
		void uploadfile();
		void uploadfile_continue();/*继续上传*/
		void uploadfile_new();/*新上传*/

		void downloadfile();
		void downloadfile_continue();/*继续下载*/
		void downloadfile_new();/*新下载*/
		
		friend void fun(int sig);
	private:
		friend void* p_fun(void*arg);
		int _sockfd;
		pthread_t id;
		string regist_str;
		string login_str;
		bool login_flag;
		string _name;
};


#endif
