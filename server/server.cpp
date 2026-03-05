#include <iostream>
#include <thread>
#include <vector>
#include <cstring>
#include <map>
#include <mutex>
#include <sstream>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

#include "../database/database.h"

const int PORT = 8080;
Database db("chat.db");
std::map<std::string, SOCKET> clientSockets;
std::mutex clientsMutex;

void handleClient(SOCKET clientSocket)
{
    char buffer[1024];
    std::string dispositivo;

    int bytes = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytes > 0)
    {
        buffer[bytes] = '\0';
        dispositivo = buffer;

        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clientSockets[dispositivo] = clientSocket;
        }

        std::cout << " Conectado: " << dispositivo << std::endl;
    }

    while (true)
    {
        memset(buffer, 0, sizeof(buffer));
        bytes = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (bytes <= 0)
            break;

        buffer[bytes] = '\0';
        std::string mensaje(buffer);

        size_t pos = mensaje.find(':');
        if (pos != std::string::npos)
        {
            std::string destinatario = mensaje.substr(0, pos);
            std::string contenido = mensaje.substr(pos + 1);

            db.guardarMensaje(dispositivo, destinatario, contenido);

            std::lock_guard<std::mutex> lock(clientsMutex);
            auto it = clientSockets.find(destinatario);
            if (it != clientSockets.end())
            {
                std::string msg = dispositivo + ": " + contenido;
                send(it->second, msg.c_str(), msg.length(), 0);
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clientSockets.erase(dispositivo);
    }

    closesocket(clientSocket);
}

int main()
{
    std::cout << " Iniciando servidor..." << std::endl;

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << " Error Winsock" << std::endl;
        return 1;
    }
#endif

    if (!db.inicializar())
    {
        std::cerr << " Error DB" << std::endl;
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET)
    {
        std::cerr << " Error socket" << std::endl;
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << " Error bind" << std::endl;
        closesocket(serverSocket);
        return 1;
    }

    if (listen(serverSocket, 5) == SOCKET_ERROR)
    {
        std::cerr << " Error listen" << std::endl;
        closesocket(serverSocket);
        return 1;
    }

    std::cout << " Servidor en puerto " << PORT << std::endl;

    while (true)
    {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket != INVALID_SOCKET)
        {
            std::thread(handleClient, clientSocket).detach();
        }
    }

    closesocket(serverSocket);
#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}