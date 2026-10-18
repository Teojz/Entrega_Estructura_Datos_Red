#include "../include/ServidorSocket.h" // Cabecera de la clase
#include <iostream>                     // Mensajes por consola
#include <sstream>                      // Separar la trama de texto
#include <winsock2.h>                   // Sockets para Windows

#pragma comment(lib, "ws2_32.lib")      // Enlace con la libreria de red

ServidorSocket::ServidorSocket(int p, Parqueadero* pq) 
    : puerto(p), activo(false), parqueadero(pq) {} // Guarda puntero al parqueadero

ServidorSocket::~ServidorSocket() {
    detener();                          // Cierra el socket al destruir
}

void ServidorSocket::detener() {
    activo = false;                     // Detiene el bucle de escucha
}

void ServidorSocket::iniciarEscucha() {
    WSADATA wsa;                        // Estructura Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return;

    SOCKET serverSock = socket(AF_INET, SOCK_STREAM, 0); // Socket TCP
    if (serverSock == INVALID_SOCKET) {
        WSACleanup();
        return;
    }

    sockaddr_in serverAddr;             // Configuracion del servidor
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Escucha cualquier IP en la red
    serverAddr.sin_port = htons(puerto); // Asigna el puerto de red

    if (bind(serverSock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(serverSock);
        WSACleanup();
        return;
    }

    listen(serverSock, 5);              // Pone el socket en escucha
    activo = true;

    while (activo) {
        sockaddr_in clientAddr;
        int clientLen = sizeof(clientAddr);
        SOCKET clientSock = accept(serverSock, (sockaddr*)&clientAddr, &clientLen); // Acepta cliente

        if (clientSock != INVALID_SOCKET) {
            char buffer[512] = {0};     // Buffer para recibir trama
            int bytes = recv(clientSock, buffer, sizeof(buffer) - 1, 0);

            if (bytes > 0 && parqueadero != nullptr) {
                std::string trama(buffer);
                std::stringstream ss(trama);
                std::string placa, celdaStr, hora;

                // Formato de trama: PLACA;CELDA;HORA
                std::getline(ss, placa, ';');
                std::getline(ss, celdaStr, ';');
                std::getline(ss, hora, ';');

                int celdaSugerida = std::stoi(celdaStr);
                // Procesa en el parqueadero a traves del puntero
                std::string res = parqueadero->procesarEventoPlaca(placa, celdaSugerida, hora);
                send(clientSock, res.c_str(), res.length(), 0); // Envia respuesta
            }
            closesocket(clientSock);    // Cierra socket del cliente
        }
    }

    closesocket(serverSock);
    WSACleanup();
}