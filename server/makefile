all: server
server: tcpserver.o mpthread.o contral.o \
create_MD5.o server.o view_breakpoint_download.o \
view_breakpoint_upload.o view_download.o \
view_exit.o view_login.o view_register.o \
view_upload.o breakpoint_transport.o user_file.o \
view_checkfile.o
	g++ -o ser *.o -levent -lpthread -ljson -lmysqlclient -g
tcpserver.o:tcpserver.cpp
	g++ -c tcpserver.cpp
mpthread.o:mpthread.cpp
	g++ -c mpthread.cpp
contral.o:contral.cpp
	g++ -c contral.cpp
breakpoint_transport.o:breakpoint_transport.cpp
	g++ -c breakpoint_transport.cpp
create_MD5.o:create_MD5.cpp
	g++ -c create_MD5.cpp
server.o:server.cpp
	g++ -c server.cpp
view_breakpoint_download.o:view_breakpoint_download.cpp
	g++ -c view_breakpoint_download.cpp
view_breakpoint_upload.o:view_breakpoint_upload.cpp
	g++ -c view_breakpoint_upload.cpp
view_download.o:view_download.cpp
	g++ -c view_download.cpp
view_upload.o:view_upload.cpp
	g++ -c view_upload.cpp
view_exit.o:view_exit.cpp
	g++ -c view_exit.cpp
view_login.o:view_login.cpp
	g++ -c view_login.cpp
view_register.o:view_register.cpp
	g++ -c view_register.cpp
user_file.o:user_file.cpp
	g++ -c user_file.cpp
view_checkfile.o:view_checkfile.cpp
	g++ -c view_checkfile.cpp
clean:
	rm -rf *.o
