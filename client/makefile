all: client
client:   view_cli.o cli_main.o client.o create_MD5.o ls_file.o
	g++ -o client  view_cli.o cli_main.o client.o create_MD5.o ls_file.o  -ljson
client.o:client.cpp
	g++ -c client.cpp
view_cli.o:view_cli.cpp
	g++ -c view_cli.cpp
cli_main.o:cli_main.cpp
	g++ -c cli_main.cpp
create_MD5.o:create_MD5.cpp
	g++ -c create_MD5.cpp
ls_file.o:ls_file.cpp
	g++ -c ls_file.cpp
clean:
	rm -rf *.o
