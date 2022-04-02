//
// Created by csamuro on 01.04.2022.
//

#include "ServerProxy.h"

ServerProxy::ServerProxy(char const * port, char const * ipAddres)  {
	clientFD = -1;
	serverFD = -1;
	addrinfo hints = makeAddrinfoHints();
	if (getaddrinfo(ipAddres, port, &hints, &proxyInfo))
		throw std::runtime_error(strerror(errno));
	maxFd = proxySocketFD = makeSocket();
	makeBind(proxySocketFD, proxyInfo);
	int resListen = listen(proxySocketFD, 1);
	if (resListen < 0)
		throw std::runtime_error("Listen error");
	FD_ZERO(&fds);
	FD_SET(proxySocketFD, &fds);
	loop = true;
}

ServerProxy::~ServerProxy() {
	close(proxySocketFD);
	std::cout << "proxySocketFD was closed from destructor!" << std::endl;
	freeaddrinfo(proxyInfo);
	std::cout << "proxyInfo was freed from destructor!" << std::endl;
	close(clientFD);
	std::cout << "clientFD was closed from destructor!" << std::endl;
	close(serverFD);
	std::cout << "serverFD was closed from destructor!" << std::endl;
}

int ServerProxy::makeSocket() {

	int res = socket(AF_INET, SOCK_STREAM, 0);
	if (res < 0)
		throw std::runtime_error("Socket error");
	fcntl(res, F_SETFD, fcntl(res, F_GETFD) | O_NONBLOCK);
	std::cout << "socket created" << std::endl;
	return res;
}

void ServerProxy::makeBind(int fd, addrinfo * addr)  {
	int res = bind(fd, addr->ai_addr, addr->ai_addrlen);
	if (res < 0)
		throw std::runtime_error(std::string("Bind error: ") + strerror(errno));
	std::cout << "socket was binded" << std::endl;
}

int ServerProxy::connectToClient() {
	clientFD = accept(proxySocketFD, proxyInfo->ai_addr, &proxyInfo->ai_addrlen);
	if (clientFD < 0 && errno != EAGAIN)
		std::runtime_error(std::string("connection to client err: ") + strerror(errno));
	FD_SET(clientFD, &fds);
	maxFd = std::max(maxFd, clientFD);
	return	clientFD;
}

int ServerProxy::connectToServer(const char *port, const char *ipAddres)  {
	addrinfo hints = makeAddrinfoHints();
	if (getaddrinfo(ipAddres, port, &hints, &serverInfo))
		throw std::runtime_error(strerror(errno));
	serverFD = makeSocket();
	if (connect(serverFD, (sockaddr *)serverInfo->ai_addr, serverInfo->ai_addrlen))
		throw std::runtime_error(std::string("cannot connect to server: ") + strerror(errno));
	if (serverFD < 0 && errno != EAGAIN)
		throw std::runtime_error(strerror(errno));
	fcntl(serverFD, F_SETFD, O_NONBLOCK);
	FD_SET(serverFD, &fds);
	maxFd = std::max(maxFd, serverFD);
	return serverFD;
}

[[noreturn]] void ServerProxy::run() {
	int numOfSelect;
	uint8_t whatLast = 0;
	fd_set copyFds;
	timeval tm = {0, 50000};

	bzero(buff, SIZE_BUFF);
	fcntl(clientFD, F_SETFD, O_NONBLOCK);
	fcntl(serverFD, F_SETFD, O_NONBLOCK);
	std::ofstream outFileLog;

	while(true){
		copyFds = fds;
		numOfSelect = select(maxFd + 1, &copyFds, NULL, NULL, &tm);
		if (numOfSelect < 0)
			throw std::runtime_error(std::string("select error: ") + strerror(errno));
		if (numOfSelect > 0){
			outFileLog.open(logFileName, std::ios::app);
			if (FD_ISSET(serverFD, &copyFds)) {
				if (whatLast != 1)
					outFileLog << "\n\nSERVER REPLY:\n";
				whatLast = 1;
				Send(serverFD, clientFD, &copyFds, outFileLog);
			}
			if (FD_ISSET(clientFD, &copyFds)) {
				if (whatLast != 2)
					outFileLog << "\n\nCLIENT REPLY:\n";
				whatLast = 2;
				Send(clientFD, serverFD, &copyFds, outFileLog);
			}
			outFileLog.close();
		}
	}
}

void ServerProxy::Send(int from, int to, fd_set * fds_set, std::ofstream & outFileLog){
	FD_CLR(from, fds_set);
	int readedBytes = recv(from, buff, SIZE_BUFF, 0);
	send(to, buff, readedBytes, 0);
	for(int i = 0; i < readedBytes; ++i)
		if (buff[i] == '\u0002')
			readedBytes = i;
	outFileLog.write(buff, readedBytes);
	bzero(buff, readedBytes);
}

addrinfo ServerProxy::makeAddrinfoHints() {
	addrinfo hints = {0};

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	return hints;
}


