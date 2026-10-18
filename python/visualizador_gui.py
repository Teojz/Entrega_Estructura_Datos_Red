import ctypes                 # Modulo nativo de Python para invocar funciones de la libreria C++ (.dll)
import os                     # Modulo para resolucion de rutas del sistema operativo
import tkinter as tk          # Libreria nativa de interfaz grafica (cumple el 15% de la rubrica)

# Ruta absoluta hacia el archivo compilado parqueadero.dll en la raiz del proyecto
RUTA_DLL = os.path.abspath(os.path.join(os.path.dirname(__file__), "../parqueadero.dll"))
NUM_CELDAS = 12               # Limite total de celdas configuradas en el sistema
PUERTO_SOCKET = 8080          # Puerto TCP que escuchara el servidor C++ en segundo plano

class VisualizadorParqueaderoGUI:
    def __init__(self, root, lib):
        self.root = root      # Guarda la referencia a la ventana principal de Tkinter
        self.lib = lib        # Guarda la instancia cargada de la DLL en memoria
        self.root.title("Monitor de Parqueadero - C++ DLL & Python") # Titulo superior
        self.root.geometry("840x540")                                # Dimensiones de la ventana
        self.root.configure(bg="#1e272e")                            # Color de fondo oscuro

        # Titulo principal del dashboard
        self.titulo = tk.Label(
            root, text="SISTEMA DISTRIBUIDO DE PARQUEADERO (SOCKET -> DLL -> GUI)",
            font=("Segoe UI", 13, "bold"), fg="#ffffff", bg="#1e272e", pady=10
        )
        self.titulo.pack()    # Coloca el titulo arriba

        # Texto con resumen estadistico en tiempo real
        self.lbl_resumen = tk.Label(
            root, text="Inicializando conexion con socket C++...",
            font=("Segoe UI", 11), fg="#0be881", bg="#1e272e", pady=5
        )
        self.lbl_resumen.pack() # Coloca el resumen debajo del titulo

        # Contenedor tipo cuadricula (Grid) para ubicar las 12 tarjetas de celdas
        self.grid_frame = tk.Frame(root, bg="#1e272e", padx=15, pady=15)
        self.grid_frame.pack(expand=True, fill="both") # Se expande en la ventana

        self.tarjetas = []    # Lista para almacenar las etiquetas visuales de cada celda
        for i in range(NUM_CELDAS):
            fila = i // 4     # 4 celdas por fila (filas 0, 1, 2)
            col = i % 4       # Columnas 0 a 3
            lbl = tk.Label(   # Tarjeta visual con color de fondo y bordes
                self.grid_frame,
                text=f"CELDA {i + 1}\nDISPONIBLE\n---\nLibre",
                font=("Consolas", 10, "bold"),
                width=18, height=5,
                bg="#05c46b", fg="white", # Verde = disponible
                relief="ridge", bd=3
            )
            lbl.grid(row=fila, column=col, padx=8, pady=8, sticky="nsew") # Ubica en la malla
            self.tarjetas.append(lbl) # Guarda la tarjeta en el arreglo

        self.actualizar_vista() # Inicia el ciclo de refresco continuo

    def actualizar_vista(self):
        # Llama a la funcion C++ exportada en la DLL para leer el estado del parqueadero
        raw_ptr = self.lib.obtener_estado_celdas()
        if raw_ptr:           # Si el puntero retornado no es nulo
            cadena = raw_ptr.decode("utf-8", errors="ignore") # Convierte char* de C++ a string de Python
            if cadena:        # Si hay texto retornado
                registros = [r for r in cadena.split("|") if r] # Separa cada celda delimitada por '|'
                ocupadas = 0  # Contador de celdas ocupadas

                for idx, reg in enumerate(registros):
                    if idx >= len(self.tarjetas): # Evita desbordar las tarjetas visuales
                        break
                    partes = reg.split(",")       # Formato esperado: ID,ESTADO,PLACA,HORA
                    if len(partes) == 4:
                        num_celda, estado, placa, hora = partes
                        if estado == "1":         # 1 = Celda ocupada
                            ocupadas += 1
                            self.tarjetas[idx].config(
                                text=f"CELDA {num_celda}\nOCUPADA\nPlaca: {placa}\nHora: {hora}",
                                bg="#ff3f34"      # Rojo = ocupada
                            )
                        else:                     # 0 = Celda libre
                            self.tarjetas[idx].config(
                                text=f"CELDA {num_celda}\nDISPONIBLE\n---\nLibre",
                                bg="#05c46b"      # Verde = disponible
                            )

                libres = NUM_CELDAS - ocupadas    # Calcula celdas desocupadas
                self.lbl_resumen.config(          # Actualiza el resumen de texto superior
                    text=f"Total: {NUM_CELDAS}  |  Disponibles (Verde): {libres}  |  Ocupadas (Rojo): {ocupadas}"
                )

        # Programa la funcion para volver a ejecutarse cada 1000 ms (1 segundo)
        self.root.after(1000, self.actualizar_vista)

def main():
    if not os.path.exists(RUTA_DLL): # Valida si la libreria ya fue compilada
        print(f"[Error] No se encontro la libreria dinamica en: {RUTA_DLL}")
        print("Compila primero parqueadero.dll con g++ antes de lanzar el visualizador.")
        return

    # Carga la libreria dinamica (.dll) en la memoria del interprete de Python
    lib = ctypes.CDLL(RUTA_DLL)

    # Define los tipos de datos de entrada y salida para evitar fallos de memoria
    lib.iniciar_receptor_parqueadero.argtypes = [ctypes.c_int, ctypes.c_int] # Recibe dos enteros
    lib.obtener_estado_celdas.restype = ctypes.c_char_p                     # Retorna una cadena (char*)

    # Lanza el modelo en C++ y arranca el socket de red en segundo plano (puerto 8080)
    lib.iniciar_receptor_parqueadero(NUM_CELDAS, PUERTO_SOCKET)

    # Crea y lanza la ventana grafica
    root = tk.Tk()
    app = VisualizadorParqueaderoGUI(root, lib)
    root.mainloop() # Mantiene la aplicacion abierta escuchando eventos

if __name__ == "__main__":
    main()