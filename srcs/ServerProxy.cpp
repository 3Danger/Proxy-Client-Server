//
// Created by csamuro on 01.04.2022.
//

#include "ServerProxy.h"
/*
 * port и ipAddres для нашего прокси сервера
 */
ServerProxy::ServerProxy(char const * port, char const * ipAddres)  {
	// Зануляем то что пока не используется.
	clientFD = -1;
	serverFD = -1;
	/*
	 * Записываем информацию,
	 * номер порта,
	 * IP адрес,
	 * и тип подключения
	 * в структуру proxyInfo
	 */
	addrinfo hints = makeAddrinfoHints();
	if (getaddrinfo(ipAddres, port, &hints, &proxyInfo))
		throw std::runtime_error(strerror(errno));
	// Получаем сокет/FD подробнее см. в комментариях самой функции makeSocket();
	maxFd = proxySocketFD = makeSocket();
	// Связываем наш пустой сокет с конкретным адресом который -
	makeBind(proxySocketFD, proxyInfo);
	// - Будем слушать в listen
	int resListen = listen(proxySocketFD, 5);
	if (resListen < 0)
		throw std::runtime_error("Listen error");

	/*
	 * Для начала искользования fd_set fds
	 * необходимо его занулить с помощью FD_ZERO
	 * и лишь затем добавить в него FD
	 * с помошью FD_SET()
	 * который далее будет использоваться в методе select
	 */
	FD_ZERO(&fds);
	FD_SET(proxySocketFD, &fds);
}

ServerProxy::~ServerProxy() {
	/*
	 * Зачищаем память и
	 * закрываем открытые сокеты,
	 * иначе будет утечка.
	 */
	close(proxySocketFD);
	std::cout << "proxySocketFD was closed from destructor!" << std::endl;
	freeaddrinfo(proxyInfo);
	std::cout << "proxyInfo was freed from destructor!" << std::endl;
	freeaddrinfo(serverInfo);
	std::cout << "serverInfo was freed from destructor!" << std::endl;
	close(clientFD);
	std::cout << "clientFD was closed from destructor!" << std::endl;
	close(serverFD);
	std::cout << "serverFD was closed from destructor!" << std::endl;
}

int ServerProxy::makeSocket() {
	/*
	 * Создаем сокет/FD
	 * который будет работать с ipv4
	 * и иметь тип подключения TCP
	 */
	int res = socket(AF_INET, SOCK_STREAM, 0);
	if (res < 0)
		throw std::runtime_error("Socket error");
	// Задаем флаг для нашего сокета как неблокирующий
	fcntl(res, F_SETFD, O_NONBLOCK);
	std::cout << "socket created" << std::endl;
	return res;
}

void ServerProxy::makeBind(int fd, addrinfo * addr)  {
	/*
	 * Одноразовая функция которая связывает
	 * прослушиываемый порт с нашим FD
	 */
	int res = bind(fd, addr->ai_addr, addr->ai_addrlen);
	if (res < 0)
		throw std::runtime_error(std::string("Bind error: ") + strerror(errno));
	std::cout << "socket was binded" << std::endl;
}

int ServerProxy::connectToClient() {
	 // Принимаем подключение
	clientFD = accept(proxySocketFD, proxyInfo->ai_addr, &proxyInfo->ai_addrlen);
	if (clientFD < 0)
		std::runtime_error(std::string("connection to client err: ") + strerror(errno));
	// Добавляем его в набор дескрипторов
	FD_SET(clientFD, &fds);
	// maxFd нужен для метода select
	maxFd = std::max(maxFd, clientFD);
	return	clientFD;
}

int ServerProxy::connectToServer(const char *port, const char *ipAddres)  {
	// Задаем параметры сервера к которобу будем подключаться
	addrinfo hints = makeAddrinfoHints();
	if (getaddrinfo(ipAddres, port, &hints, &serverInfo))
		throw std::runtime_error(strerror(errno));
	// Создаем пустой сокет
	serverFD = makeSocket();
	if (serverFD < 0)
		throw std::runtime_error(strerror(errno));
	// Подключаемся к серверу и ассоциируем этот совет к подключаемому серверу
	if (connect(serverFD, (sockaddr *)serverInfo->ai_addr, serverInfo->ai_addrlen)) {
		throw std::runtime_error(std::string("cannot connect to server: ") + strerror(errno));
	}
	// Задаем флаг сокету как неблокирующий
	fcntl(serverFD, F_SETFD, O_NONBLOCK);
	// Добавляем в набор сокетов в fds, это нужно для метода select
	FD_SET(serverFD, &fds);
	// maxFd нужен для метода select
	maxFd = std::max(maxFd, serverFD);
	return serverFD;
}

