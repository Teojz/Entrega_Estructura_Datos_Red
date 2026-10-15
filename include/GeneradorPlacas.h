#ifndef GENERADOR_PLACAS_H
#define GENERADOR_PLACAS_H

#include <string>
#include <random>

/**
 * @brief Genera placas aleatorias tomadas de un pool definido de 15 placas,
 * proyecta celdas y obtiene la hora del sistema en formato HH:MM:SS.
 */
class GeneradorPlacas {
private:
    std::string poolPlacas[15];
    int maxCeldas;
    mutable std::mt19937 rng;

public:
    // Constructor según diagrama UML
    explicit GeneradorPlacas(int maxC);

    // Métodos según diagrama UML
    std::string generarPlacaAleatoria();
    int elegirCelda();
    std::string obtenerHoraActual();
};

#endif // GENERADOR_PLACAS_H
