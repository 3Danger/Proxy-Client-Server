//
// Created by csamuro on 31.03.2022.
//
#include <iostream>
#include "srcs/ServerProxy.h"
int main(){
	ServerProxy server("2220", "127.0.0.1");
	server.connectToClient();

	std::cout << "there" << std::endl;
}