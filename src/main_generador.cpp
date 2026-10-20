#include "../include/GeneradorPlacas.h" // Cabecera del generador de placas
#include "../include/ClienteSocket.h"   // Cabecera del cliente socket de red
#include <iostream>                     // Para entrada y salida por consola
#include <cstdlib>                      // strtol
#include <csignal>                      // Captura de Ctrl+C para cerrar limpio

// Puntero global SOLO para que el manejador de Ctrl+C pueda pedir la parada (detener() solo escribe un atomic).
static ClienteSocket* g_cliente = nullptr;

static void manejarSenal(int) {
    if (g_cliente != nullptr) g_cliente->detener();
}

static bool leerEntero(const char* texto, int minimo, int maximo, int& salida) {
    char* fin = nullptr;
    long v = std::strtol(texto, &fin, 10);
    if (fin == texto || *fin != '\0' || v < minimo || v > maximo) return false;
    salida = static_cast<int>(v);
    return true;
}

/**
 * Uso:
 *   generador                                  -> pregunta la IP por consola (Enter = 127.0.0.1)
 *   generador <ip> [puerto] [minMs] [maxMs]    -> sin preguntas (util para pruebas/automatizacion)
 */
int main(int argc, char* argv[]) {
    int maxCeldas = 12;                 // Total de celdas configuradas en el parqueadero
    int puerto = 8080;                  // Puerto de red acordado para la transmision
    int minMs = 2000;                   // Intervalo minimo entre tramas (2 s)
    int maxMs = 5000;                   // Intervalo maximo entre tramas (5 s)
    std::string ipServidor = "127.0.0.1"; // IP local por defecto

    if (argc >= 2) {
        ipServidor = argv[1];
        if (argc >= 3 && !leerEntero(argv[2], 1, 65535, puerto)) {
            std::cerr << "Puerto invalido: " << argv[2] << std::endl;
            return 1;
        }
        if (argc >= 4 && !leerEntero(argv[3], 1, 600000, minMs)) {
            std::cerr << "minMs invalido: " << argv[3] << std::endl;
            return 1;
        }
        if (argc >= 5 && !leerEntero(argv[4], 1, 600000, maxMs)) {
            std::cerr << "maxMs invalido: " << argv[4] << std::endl;
            return 1;
        }
    } else {
        std::cout << "==============================================\n"; // Formato visual
        std::cout << "   EMISOR AUTOMATICO DE PLACAS POR RED (C++)  \n"; // Titulo del modulo
        std::cout << "==============================================\n";
        std::cout << "Ingrese la IP del servidor receptor (Enter para 127.0.0.1): " << std::flush;

        std::string ipEntrada;              // Variable para leer la IP ingresada
        std::getline(std::cin, ipEntrada);  // Captura toda la linea

        if (!ipEntrada.empty()) {           // Si el usuario escribio una IP remota
            ipServidor = ipEntrada;         // Asigna esa IP para enviar por socket
        }
    }

    // Instanciacion en el Heap mediante memoria dinamica (estilo del profesor)
    GeneradorPlacas* generador = new GeneradorPlacas(maxCeldas);   // Crea el generador en memoria
    ClienteSocket* cliente = new ClienteSocket(ipServidor, puerto); // Crea el cliente socket
    cliente->configurarIntervalo(minMs, maxMs);

    g_cliente = cliente;
    std::signal(SIGINT, manejarSenal);
    std::signal(SIGTERM, manejarSenal);

    // Bucle continuo de transmision cada 2-5 seg pasando el puntero; termina con Ctrl+C
    cliente->iniciarTransmision(generador);

    // Liberacion de memoria dinamica (antes era codigo inalcanzable porque el bucle era infinito)
    g_cliente = nullptr;
    delete cliente;                     // Destruye el objeto cliente
    delete generador;                   // Destruye el objeto generador

    return 0;                           // Retorno exitoso
}
