#include "../include/ClienteSocket.h" // Cabecera del cliente
#include <iostream>                   // Salida en consola
#include <winsock2.h>                 // Sockets para Windows
#include <thread>                     // Pausa de ejecucion
#include <chrono>                     // Medicion de tiempo
#include <cstdlib>                    // Numeros aleatorios con rand()

#pragma comment(lib, "ws2_32.lib")

ClienteSocket::ClienteSocket(const std::string& ip, int p) 
    : ipServidor(ip), puerto(p) {}    // Inicializa IP y puerto

bool ClienteSocket::enviarTrama(const std::string& trama) {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return false;

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0); // Crea el socket TCP
    if (sock == INVALID_SOCKET) {
        WSACleanup();
        return false;
    }

    sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(puerto);
    servAddr.sin_addr.s_addr = inet_addr(ipServidor.c_str());

    if (connect(sock, (sockaddr*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR) {
        closesocket(sock);
        WSACleanup();
        return false; // Servidor no disponible
    }

    send(sock, trama.c_str(), trama.length(), 0); // Envia la trama

    char buffer[512] = {0};
    int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0); // Recibe confirmacion
    if (bytes > 0) {
        std::cout << " -> [Respuesta Servidor]: " << buffer << "\n";
    }

    closesocket(sock);
    WSACleanup();
    return true;
}

void ClienteSocket::iniciarTransmision(GeneradorPlacas* gen) {
    std::cout << "=== EMISOR DE PLACAS C++ (Cada 2 a 5 seg) ===\n";
    
    while (true) {
        std::string placa = gen->generarPlacaAleatoria(); // Usa puntero al generador
        int celda = gen->elegirCelda();                   // Celda sugerida
        std::string hora = gen->obtenerHoraActual();      // Hora actual

        std::string trama = placa + ";" + std::to_string(celda) + ";" + hora;
        std::cout << "[Enviando]: " << trama << "\n";

        if (!enviarTrama(trama)) {
            std::cout << " [!] Servidor no disponible en " << ipServidor << ":" << puerto << "...\n";
        }

        // Pausa aleatoria de 2 a 5 segundos segun la guia
        int espera = 2 + (std::rand() % 4); 
        std::this_thread::sleep_for(std::chrono::seconds(espera));
    }
}