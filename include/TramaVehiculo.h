#ifndef TRAMA_VEHICULO_H
#define TRAMA_VEHICULO_H

#include <cstddef>
#include <string>

/**
 * @brief Trama de red que viaja del cliente al servidor (exigida por el enunciado como struct).
 *
 * Se usa #pragma pack(1) para que el tamano sea identico en cualquier compilador/plataforma:
 *   placa[7] (7 bytes) + celda (4 bytes) + hora[8] (8 bytes) = 19 bytes.
 *
 * Notas importantes:
 *  - "KLM-452" ocupa los 7 bytes completos y "HH:MM:SS" los 8 bytes completos: NO hay espacio
 *    para el terminador '\0'. Por eso se usan las funciones auxiliares de abajo en vez de
 *    tratar los arreglos como cadenas C.
 *  - "celda" viaja en orden de red (big-endian, htonl/ntohl) para ser independiente de la
 *    arquitectura. En memoria local (host) se maneja como un int normal.
 */
#pragma pack(push, 1)
struct TramaVehiculo {
    char placa[7];
    int  celda;
    char hora[8];
};
#pragma pack(pop)

static_assert(sizeof(int) == 4, "La trama asume int de 4 bytes");
static_assert(sizeof(TramaVehiculo) == 19, "TramaVehiculo debe medir exactamente 19 bytes");

const std::size_t TAM_TRAMA = sizeof(TramaVehiculo);

// Construye una trama copiando (y recortando si hace falta) placa y hora; rellena con '\0'.
inline TramaVehiculo crearTrama(const std::string& placa, int celda, const std::string& hora) {
    TramaVehiculo t;
    for (std::size_t i = 0; i < sizeof(t.placa); ++i) t.placa[i] = (i < placa.size()) ? placa[i] : '\0';
    for (std::size_t i = 0; i < sizeof(t.hora); ++i)  t.hora[i]  = (i < hora.size())  ? hora[i]  : '\0';
    t.celda = celda;
    return t;
}

// Extrae la placa como std::string (se detiene en '\0' o a los 7 bytes).
inline std::string placaDeTrama(const TramaVehiculo& t) {
    std::size_t n = 0;
    while (n < sizeof(t.placa) && t.placa[n] != '\0') ++n;
    return std::string(t.placa, n);
}

// Extrae la hora como std::string (se detiene en '\0' o a los 8 bytes).
inline std::string horaDeTrama(const TramaVehiculo& t) {
    std::size_t n = 0;
    while (n < sizeof(t.hora) && t.hora[n] != '\0') ++n;
    return std::string(t.hora, n);
}

// Validacion defensiva en el servidor: nunca se confia en los bytes recibidos por red.
// Placa: 3 a 7 caracteres [A-Za-z0-9-]. Hora: formato HH:MM:SS valido.
inline bool tramaValida(const TramaVehiculo& t) {
    std::string p = placaDeTrama(t);
    if (p.size() < 3) return false;
    for (std::size_t i = 0; i < p.size(); ++i) {
        char c = p[i];
        bool ok = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-';
        if (!ok) return false;
    }
    if (t.hora[2] != ':' || t.hora[5] != ':') return false;
    const int digitos[6] = {0, 1, 3, 4, 6, 7};
    for (int k = 0; k < 6; ++k) {
        char c = t.hora[digitos[k]];
        if (c < '0' || c > '9') return false;
    }
    int hh = (t.hora[0] - '0') * 10 + (t.hora[1] - '0');
    int mm = (t.hora[3] - '0') * 10 + (t.hora[4] - '0');
    int ss = (t.hora[6] - '0') * 10 + (t.hora[7] - '0');
    return hh < 24 && mm < 60 && ss < 60;
}

#endif // TRAMA_VEHICULO_H
