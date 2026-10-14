#ifndef GENERADORPLACAS_H
#define GENERADORPLACAS_H

#include <string>

class GeneradorPlacas {
private:
    std::string poolPlacas[15];
    int maxCeldas;

public:
    GeneradorPlacas(int maxC);
    std::string generarPlacaAleatoria();
    int elegirCelda();
    std::string obtenerHoraActual();
};

#endif // GENERADORPLACAS_H