#include "../include/Parqueadero.h"    // Modelo del parqueadero
#include "../include/ServidorSocket.h" // Servidor que recibe las tramas
#include <thread>                      // Hilo para el socket
#include <mutex>                       // Protege los punteros globales
#include <chrono>                      // Espera de arranque del servidor
#include <string>                      // Manejo de cadenas de texto

#ifdef _WIN32                          // Entorno Windows
#define EXPORT __declspec(dllexport)   // Exporta las funciones a la DLL
#else                                  // Linux u otros
#define EXPORT __attribute__((visibility("default")))
#endif

// Estado global de la libreria (privado de este archivo)
static Parqueadero*    g_parqueadero = nullptr;  // Modelo del parqueadero en el Heap
static ServidorSocket* g_servidor    = nullptr;  // Servidor TCP en el Heap
static std::thread*    g_hilo        = nullptr;  // Hilo del servidor (puntero: nunca se destruye solo al descargar la DLL)
static std::mutex      g_mutex;                  // Protege los punteros anteriores
static std::string     g_bufEstado;              // Buffers estables: Python lee el char* despues del return
static std::string     g_bufBitacora;
static std::string     g_ultimoError;

// Detiene el hilo, espera a que termine (join) y libera la memoria. Requiere g_mutex tomado.
static void detenerInterno() {
    if (g_servidor != nullptr) g_servidor->detener();
    if (g_hilo != nullptr) {
        if (g_hilo->joinable()) g_hilo->join();   // join: el hilo termina ANTES de borrar el servidor
        delete g_hilo;
        g_hilo = nullptr;
    }
    delete g_servidor;      g_servidor = nullptr;
    delete g_parqueadero;   g_parqueadero = nullptr;
}

// extern "C" evita que el compilador altere los nombres (permite enlace limpio con ctypes en Python)
extern "C" {

    // Instancia el modelo y lanza el socket de escucha en segundo plano.
    // Retorna: 0 = OK, -1 = el servidor no pudo iniciar (p. ej. puerto ocupado; ver obtener_ultimo_error), -2 = parametros invalidos
    EXPORT int iniciar_receptor_parqueadero(int numCeldas, int puerto) {
        std::lock_guard<std::mutex> lock(g_mutex);
        detenerInterno();                                   // Si ya habia uno, se cierra bien antes de crear otro
        g_ultimoError.clear();

        if (numCeldas < 1 || numCeldas > 1000 || puerto < 1 || puerto > 65535) {
            g_ultimoError = "Parametros invalidos (celdas 1-1000, puerto 1-65535)";
            return -2;
        }

        g_parqueadero = new Parqueadero(numCeldas);                 // Reserva dinámica en Heap
        g_servidor = new ServidorSocket(puerto, g_parqueadero);     // Asocia servidor con el modelo

        // Inicia el socket en un hilo aparte para no bloquear la interfaz grafica de Python
        g_hilo = new std::thread(&ServidorSocket::iniciarEscucha, g_servidor);

        // Espera (max. 3 s) a que el servidor confirme que quedo escuchando o que fallo
        for (int i = 0; i < 300 && g_servidor->getEstado() == 0; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        if (g_servidor->getEstado() != 1) {
            g_ultimoError = g_servidor->getUltimoError();
            if (g_ultimoError.empty()) g_ultimoError = "El servidor no respondio al iniciar";
            detenerInterno();
            return -1;
        }
        return 0;
    }

    // Detiene el servidor y libera toda la memoria (llamar al cerrar la GUI).
    EXPORT void detener_receptor_parqueadero() {
        std::lock_guard<std::mutex> lock(g_mutex);
        detenerInterno();
    }

    // Entrega a Python el estado de las celdas: ID,ESTADO,PLACA,HORA|ID,ESTADO,PLACA,HORA|...
    EXPORT const char* obtener_estado_celdas() {
        std::lock_guard<std::mutex> lock(g_mutex);
        if (!g_parqueadero) { g_bufEstado.clear(); return g_bufEstado.c_str(); }
        g_bufEstado = g_parqueadero->obtenerEstadoSerializado(); // std::string dinamico: sin truncar como el buffer fijo de 4096
        return g_bufEstado.c_str();
    }

    // Entrega a Python el registro de actividad del socket (una linea por evento, separadas por '\n').
    EXPORT const char* obtener_bitacora() {
        std::lock_guard<std::mutex> lock(g_mutex);
        if (!g_parqueadero) { g_bufBitacora.clear(); return g_bufBitacora.c_str(); }
        g_bufBitacora = g_parqueadero->obtenerBitacora(50);
        return g_bufBitacora.c_str();
    }

    EXPORT int obtener_celdas_ocupadas() {
        std::lock_guard<std::mutex> lock(g_mutex);
        return g_parqueadero ? g_parqueadero->getCeldasOcupadas() : 0;
    }

    EXPORT int obtener_celdas_disponibles() {
        std::lock_guard<std::mutex> lock(g_mutex);
        return g_parqueadero ? g_parqueadero->getCeldasDisponibles() : 0;
    }

    // Texto del ultimo error de iniciar_receptor_parqueadero (vacio si no hubo error).
    EXPORT const char* obtener_ultimo_error() {
        std::lock_guard<std::mutex> lock(g_mutex);
        return g_ultimoError.c_str();
    }
}
