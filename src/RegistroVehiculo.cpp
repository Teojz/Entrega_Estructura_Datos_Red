#include "../include/RegistroVehiculo.h"

RegistroVehiculo::RegistroVehiculo() : placa(""), horaIngreso("") {}

RegistroVehiculo::RegistroVehiculo(std::string pl, std::string hora)
    : placa(pl), horaIngreso(hora) {}

std::string RegistroVehiculo::getPlaca() const {
    return placa;
}

std::string RegistroVehiculo::getHoraIngreso() const {
    return horaIngreso;
}
