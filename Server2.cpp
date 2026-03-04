#define WIN32_LEAN_AND_MEAN // для прискорення процесу компіляції: https://stackoverflow.com/questions/11040133/what-does-defining-win32-lean-and-mean-exclude-exactly

#include <iostream>

#include <string>
#include <ws2tcpip.h>
using namespace std;

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

#define PAUSE 1

SOCKET ClientSocket = INVALID_SOCKET;

DWORD Receiver(void* param)
{
	while (true)
	{
		char buffer[DEFAULT_BUFLEN];

		int iResult = recv(ClientSocket, buffer, DEFAULT_BUFLEN, 0);

		if (iResult > 0)
		{
			buffer[iResult] = '\0';

			int number = atoi(buffer);   // преобразуем строку в число
			int result = number + 1;     // увеличиваем на 1

			cout << "Получено число: " << number << endl;
			cout << "Отправляем число: " << result << endl;

			string answer = to_string(result);

			send(ClientSocket, answer.c_str(), answer.size(), 0);
		}
		else if (iResult == 0)
		{
			cout << "Клиент отключился.\n";
			break;
		}
		else
		{
			cout << "Ошибка recv: " << WSAGetLastError() << endl;
			break;
		}
	}

	closesocket(ClientSocket);
	WSACleanup();
	return 0;
}

int main()
{
	setlocale(0, "");
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	system("title СЕРВЕРНА СТОРОНА");

	///////////////////////////////////////////////////////////////////////////////////////////////////////

	// ініціалізація Winsock
	WSADATA wsaData; // структура WSADATA містить інформацію про реалізацію Windows Sockets: https://docs.microsoft.com/en-us/windows/win32/api/winsock/ns-winsock-wsadata
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData); // функція WSAStartup ініціалізує використання DLL Winsock процесом: https://firststeps.ru/mfc/net/socket/r.php?2
	if (iResult != 0) {
		cout << "WSAStartup не вдалося з помилкою: " << iResult << "\n";
		cout << "підключення Winsock.dll пройшло з помилкою!\n";
		return 1;
	}
	else {
		Sleep(PAUSE);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	struct addrinfo hints {}; // структура addrinfo використовується функцією getaddrinfo для зберігання інформації про адресу хоста: https://docs.microsoft.com/en-us/windows/win32/api/ws2def/ns-ws2def-addrinfoa
	hints.ai_family = AF_INET; // сімейство адрес інтернет-протоколу версії 4 (IPv4)
	hints.ai_socktype = SOCK_STREAM; // забезпечує послідовні, надійні, двосторонні потоки з'єднань з механізмом передачі даних
	hints.ai_protocol = IPPROTO_TCP; // протокол TCP (Transmission Control Protocol). Це можливе значення, коли ai_family дорівнює AF_INET або AF_INET6, а ai_socktype — SOCK_STREAM
	hints.ai_flags = AI_PASSIVE; // адреса сокета буде використовуватися у виклику функції "bind"

	// отримання адреси та порту сервера
	struct addrinfo* result = NULL;
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		cout << "getaddrinfo не вдалося з помилкою: " << iResult << "\n";
		cout << "отримання адреси та порту сервера пройшло з помилкою!\n";
		WSACleanup(); // функція WSACleanup завершує використання DLL Winsock 2 (Ws2_32.dll): https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-wsacleanup
		return 2;
	}
	else {
		Sleep(PAUSE);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	// створення SOCKET для підключення до сервера
	SOCKET ListenSocket = INVALID_SOCKET;
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		cout << "сокет не вдалося створити з помилкою: " << WSAGetLastError() << "\n";
		cout << "створення сокета пройшло з помилкою!\n";
		freeaddrinfo(result);
		WSACleanup();
		return 3;
	}
	else {
		Sleep(PAUSE);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	// налаштування сокета для прослуховування
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen); // функція bind пов'язує локальну адресу з сокетом: https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-bind
	if (iResult == SOCKET_ERROR) {
		cout << "bind не вдалося з помилкою: " << WSAGetLastError() << "\n";
		cout << "прив'язка сокета не вдалася!\n";
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 4;
	}
	else {
		Sleep(PAUSE);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	// починаємо прослуховування на сокеті
	iResult = listen(ListenSocket, SOMAXCONN); // функція listen переводить сокет у стан очікування вхідних з'єднань: https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-listen
	if (iResult == SOCKET_ERROR) {
		cout << "listen не вдалося з помилкою: " << WSAGetLastError() << "\n";
		closesocket(ListenSocket);
		WSACleanup();
		return 5;
	}
	else {
		Sleep(PAUSE);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	// приймаємо клієнтський сокет
	ClientSocket = accept(ListenSocket, NULL, NULL); // функція accept приймає вхідне з'єднання: https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-accept
	if (ClientSocket == INVALID_SOCKET) {
		cout << "accept не вдалося з помилкою: " << WSAGetLastError() << "\n";
		closesocket(ListenSocket);
		WSACleanup();
		return 6;
	}
	else {
		Sleep(PAUSE);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	// закриття прослуховуючого сокета, оскільки він більше не потрібен
	closesocket(ListenSocket);

	// створення потоків надсилання та отримання повідомлень
	CreateThread(0, 0, Receiver, 0, 0, 0);

	Sleep(INFINITE);
}