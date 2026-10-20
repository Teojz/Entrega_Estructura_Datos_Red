#include "../include/ServidorSocket.h"
#include "../include/SocketCompat.h"
#include "../include/TramaVehiculo.h"

namespace {

const int TIMEOUT_CLIENTE_MS = 2000;   // Un cliente lento no puede bloquear al servidor
const int SONDEO_ACCEPT_MS   = 200;    // Cada cuanto se revisa si se pidio detener

// Atiende a UN cliente: lee la trama completa, la valida y responde con el resultado.
void atenderCliente(socket_t cliente, Parqueadero* parqueadero) {
    red::establecerTimeoutRecepcion(cliente, TIMEOUT_CLIENTE_MS);

    TramaVehiculo trama;
    if (!red::recibirExacto(cliente, &trama, TAM_TRAMA)) {
        parqueadero->registrarEvento("[TRAMA INVALIDA] Trama incompleta o conexion cerrada antes de tiempo.");
        const char* msg = "[ERROR] Trama incompleta";
        red::enviarTodo(cliente, msg, std::strlen(msg));
        return;
    }

    if (!tramaValida(trama)) {
        parqueadero->registrarEvento("[TRAMA INVALIDA] Placa u hora con formato incorrecto, trama descartada.");
        const char* msg = "[ERROR] Trama invalida";
        red::enviarTodo(cliente, msg, std::strlen(msg));
        return;
    }

    int celdaSugerida = static_cast<int>(ntohl(static_cast<uint32_t>(trama.celda)));
    std::string res = parqueadero->procesarEventoPlaca(placaDeTrama(trama), celdaSugerida, horaDeTrama(trama));
    red::enviarTodo(cliente, res.c_str(), res.length());
}

} // namespace

ServidorSocket::ServidorSocket(int p, Parqueadero* pq)
    : puerto(p), activo(false), detenido(false), estado(0), ultimoError(""), parqueadero(pq) {}

ServidorSocket::~ServidorSocket() {
    detener();                          // El hilo que ejecuta iniciarEscucha() debe unirse (join) antes de destruir
}

void ServidorSocket::detener() {
    detenido = true;                    // El bucle de escucha lo detecta en <= 200 ms
}

bool ServidorSocket::estaActivo() const { return activo; }
int ServidorSocket::getEstado() const { return estado; }
std::string ServidorSocket::getUltimoError() const { return ultimoError; }

void ServidorSocket::iniciarEscucha() {
    if (parqueadero == nullptr) {
        ultimoError = "Parqueadero nulo";
        estado = -1;
        return;
    }
    if (!red::iniciar()) {
        ultimoError = "No se pudo inicializar la pila de red (WSAStartup)";
        estado = -1;
        return;
    }

    socket_t servidor = socket(AF_INET, SOCK_STREAM, 0);   // Socket TCP
    if (!red::esValido(servidor)) {
        ultimoError = "No se pudo crear el socket (codigo " + std::to_string(red::ultimoError()) + ")";
        estado = -1;
        red::finalizar();
        return;
    }

#ifndef _WIN32
    // Permite reiniciar el servidor de inmediato sin esperar el TIME_WAIT del puerto (solo Linux;
    // en Windows esta opcion permitiria "robar" el puerto, por eso no se activa).
    int uno = 1;
    setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &uno, sizeof(uno));
#endif

    sockaddr_in direccion;
    std::memset(&direccion, 0, sizeof(direccion));
    direccion.sin_family = AF_INET;
    direccion.sin_addr.s_addr = htonl(INADDR_ANY);          // Escucha en cualquier interfaz
    direccion.sin_port = htons(static_cast<uint16_t>(puerto));

    if (bind(servidor, reinterpret_cast<sockaddr*>(&direccion), sizeof(direccion)) != 0) {
        ultimoError = "No se pudo usar el puerto " + std::to_string(puerto) +
                      " (ya esta ocupado?). Codigo " + std::to_string(red::ultimoError());
        estado = -1;
        red::cerrar(servidor);
        red::finalizar();
        return;
    }

    if (listen(servidor, 16) != 0) {
        ultimoError = "listen() fallo (codigo " + std::to_string(red::ultimoError()) + ")";
        estado = -1;
        red::cerrar(servidor);
        red::finalizar();
        return;
    }

    activo = true;
    estado = 1;
    parqueadero->registrarEvento("[SERVIDOR] Escuchando en el puerto " + std::to_string(puerto) + ".");

    while (!detenido) {
        // select() con tiempo limite: accept() ya no bloquea para siempre y detener() funciona.
        int listo = red::esperarLectura(servidor, SONDEO_ACCEPT_MS);
        if (listo <= 0) continue;

        socket_t cliente = accept(servidor, nullptr, nullptr);
        if (!red::esValido(cliente)) continue;

        atenderCliente(cliente, parqueadero);
        red::cerrar(cliente);
    }

    parqueadero->registrarEvento("[SERVIDOR] Detenido.");
    red::cerrar(servidor);
    red::finalizar();
    activo = false;
    estado = 2;
}
