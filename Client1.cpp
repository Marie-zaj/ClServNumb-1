#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <string>
#include <ws2tcpip.h>
using namespace std;

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

#define PAUSE 1

SOCKET ServerSocket = INVALID_SOCKET;

DWORD Sender(void* param)
{
	while (true)
	{
		int number;
		cout << "Введите целое число: ";
		cin >> number;

		string message = to_string(number);

		int iResult = send(ServerSocket, message.c_str(), message.size(), 0);

		if (iResult == SOCKET_ERROR)
		{
			cout << "Ошибка отправки: " << WSAGetLastError() << endl;
			closesocket(ServerSocket);
			WSACleanup();
			return 1;
		}
	}
}

DWORD Receiver(void* param)
{
	while (true)
	{
		char buffer[DEFAULT_BUFLEN];

		int iResult = recv(ServerSocket, buffer, DEFAULT_BUFLEN, 0);

		if (iResult > 0)
		{
			buffer[iResult] = '\0';
			cout << "Ответ сервера: " << buffer << endl;
		}
		else if (iResult == 0)
		{
			cout << "Соединение закрыто сервером.\n";
			break;
		}
		else
		{
			cout << "Ошибка recv: " << WSAGetLastError() << endl;
			break;
		}
	}

	return 0;
}

int main()
{
	setlocale(0, "");
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	system("title КЛІЄНТСЬКА СТОРОНА");

	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	// ініціалізація Winsock
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		cout << "не вдалося ініціалізувати Winsock: " << iResult << "\n";
		return 11;
	}
	else {
		Sleep(PAUSE);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	addrinfo hints{};
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// отримання адреси та порту сервера
	const char* ip = "localhost"; // за замовчуванням, обидва додатки, і клієнт, і сервер, працюють на одній машині

	addrinfo* result = NULL;
	iResult = getaddrinfo(ip, DEFAULT_PORT, &hints, &result);

	if (iResult != 0) {
		cout << "getaddrinfo не вдалося: " << iResult << "\n";
		WSACleanup();
		return 12;
	}
	else {
		Sleep(PAUSE);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	for (addrinfo* ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		// створення SOCKET для підключення до сервера
		ServerSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

		if (ServerSocket == INVALID_SOCKET) {
			cout << "не вдалося створити сокет: " << WSAGetLastError() << "\n";
			WSACleanup();
			return 13;
		}

		// підключення до сервера
		iResult = connect(ServerSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ServerSocket);
			ServerSocket = INVALID_SOCKET;
			continue;
		}
		Sleep(PAUSE);
		break;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	freeaddrinfo(result);

	if (ServerSocket == INVALID_SOCKET) {
		cout << "неможливо підключитися до сервера!\n";
		WSACleanup();
		return 14;
	}
	else {
		Sleep(PAUSE);
	}

	CreateThread(0, 0, Sender, 0, 0, 0);
	CreateThread(0, 0, Receiver, 0, 0, 0);

	Sleep(INFINITE);
}