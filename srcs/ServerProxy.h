//
// Created by csamuro on 01.04.2022.
//

#ifndef PROXY_CLIENT_SERVER_SEREVERPROXY_H
#define PROXY_CLIENT_SERVER_SEREVERPROXY_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <exception>
#include <string>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <netdb.h>

class ServerProxy {
	int serverSocketFD;
	addrinfo hints;
	addrinfo * servInfo;
public:
	ServerProxy(char * port, char * ipAddres) throw();
	~ServerProxy();



private:
	int makeSocket() throw();
	int makeBind() throw();
};


#endif //PROXY_CLIENT_SERVER_SEREVERPROXY_H
