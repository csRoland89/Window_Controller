main:
	clear
	g++ main.cpp -Wall -lsqlite3  -omain.out -lgpiod

debug:
	clear
	g++ main.cpp -Wall -DDEBUG -lsqlite3  -omain.out -lgpiod
