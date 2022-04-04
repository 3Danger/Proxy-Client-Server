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
#include <fcntl.h>
#include <vector>
#include <iostream>
#include <fstream>


#define SIZE_BUFF 128

class ServerProxy {
	int proxySocketFD;	// Сокет этого прокси-сервера
	int clientFD;		// Сокет клиента
	int serverFD;		// Сокет сервера
	addrinfo * proxyInfo;	// Информация о нашем прокси
	addrinfo * serverInfo;	// Информация о удаленном сервере
	char buff[SIZE_BUFF];	// Буффер для обмена сообщений
	int maxFd;		// Маскимальное id of FD, необходим для select
	fd_set fds;		// Тут будем хранить все наборы FD, в нашем случае их будет всего 3
	std::string logFileName = "log.txt"; // Имя лог файла;
public:
	ServerProxy(char const * port, char const * ipAddres);
	~ServerProxy();

	[[noreturn]] void run();
	// Принимаем подключение клиента
	int connectToClient();
	// Подключаемся к серверу по его адресу
	int connectToServer(char const * port, char const * ipAddres);
private:
	// Создаем сокет типа TCP ipv4
	static int makeSocket();
	// Связвываем сокет с конкрентым адресом
	static void makeBind(int fd, addrinfo * addr);
	// Создаем структуру с базовой информацией для использования по дефолту
	addrinfo makeAddrinfoHints();
	// Читаем сообщение из FD \from, и отправляем это сообщение по FD \to,
	// при этом логгируя это в файле outFileLog
	void Send(int from, int to, fd_set * fds_set, std::ofstream & outFileLog);
};


#endif //PROXY_CLIENT_SERVER_SEREVERPROXY_H
