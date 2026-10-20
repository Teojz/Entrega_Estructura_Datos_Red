#include "../include/GeneradorPlacas.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <mutex>

GeneradorPlacas::GeneradorPlacas(int maxC) : maxCeldas(maxC > 0 ? maxC : 10) {
    // Inicializar pool de 15 placas predefinidas
    poolPlacas[0]  = "KLM-452";
    poolPlacas[1]  = "ABC-123";
    poolPlacas[2]  = "XYZ-789";
    poolPlacas[3]  = "FGH-101";
    poolPlacas[4]  = "QWE-999";
    poolPlacas[5]  = "TYU-456";
    poolPlacas[6]  = "BNM-321";
    poolPlacas[7]  = "ASD-654";
    poolPlacas[8]  = "JKL-888";
    poolPlacas[9]  = "RST-777";
    poolPlacas[10] = "UVW-222";
    poolPlacas[11] = "OPQ-333";
    poolPlacas[12] = "HJK-444";
    poolPlacas[13] = "ZXC-555";
    poolPlacas[14] = "VBN-666";

    // Inicializar generador Mersenne Twister con entropía aleatoria
    std::random_device rd;
    rng = std::mt19937(rd());
}

std::string GeneradorPlacas::generarPlacaAleatoria() {
    std::uniform_int_distribution<int> dist(0, 14);
    return poolPlacas[dist(rng)];
}

int GeneradorPlacas::elegirCelda() {
    std::uniform_int_distribution<int> dist(1, maxCeldas);
    return dist(rng);
}

std::string GeneradorPlacas::obtenerHoraActual() {
    auto ahora = std::chrono::system_clock::now();
    std::time_t tiempo = std::chrono::system_clock::to_time_t(ahora);

    // localtime_s / localtime_r tienen firmas distintas segun compilador (MinGW antiguo no trae localtime_s).
    // std::localtime es estandar en todos; se protege con mutex porque usa un buffer interno compartido.
    static std::mutex mutexTiempo;
    std::tm tmBuffer;
    {
        std::lock_guard<std::mutex> lock(mutexTiempo);
        tmBuffer = *std::localtime(&tiempo);
    }

    std::ostringstream ss;
    ss << std::setfill('0')
       << std::setw(2) << tmBuffer.tm_hour << ":"
       << std::setw(2) << tmBuffer.tm_min << ":"
       << std::setw(2) << tmBuffer.tm_sec;
    return ss.str();
}
