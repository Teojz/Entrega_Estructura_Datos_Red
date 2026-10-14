#include "../include/GeneradorPlacas.h" // Modulo generador
#include "../include/ClienteSocket.h"   // Modulo transmisor
#include <iostream>                     // Entrada/salida

int main() {
    int maxCeldas = 12;                 // Total de celdas
    int puerto = 8080;                  // Puerto de conexion
    std::string ipServidor = "127.0.0.1"; // IP local por defecto

    std::cout << "--- GENERADOR DE PLACAS AUTOMATICO (C++) ---\n";
    std::cout << "Ingrese IP del servidor (Enter para 127.0.0.1): ";
    std::string ipEntrada;
    std::getline(std::cin, ipEntrada);

    if (!ipEntrada.empty()) {
        ipServidor = ipEntrada;
    }

    // Instanciacion en memoria dinamica con punteros (estilo del profesor)
    GeneradorPlacas* generador = new GeneradorPlacas(maxCeldas);
    ClienteSocket* cliente = new ClienteSocket(ipServidor, puerto);

    cliente->iniciarTransmision(generador); // Pasa puntero por parametro

    delete cliente;                     // Libera memoria
    delete generador;

    return 0;
}