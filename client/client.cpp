#include <iostream>
#include <string>
#include <thread>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

const int PORT = 8080;
const char *SERVER_IP = "127.0.0.1";
bool running = true;

void receiveMessages(SOCKET sock) {
    char buffer[1024];
    while (running) {
        memset(buffer, 0, sizeof(buffer));
        int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes <= 0) {
            if (running) {
                std::cout << "\n❌ Desconectado del servidor" << std::endl;
            }
            break;
        }
        
        buffer[bytes] = '\0';
        std::cout << "\n📩 " << buffer << std::endl;
        std::cout << "> " << std::flush;
    }
}

int main() {
    std::string dispositivo;
    std::cout << "🔷 Ingresa el nombre único de este dispositivo: ";
    std::getline(std::cin, dispositivo);
    
    #ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "❌ Error Winsock" << std::endl;
            return 1;
        }
    #endif
    
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "❌ Error socket" << std::endl;
        return 1;
    }
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);
    
    std::cout << "🔄 Conectando..." << std::endl;
    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "❌ Error conectando" << std::endl;
        closesocket(sock);
        return 1;
    }
    
    send(sock, dispositivo.c_str(), dispositivo.length(), 0);
    
    std::cout << "✅ Conectado como " << dispositivo << std::endl;
    std::cout << "📝 Formato: destinatario:mensaje" << std::endl;
    std::cout << "   Escribe 'salir' para terminar\n" << std::endl;
    
    std::thread receiver(receiveMessages, sock);
    
    std::string input;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);
        
        if (input == "salir") {
            running = false;
            break;
        }
        
        if (input.empty()) continue;
        
        send(sock, input.c_str(), input.length(), 0);
    }
    
    closesocket(sock);
    #ifdef _WIN32
        WSACleanup();
    #endif
    
    if (receiver.joinable()) {
        receiver.join();
    }
    
    std::cout << "👋 Hasta luego!" << std::endl;
    return 0;
}