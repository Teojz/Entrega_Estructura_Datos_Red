#include "../include/Parqueadero.h"   // Modelo
#include "../include/ServidorSocket.h" // Servidor
#include <thread>                      // Hilo para el socket
#include <string>
#include <cstring>

#ifdef _WIN32
#define EXPORT __declspec(dllexport)   // Exporta funcion para Windows DLL
#else
#define EXPORT
#endif

extern "C" {
    Parqueadero* g_parqueadero = nullptr; // Puntero al parqueadero
    ServidorSocket* g_servidor = nullptr; // Puntero al socket servidor
    static char bufferEstado[4096];       // Buffer para devolver estado a Python

    EXPORT void iniciar_receptor_parqueadero(int numCeldas, int puerto) {
        if (g_parqueadero != nullptr) delete g_parqueadero;
        if (g_servidor != nullptr) delete g_servidor;

        g_parqueadero = new Parqueadero(numCeldas);
        g_servidor = new ServidorSocket(puerto, g_parqueadero);

        // Hilo en segundo plano para no congelar la ventana de Python
        std::thread hilo(&ServidorSocket::iniciarEscucha, g_servidor);
        hilo.detach();
    }

    EXPORT const char* obtener_estado_celdas() {
        if (!g_parqueadero) return "";
        std::string estado = g_parqueadero->obtenerEstadoSerializado();
        std::strncpy(bufferEstado, estado.c_str(), sizeof(bufferEstado) - 1);
        bufferEstado[sizeof(bufferEstado) - 1] = '\0';
        return bufferEstado;
    }
}