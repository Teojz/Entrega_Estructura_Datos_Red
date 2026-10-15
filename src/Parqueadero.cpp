#include "../include/Parqueadero.h"
#include <sstream>
#include <iostream>

Parqueadero::Parqueadero(int cap) : capacidadTotal(cap), celdas(nullptr) {
    if (capacidadTotal < 1) {
        capacidadTotal = 10; // Capacidad mínima de seguridad
    }
    // Asignación de memoria dinámica para el arreglo de celdas
    celdas = new Celda[capacidadTotal];
    for (int i = 0; i < capacidadTotal; ++i) {
        celdas[i].inicializar(i + 1); // Celdas identificadas del 1 al N
    }
}

Parqueadero::~Parqueadero() {
    // Liberación del arreglo dinámico de celdas
    if (celdas != nullptr) {
        delete[] celdas;
        celdas = nullptr;
    }
}

int Parqueadero::buscarPlaca(std::string placa) {
    if (celdas == nullptr || placa.empty()) {
        return -1;
    }
    for (int i = 0; i < capacidadTotal; ++i) {
        if (celdas[i].estaOcupada()) {
            RegistroVehiculo* v = celdas[i].getVehiculo();
            if (v != nullptr && v->getPlaca() == placa) {
                return i; // Retorna el índice de la celda encontrada
            }
        }
    }
    return -1; // No encontrada
}

std::string Parqueadero::procesarEventoPlaca(std::string placa, int celdaSugerida, std::string hora) {
    std::lock_guard<std::mutex> lock(mutexAcceso);

    if (celdas == nullptr) {
        return "[ERROR] Sistema de celdas no inicializado.";
    }

    // Regla de Negocio: Verificar si la placa ya se encuentra dentro del parqueadero
    int posExistente = buscarPlaca(placa);

    if (posExistente != -1) {
        // La placa ya existe: Evento de SALIDA / LIBERACIÓN automática
        int numCelda = celdas[posExistente].getNumeroCelda();
        celdas[posExistente].liberar();
        std::string resultado = "[SALIDA] Vehiculo [" + placa + "] salio de la celda #" + 
                                std::to_string(numCelda) + " a las " + hora + ". Celda liberada.";
        return resultado;
    }

    // La placa no existe: Evento de ENTRADA / ASIGNACIÓN
    int indiceAsignado = -1;

    // Verificar si la celda sugerida es válida y está libre (1-based index)
    int idxSugerido = celdaSugerida - 1;
    if (idxSugerido >= 0 && idxSugerido < capacidadTotal && !celdas[idxSugerido].estaOcupada()) {
        indiceAsignado = idxSugerido;
    } else {
        // Si la celda sugerida está ocupada o fuera de rango, buscar la primera celda libre disponible
        for (int i = 0; i < capacidadTotal; ++i) {
            if (!celdas[i].estaOcupada()) {
                indiceAsignado = i;
                break;
            }
        }
    }

    if (indiceAsignado == -1) {
        // No hay celdas libres disponibles
        return "[LLENO] Parqueadero al 100%. Vehiculo [" + placa + "] no pudo ingresar a las " + hora + ".";
    }

    // Ocupar la celda seleccionada
    celdas[indiceAsignado].ocupar(placa, hora);
    std::string resultado = "[ENTRADA] Vehiculo [" + placa + "] parqueado en celda #" + 
                            std::to_string(celdas[indiceAsignado].getNumeroCelda()) + " a las " + hora + ".";
    return resultado;
}

std::string Parqueadero::obtenerEstadoSerializado() const {
    std::lock_guard<std::mutex> lock(mutexAcceso);

    if (celdas == nullptr) {
        return "";
    }

    // Formato exportado: ID_CELDA,ESTADO(0/1),PLACA,HORA|...
    std::ostringstream ss;
    for (int i = 0; i < capacidadTotal; ++i) {
        int id = celdas[i].getNumeroCelda();
        int estado = celdas[i].estaOcupada() ? 1 : 0;
        std::string placa = "";
        std::string hora = "";

        if (estado == 1) {
            RegistroVehiculo* v = celdas[i].getVehiculo();
            if (v != nullptr) {
                placa = v->getPlaca();
                hora = v->getHoraIngreso();
            }
        }

        ss << id << "," << estado << "," << placa << "," << hora;
        if (i < capacidadTotal - 1) {
            ss << "|";
        }
    }
    return ss.str();
}

int Parqueadero::getCapacidadTotal() const {
    return capacidadTotal;
}

int Parqueadero::getCeldasOcupadas() const {
    std::lock_guard<std::mutex> lock(mutexAcceso);
    int ocupadas = 0;
    if (celdas != nullptr) {
        for (int i = 0; i < capacidadTotal; ++i) {
            if (celdas[i].estaOcupada()) {
                ocupadas++;
            }
        }
    }
    return ocupadas;
}

int Parqueadero::getCeldasDisponibles() const {
    return getCapacidadTotal() - getCeldasOcupadas();
}
