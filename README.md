# Sistema de Gestión de Parqueadero (C++ & Python)

Proyecto desarrollado para el curso de Estructura de Datos. Implementa un núcleo en C++ con manejo dinámico de memoria, exportado como librería dinámica (`.dll`/`.so`) para su ejecución e interfaz en Python, e integrando un servidor de sockets TCP/IP para administración remota entre equipos independientes.

## Descripción del Proyecto
El sistema gestiona en tiempo real la ocupación, tarifas y facturación de un parqueadero comercial con capacidades diferenciadas para automóviles y motocicletas. 

La arquitectura desacopla la capa de presentación de la capa lógica:
1. **Núcleo de C++:** Controla el estado de las celdas, el cálculo matemático del tiempo y costos de facturación, y ejecuta en segundo plano un hilo de red con sockets.
2. **Capa Externa / Sockets:** Permite que un cliente conectado desde otra computadora a través de la red local (LAN) consulte la disponibilidad o solicite el ingreso y salida de vehículos.
3. **Interfaz de Control (Python):** Se comunica directamente con el núcleo de bajo nivel mediante vinculación dinámica en memoria (`ctypes`), consumiendo las funciones nativas exportadas por C++.

## Diagrama de Clases UML
<img width="867" height="940" alt="Imágen Diagrama UML" src="https://github.com/user-attachments/assets/7c31cc5f-3099-485c-b1fa-1f366be2551a" />



## Funcionalidades Principales
- **Memoria Dinámica en Heap:** Creación y dimensionamiento en tiempo de ejecución de las celdas de parqueo (Celda*) y administración de vehículos por punteros (Vehiculo*).
- **Control de Tiempo y Facturación:** Captura de marcas de tiempo precisas con la librería `<ctime>` (`time_t`), cálculo de fracciones/horas completas transcurridas y aplicación de tarifas diferenciadas por tipo de vehículo.
- **Librería Compartida / Dinámica:** Exportación de interfaces con compatibilidad de enlace en C (`extern "C"`), permitiendo llamadas nativas desde scripts de Python.
- **Servicio Concurrente por Sockets:** Socket servidor TCP/IP multihilo (std::thread) ejecutándose sobre puertos de red dedicados para recibir comandos desde PCs externas sin congelar la aplicación principal.

## Distribución de Actividades y Commits
- **[Teojz]:** Implementación de la arquitectura del servidor de sockets de red (ServidorRed), administración de conexiones TCP/IP concurrentes e integración del wrapper de enlace dinámico (extern "C").
- **[Carlos10marin]:** Implementación de clases de entidad (Vehiculo, Celda), gestión de memoria dinámica (Celda* celdasCarro, Celda* celdasMoto) y control de ocupación/disponibilidad.
- **[Carlos10marin]:** Lógica de facturación y medición de tiempo en `Parqueadero` (time_t), cálculo de cobro por tarifas y liberación de celdas.
- **[Teojz]:** Script de carga e interfaz en Python (`ctypes`), pruebas de integración cliente-servidor externa y documentación del repositorio.

## Protocolo de Comunicación por Socket (Trama de Red)
El servidor socket expone un protocolo de texto plano delimitado por comas (`CSV`) para procesar operaciones remotas enviadas por clientes externos:

### 1. Registro de Entrada
- **Petición:** `ENTRADA;<PLACA>;<TIPO>` (Ejemplo: `ENTRADA;ABC123;Carro`)
- **Respuesta Servidor:** `OK;CELDA_ASIGNADA:<ID>` o `ERROR;PARQUEADERO_LLENO`

### 2. Registro de Salida y Cobro
- **Petición:** `SALIDA;<PLACA>` (Ejemplo: `SALIDA;ABC123`)
- **Respuesta Servidor:** `OK;TIEMPO_HORAS:<H>;TOTAL_PAGAR:<VALOR>` o `ERROR;VEHICULO_NO_ENCONTRADO`

### 3. Consulta de Disponibilidad
- **Petición:** `ESTADO`
- **Respuesta Servidor:** `AUTOS_DISP:<N>/<TOTAL>;MOTOS_DISP:<M>/<TOTAL>`
