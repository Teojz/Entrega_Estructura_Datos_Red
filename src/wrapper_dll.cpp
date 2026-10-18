#include "../include/Parqueadero.h"   // Modelo del parqueadero
#include "../include/ServidorSocket.h" // Servidor que recibe las tramas
#include <thread>                      // Hilo para el socket
#include <string>                      // Manejo de cadenas de texto
#include <cstring>                     // Funcion strncpy para copiar texto

#ifdef _WIN32                          // Entorno Windows
#define EXPORT __declspec(dllexport)   // Exporta las funciones a la DLL
#else                                  // Linux u otros
#define EXPORT
#endif

// extern "C" evita que el compilador altere los nombres (permite enlace limpio con ctypes en Python)
extern "C" {
    Parqueadero* g_parqueadero = nullptr; // Puntero global al modelo del parqueadero
    ServidorSocket* g_servidor = nullptr; // Puntero global al socket servidor
    static char bufferEstado[4096];       // Buffer de memoria para pasar cadenas a Python

    // Exportada: instancia el modelo y lanza el socket de escucha en segundo plano
    EXPORT void iniciar_receptor_parqueadero(int numCeldas, int puerto) {
        if (g_parqueadero != nullptr) delete g_parqueadero; // Libera memoria previa si existia
        if (g_servidor != nullptr) delete g_servidor;       // Libera memoria del servidor

        g_parqueadero = new Parqueadero(numCeldas);         // Reserva dinámica en Heap
        g_servidor = new ServidorSocket(puerto, g_parqueadero); // Asocia servidor con el modelo

        // Inicia el socket en un hilo aparte para no bloquear la interfaz grafica de Python
        std::thread hilo(&ServidorSocket::iniciarEscucha, g_servidor);
        hilo.detach(); // Corre libre en segundo plano
    }

    // Exportada: entrega a Python el estado de las celdas delimitado por '|'
    EXPORT const char* obtener_estado_celdas() {
        if (!g_parqueadero) return "";
        std::string estado = g_parqueadero->obtenerEstadoSerializado(); // Pide el texto serializado
        std::strncpy(bufferEstado, estado.c_str(), sizeof(bufferEstado) - 1);
        bufferEstado[sizeof(bufferEstado) - 1] = '\0';
        return bufferEstado;
    }
}