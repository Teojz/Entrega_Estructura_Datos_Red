#include "../include/GeneradorPlacas.h" // Cabecera del generador de placas
#include "../include/ClienteSocket.h"   // Cabecera del cliente socket de red
#include <iostream>                     // Para entrada y salida por consola

int main() {
    int maxCeldas = 12;                 // Total de celdas configuradas en el parqueadero
    int puerto = 8080;                  // Puerto de red acordado para la transmision
    std::string ipServidor = "127.0.0.1"; // IP local por defecto

    std::cout << "==============================================\n"; // Formato visual
    std::cout << "   EMISOR AUTOMATICO DE PLACAS POR RED (C++)  \n"; // Titulo del modulo
    std::cout << "==============================================\n";
    std::cout << "Ingrese la IP del servidor receptor (Enter para 127.0.0.1): ";
    
    std::string ipEntrada;              // Variable para leer la IP ingresada
    std::getline(std::cin, ipEntrada);  // Captura toda la linea

    if (!ipEntrada.empty()) {           // Si el usuario escribio una IP remota
        ipServidor = ipEntrada;         // Asigna esa IP para enviar por socket
    }

    // Instanciacion en el Heap mediante memoria dinamica (estilo del profesor)
    GeneradorPlacas* generador = new GeneradorPlacas(maxCeldas);   // Crea el generador en memoria
    ClienteSocket* cliente = new ClienteSocket(ipServidor, puerto); // Crea el cliente socket

    // Inicia el bucle continuo de transmision cada 2-5 seg pasando el puntero
    cliente->iniciarTransmision(generador); // Bucle infinito de envio

    // Liberacion de memoria dinamica para evitar fugas (memory leaks)
    delete cliente;                     // Destruye el objeto cliente
    delete generador;                   // Destruye el objeto generador

    return 0;                           // Retorno exitoso
}