#include "../include/Celda.h"

// Constructor por defecto
Celda::Celda() : numeroCelda(0), ocupada(false), vehiculo(nullptr) {}

// Destructor explícito
Celda::~Celda() {
    liberar();
}

// Constructor de copia (Copia profunda para evitar doble liberación de puntero)
Celda::Celda(const Celda& otra) : numeroCelda(otra.numeroCelda), ocupada(otra.ocupada), vehiculo(nullptr) {
    if (otra.ocupada && otra.vehiculo != nullptr) {
        vehiculo = new RegistroVehiculo(otra.vehiculo->getPlaca(), otra.vehiculo->getHoraIngreso());
    }
}

// Operador de asignación por copia
Celda& Celda::operator=(const Celda& otra) {
    if (this != &otra) {
        liberar();
        numeroCelda = otra.numeroCelda;
        ocupada = otra.ocupada;
        if (otra.ocupada && otra.vehiculo != nullptr) {
            vehiculo = new RegistroVehiculo(otra.vehiculo->getPlaca(), otra.vehiculo->getHoraIngreso());
        }
    }
    return *this;
}

// Constructor de movimiento
Celda::Celda(Celda&& otra) noexcept
    : numeroCelda(otra.numeroCelda), ocupada(otra.ocupada), vehiculo(otra.vehiculo) {
    otra.vehiculo = nullptr;
    otra.ocupada = false;
    otra.numeroCelda = 0;
}

// Operador de asignación por movimiento
Celda& Celda::operator=(Celda&& otra) noexcept {
    if (this != &otra) {
        liberar();
        numeroCelda = otra.numeroCelda;
        ocupada = otra.ocupada;
        vehiculo = otra.vehiculo;
        otra.vehiculo = nullptr;
        otra.ocupada = false;
        otra.numeroCelda = 0;
    }
    return *this;
}

void Celda::inicializar(int num) {
    numeroCelda = num;
    ocupada = false;
    if (vehiculo != nullptr) {
        delete vehiculo;
        vehiculo = nullptr;
    }
}

bool Celda::estaOcupada() const {
    return ocupada;
}

void Celda::ocupar(std::string placa, std::string hora) {
    // Si ya había un vehículo previamente asignado, liberamos memoria antes de asignar uno nuevo
    if (vehiculo != nullptr) {
        delete vehiculo;
        vehiculo = nullptr;
    }
    vehiculo = new RegistroVehiculo(placa, hora);
    ocupada = true;
}

void Celda::liberar() {
    if (vehiculo != nullptr) {
        delete vehiculo;
        vehiculo = nullptr;
    }
    ocupada = false;
}

int Celda::getNumeroCelda() const {
    return numeroCelda;
}

RegistroVehiculo* Celda::getVehiculo() const {
    return vehiculo;
}
