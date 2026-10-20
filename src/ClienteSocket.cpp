#include "../include/ClienteSocket.h"
#include "../include/SocketCompat.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <random>

namespace {
const int TIMEOUT_CONEXION_MS = 3000;
const int TIMEOUT_RESPUESTA_MS = 3000;
}

ClienteSocket::ClienteSocket(const std::string& ip, int p)
    : ipServidor(ip), puerto(p), redLista(false), activo(true),
      intervaloMinMs(2000), intervaloMaxMs(5000) {
    redLista = red::iniciar();     // WSAStartup una sola vez (antes se hacia en cada envio)
}

ClienteSocket::~ClienteSocket() {
    if (redLista) red::finalizar();
}

void ClienteSocket::detener() {
    activo = false;
}

void ClienteSocket::configurarIntervalo(int minMs, int maxMs) {
    if (minMs < 1) minMs = 1;
    if (maxMs < minMs) maxMs = minMs;
    intervaloMinMs = minMs;
    intervaloMaxMs = maxMs;
}

bool ClienteSocket::enviarTrama(const TramaVehiculo& trama) {
    if (!redLista) return false;

    // Resolver IP o nombre de host (sirve "127.0.0.1" y tambien "localhost")
    addrinfo pistas;
    std::memset(&pistas, 0, sizeof(pistas));
    pistas.ai_family = AF_INET;
    pistas.ai_socktype = SOCK_STREAM;
    addrinfo* resultado = nullptr;
    if (getaddrinfo(ipServidor.c_str(), std::to_string(puerto).c_str(), &pistas, &resultado) != 0 || resultado == nullptr) {
        return false;
    }

    socket_t sock = socket(resultado->ai_family, resultado->ai_socktype, resultado->ai_protocol);
    if (!red::esValido(sock)) {
        freeaddrinfo(resultado);
        return false;
    }

    bool conectado = red::conectarConTimeout(sock, resultado->ai_addr, static_cast<int>(resultado->ai_addrlen),
                                             TIMEOUT_CONEXION_MS);
    freeaddrinfo(resultado);
    if (!conectado) {
        red::cerrar(sock);
        return false;               // Servidor no disponible
    }

    // La trama viaja como bytes crudos; "celda" se convierte a orden de red (big-endian)
    TramaVehiculo enRed = trama;
    enRed.celda = static_cast<int>(htonl(static_cast<uint32_t>(trama.celda)));

    if (!red::enviarTodo(sock, &enRed, TAM_TRAMA)) {
        red::cerrar(sock);
        return false;
    }

    // Recibe la confirmacion del servidor (con tiempo limite)
    red::establecerTimeoutRecepcion(sock, TIMEOUT_RESPUESTA_MS);
    char buffer[512] = {0};
    int bytes = static_cast<int>(recv(sock, buffer, sizeof(buffer) - 1, 0));
    if (bytes > 0) {
        std::cout << " -> [Respuesta Servidor]: " << buffer << std::endl;
    }

    red::cerrar(sock);
    return true;
}

void ClienteSocket::iniciarTransmision(GeneradorPlacas* gen) {
    if (gen == nullptr) return;

    std::cout << "=== EMISOR DE PLACAS C++ (Cada " << intervaloMinMs / 1000.0 << " a "
              << intervaloMaxMs / 1000.0 << " seg) ===" << std::endl;

    // Generador propio para los intervalos: std::rand() sin srand() repetia siempre la misma secuencia
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> distEspera(intervaloMinMs, intervaloMaxMs);

    while (activo) {
        std::string placa = gen->generarPlacaAleatoria();   // Usa puntero al generador
        int celda = gen->elegirCelda();                     // Celda sugerida
        std::string hora = gen->obtenerHoraActual();        // Hora actual

        TramaVehiculo trama = crearTrama(placa, celda, hora);
        std::cout << "[Enviando]: " << placa << ";" << celda << ";" << hora
                  << "  (" << TAM_TRAMA << " bytes)" << std::endl;

        if (!enviarTrama(trama)) {
            std::cout << " [!] Servidor no disponible en " << ipServidor << ":" << puerto << "..." << std::endl;
        }

        // Pausa aleatoria de 2 a 5 segundos; se duerme en tramos de 50 ms para poder detenerse rapido (Ctrl+C)
        int espera = distEspera(rng);
        for (int t = 0; t < espera && activo; t += 50) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    std::cout << "[Emisor detenido]" << std::endl;
}
