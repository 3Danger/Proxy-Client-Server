//
// Created by csamuro on 01.04.2022.
//

#include "ServerProxy.h"

ServerProxy::ServerProxy(char * port, char * ipAddres) throw() {
	memset(&hints, 0, sizeof hints);

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int res = getaddrinfo(ipAddres, port, &hints, &servInfo);
	makeSocket();
	makeBind();
}

ServerProxy::~ServerProxy() {
	close(serverSocketFD);
	std::cout << "serverSocketFD was closed from destructor!" << std::endl;
	freeaddrinfo(servInfo);
	std::cout << "servInfo was freed from destructor!" << std::endl;
}

int ServerProxy::makeSocket() throw(){
	serverSocketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocketFD < 0){
		throw std::runtime_error("Socket error");
	}
	std::cout << "socket created" << std::endl;
}

int ServerProxy::makeBind() throw() {
	int res = bind(serverSocketFD, servInfo->ai_addr, servInfo->ai_addrlen);

	std::cout << "res = " << res << std::endl;
	std::cout << " = " << strerror(errno) << std::endl;

	if (res < 0)
		throw std::runtime_error(std::string("Bind error: ") + strerror(errno));
	std::cout << "socket was binded" << std::endl;
}

