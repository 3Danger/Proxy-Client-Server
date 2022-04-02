//
// Created by csamuro on 31.03.2022.
//
#include <iostream>
#include "srcs/ServerProxy.h"
int main(){
	ServerProxy server("2221", "127.0.0.1");
	server.connectToClient();
	server.connectToServer("6667", "31.220.7.179");
	server.run();

	std::cout << "there" << std::endl;
}