[[noreturn]] void ServerProxy::run() {
	int numOfSelect;
	uint8_t whatLast = 0;
	fd_set copyFds;
	timeval tm = {0, 50000}; // Тайм-оут для select

	// Зачищаем буффер перед использованием, хотя это здесь не нужно
	bzero(buff, SIZE_BUFF);
	std::ofstream outFileLog;

	while(true){
		/*
		 * Делаем копию fds,
		 * потому что select удаляет из набора те сокеты,
		 * которых не появились данные.
		 */
		copyFds = fds;
		// тут мы удаляем из набора copFds те сокеты в которых ничего не произошло, те не появились данные
		// select так же возвращает число означающее количество сокетов в которых есть что прочитать
		numOfSelect = select(maxFd + 1, &copyFds, NULL, NULL, &tm);
		if (numOfSelect < 0)
			throw std::runtime_error(std::string("select error: ") + strerror(errno));
		if (numOfSelect > 0){
			/*
			 * открываем наш лог файл для записи,
			 * так же переносим 'курсор' в конец файлы, чтоб не перезаписывала поверх.
			 * если файла нет, то он будет создан
			 */
			outFileLog.open(logFileName, std::ios::app);
			/*
			 * FD_ISSET проверяет есть ли этот сокет/FD в наборе copyFds -
			 * который имеет только те FDs - в которых есть данные
			 */
			if (FD_ISSET(serverFD, &copyFds)) {
				/*
				 * whatLast != FD
				 * это условие необходимо для того что бы 2
				 * раза подряд не писать в лог файле откуда пришло сообщение
				 */
				if (whatLast != serverFD)
					outFileLog << "\n\nSERVER REPLY:\n";
				whatLast = serverFD;
				// Отправляем сообщение от сервера к клиенту если там был мессендж
				Send(serverFD, clientFD, &copyFds, outFileLog);
			}
			if (FD_ISSET(clientFD, &copyFds)) {
				if (whatLast != clientFD) {
					outFileLog << "\n\nCLIENT REPLY:\n";
				}
				whatLast = clientFD;
				// Отправляем сообщение от клиента к серверу если там был мессендж
				Send(clientFD, serverFD, &copyFds, outFileLog);
			}
			outFileLog.close();
		}
	}
}

/*
 * Читаем сообщения из \from и
 * отправляем сообщение в \to
 */
void ServerProxy::Send(int from, int to, fd_set * fds_set, std::ofstream & outFileLog){
	/*
	 * Удаляем данный FD из набора непрочитанных сокетов,
	 * тут это пока не нужно,
	 * но в случае работы со множеством сокетов
	 * это будет необходимо.
	 */
	FD_CLR(from, fds_set);
	// Читаем сообщение с \from в наш буффер. так же получаем колличество прочитаннх байт
	int readedBytes = recv(from, buff, SIZE_BUFF, 0);
	// Отправляем прочитанные сообщение к \to
	send(to, buff, readedBytes, 0);
	// Удаляем служебый символ который не нужен для лог файла
	// это наверное костыль, при необходимости придумаю чтонибудь
	for(int i = 0; i < readedBytes; ++i) {
		if (buff[i] == '\u0002') {
			buff[i] = ' ';
		}
	}
	// Логгируем месендж
	outFileLog.write(buff, readedBytes);
	// Зачищаем буфер
	bzero(buff, readedBytes);
}

// Создаем структуру с базовой информацией для использования по дефолту
addrinfo ServerProxy::makeAddrinfoHints() {
	addrinfo hints = {0};

	/*
	* AF_INET говорит нам что нужно использовать тип ipv4
	* SOCK_STREAM этим указываем TCP
	*/
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	return hints;
}


