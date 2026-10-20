#ifndef SOCKET_COMPAT_H
#define SOCKET_COMPAT_H

/**
 * @brief Capa de compatibilidad de sockets (Windows Winsock2 / Linux POSIX).
 * Solo contiene funciones utilitarias inline en el namespace "red"; no es una clase.
 * Permite que ClienteSocket y ServidorSocket compilen igual en .dll (Windows) y .so (Linux).
 */

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef SOCKET socket_t;
    #define SOCKET_INVALIDO INVALID_SOCKET
    #ifndef MSG_NOSIGNAL
    #define MSG_NOSIGNAL 0
    #endif
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <sys/select.h>
    #include <sys/time.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <cerrno>
    typedef int socket_t;
    #define SOCKET_INVALIDO (-1)
#endif

namespace red {

// Inicializa la pila de red (Winsock en Windows; no hace nada en Linux).
inline bool iniciar() {
#ifdef _WIN32
    WSADATA wsa;
    return WSAStartup(MAKEWORD(2, 2), &wsa) == 0;
#else
    return true;
#endif
}

inline void finalizar() {
#ifdef _WIN32
    WSACleanup();
#endif
}

inline bool esValido(socket_t s) { return s != SOCKET_INVALIDO; }

inline void cerrar(socket_t s) {
#ifdef _WIN32
    closesocket(s);
#else
    close(s);
#endif
}

inline int ultimoError() {
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
}

// Tiempo maximo de espera de recv() en el socket (evita bloqueos con clientes colgados).
inline void establecerTimeoutRecepcion(socket_t s, int ms) {
#ifdef _WIN32
    DWORD t = static_cast<DWORD>(ms);
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&t), sizeof(t));
#else
    timeval tv;
    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif
}

// Espera hasta "ms" milisegundos a que el socket tenga datos / conexiones entrantes.
// Retorna >0 si hay algo para leer, 0 si vencio el tiempo, <0 si hubo error.
inline int esperarLectura(socket_t s, int ms) {
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(s, &fds);
    timeval tv;
    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
#ifdef _WIN32
    return select(0, &fds, nullptr, nullptr, &tv);
#else
    return select(s + 1, &fds, nullptr, nullptr, &tv);
#endif
}

inline bool enviarTodo(socket_t s, const void* datos, size_t n) {
    const char* p = static_cast<const char*>(datos);
    size_t enviados = 0;
    while (enviados < n) {
        int r = static_cast<int>(send(s, p + enviados, static_cast<int>(n - enviados), MSG_NOSIGNAL));
        if (r <= 0) return false;
        enviados += static_cast<size_t>(r);
    }
    return true;
}

// TCP es un flujo de bytes: un recv() puede devolver menos de lo pedido.
// Esta funcion repite recv() hasta completar exactamente "n" bytes (o falla por cierre/timeout).
inline bool recibirExacto(socket_t s, void* buffer, size_t n) {
    char* p = static_cast<char*>(buffer);
    size_t recibidos = 0;
    while (recibidos < n) {
        int r = static_cast<int>(recv(s, p + recibidos, static_cast<int>(n - recibidos), 0));
        if (r <= 0) return false;
        recibidos += static_cast<size_t>(r);
    }
    return true;
}

// connect() con tiempo limite (por defecto el sistema puede tardar >20 s con una IP inalcanzable).
inline bool conectarConTimeout(socket_t s, const sockaddr* dir, int largo, int ms) {
#ifdef _WIN32
    u_long noBloq = 1;
    ioctlsocket(s, FIONBIO, &noBloq);
    int r = connect(s, dir, largo);
    bool enProgreso = (r == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK);
#else
    int flags = fcntl(s, F_GETFL, 0);
    fcntl(s, F_SETFL, flags | O_NONBLOCK);
    int r = connect(s, dir, static_cast<socklen_t>(largo));
    bool enProgreso = (r < 0 && errno == EINPROGRESS);
#endif
    bool ok = (r == 0);
    if (!ok && enProgreso) {
        fd_set escritura;
        FD_ZERO(&escritura);
        FD_SET(s, &escritura);
        timeval tv;
        tv.tv_sec = ms / 1000;
        tv.tv_usec = (ms % 1000) * 1000;
#ifdef _WIN32
        int sel = select(0, nullptr, &escritura, nullptr, &tv);
        int err = 0;
        int len = sizeof(err);
        getsockopt(s, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&err), &len);
#else
        int sel = select(s + 1, nullptr, &escritura, nullptr, &tv);
        int err = 0;
        socklen_t len = sizeof(err);
        getsockopt(s, SOL_SOCKET, SO_ERROR, &err, &len);
#endif
        ok = (sel > 0 && err == 0);
    }
    // Restaurar modo bloqueante
#ifdef _WIN32
    u_long bloq = 0;
    ioctlsocket(s, FIONBIO, &bloq);
#else
    fcntl(s, F_SETFL, flags);
#endif
    return ok;
}

} // namespace red

#endif // SOCKET_COMPAT_H
