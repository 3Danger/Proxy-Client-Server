//
// Created by csamuro on 31.03.2022.
//
#include <iostream>
#include "srcs/ServerProxy.h"

int main(int ac, const char ** av){
	std::string appName = av[0];
	appName = appName.substr(appName.find_last_of('/'));
	if (ac != 3){
		std::cout << "\nusage: ." << appName << " [port] [ipAddress]\n";
		exit(0);
	}

	const char * port = av[1];
	const char * address = av[2];

	std::cout << "for connect to proxy: port 2221, ipAddress 127.0.0.1\n";
	ServerProxy server("2221", "127.0.0.1");

	server.connectToClient();
	server.connectToServer(port, address);
	server.run();
}