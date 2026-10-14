#ifndef REGISTROVEHICULO_H
#define REGISTROVEHICULO_H

#include <string>

class RegistroVehiculo {
private:
    std::string placa;
    std::string horaIngreso;

public:
    RegistroVehiculo();
    RegistroVehiculo(const std::string& pl, const std::string& hora);
    std::string getPlaca() const;
    std::string getHoraIngreso() const;
};

#endif // REGISTROVEHICULO_H