# Sistema de Parqueadero con Generación Automática, Sockets C++ y Librería Dinámica en Python

Entrega del curso **Estructura de Datos**[cite: 1, 3]. Proyecto cliente-servidor desarrollado en **C++** y **Python** utilizando memoria dinámica, comunicación por sockets TCP/IP, enlaces dinámicos (`.dll`/`.so`) e interfaz gráfica de usuario (GUI)[cite: 1, 3].

## Descripción del Proyecto
El sistema modela y monitorea en tiempo real el flujo de entrada y salida de vehículos en un parqueadero mediante dos componentes desacoplados:

1. **Módulo Generador y Transmisor (Proceso Cliente C++):** 
   Genera placas vehiculares de forma automática cada 2 a 5 segundos. En cada emisión, asigna la marca de tiempo actual (`HH:MM:SS`) y la celda proyectada, transmitiendo la trama a través de un socket de red TCP/IP hacia el servidor[cite: 1].
2. **Módulo Receptor y Librería Dinámica (Proceso Servidor C++ / DLL):** 
   Escucha las conexiones entrantes por socket en segundo plano y administra un arreglo en memoria dinámica de celdas (`Celda*`)[cite: 1, 3]. 
   - **Regla de Ocupación y Liberación:** Si ingresa una placa nueva, ocupa la celda asignada registrando la hora de entrada[cite: 1]. Si el sistema detecta que una placa enviada ya se encuentra registrada en el parqueadero (placa repetida), procede automáticamente a liberar dicha celda de parqueo[cite: 1].
3. **Módulo Visualizador Gráfico (Python GUI + `ctypes`):** 
   Carga la librería dinámica compilada de C++ y despliega una interfaz gráfica en tiempo real donde se visualizan las celdas disponibles (verde) y ocupadas (rojo) junto con la placa, hora de ingreso y registro de actividad del socket[cite: 1].

## Arquitectura del Sistema y Diagrama UML
<img width="867" height="940" alt="Imágen Diagrama UML" src="https://github.com/user-attachments/assets/718498e6-d725-4335-9c66-dd608d1f30f4" />


## Cumplimiento de Criterios y Funcionalidades
- **Memoria Dinámica y Punteros:** Administración del arreglo `Celda* celdas` y asociación `RegistroVehiculo*` en el heap con constructores y destructores explícitos (`new` / `delete[]`)[cite: 2, 3].
- **Modularidad Estricta:** Organización bajo el principio de una clase por archivo (`.h` en `include/` y `.cpp` en `src/`)[cite: 1, 3].
- **Comunicación Interprocesos (IPC):** Transmisión cliente-servidor 100% en C++ mediante Sockets TCP/IP[cite: 1].
- **Interfaz Gráfica (GUI):** Tablero visual interactivo en Python (`Tkinter`) acoplado a la librería dinámica mediante `ctypes`[cite: 1].

## Distribución de Actividades, Ramas y Commits
- **[Carlos10marin] (Rama `feature/modelo-parqueadero`):** Clases `RegistroVehiculo`, `Celda` y `Parqueadero` con gestión de memoria dinámica y lógica de liberación por placa repetida[cite: 1, 3].
- **[Carlos10marin] (Rama `feature/generador`):** Clase `GeneradorPlacas` con temporizador aleatorio de 2 a 5 segundos y captura de hora del sistema[cite: 1, 3].
- **[Teojz] (Rama `feature/sockets-red`):** Clases `ClienteSocket` y `ServidorSocket` en C++ para transmisión y recepción de tramas TCP/IP[cite: 1, 3].
- **[Teojz] (Rama `feature/visualizador-dll`):** Puente de librería dinámica (`wrapper_dll.cpp`), ejecutable cliente (`main_generador.cpp`), interfaz gráfica en Python (`visualizador_gui.py`) y documentación del repositorio[cite: 1, 3].

## Formato de Trama de Red (Socket C++)
- **Trama enviada cada 2-5 seg (Debe ser una estructura) :** 
Ejemplo:
struct TramaVehiculo {
    char placa[7];       
    int celda;      
    char hora[8];        
};
- **Estado exportado a Python:** ID_CELDA,ESTADO(0/1),PLACA,HORA|...
