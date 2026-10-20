import argparse               # Opciones de linea de comandos (--celdas, --puerto, --dll)
import ctypes                 # Modulo nativo de Python para invocar funciones de la libreria C++ (.dll / .so)
import os                     # Modulo para resolucion de rutas del sistema operativo
import sys                    # Para detectar el sistema operativo
import tkinter as tk          # Libreria nativa de interfaz grafica (cumple el 15% de la rubrica)
from tkinter import messagebox

NUM_CELDAS = 12               # Limite total de celdas configuradas en el sistema
PUERTO_SOCKET = 8080          # Puerto TCP que escuchara el servidor C++ en segundo plano
REFRESCO_MS = 500             # Cada cuantos milisegundos se actualiza la vista

COLOR_FONDO = "#1e272e"
COLOR_LIBRE = "#05c46b"       # Verde = disponible
COLOR_OCUPADA = "#ff3f34"     # Rojo = ocupada


def ruta_libreria_por_defecto():
    """Ruta de la libreria dinamica en la raiz del proyecto (../ respecto a gui/).
    Windows -> parqueadero.dll ; Linux -> libparqueadero.so"""
    base = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
    nombre = "parqueadero.dll" if sys.platform.startswith("win") else "libparqueadero.so"
    return os.path.join(base, nombre)


def cargar_libreria(ruta):
    """Carga la libreria y declara tipos de argumentos/retorno (evita errores de memoria con ctypes)."""
    if sys.platform.startswith("win") and hasattr(os, "add_dll_directory"):
        os.add_dll_directory(os.path.dirname(ruta))   # Python 3.8+ ya no busca DLLs en el PATH
    lib = ctypes.CDLL(ruta)

    lib.iniciar_receptor_parqueadero.argtypes = [ctypes.c_int, ctypes.c_int]   # (celdas, puerto)
    lib.iniciar_receptor_parqueadero.restype = ctypes.c_int                    # 0 = OK
    lib.detener_receptor_parqueadero.argtypes = []
    lib.detener_receptor_parqueadero.restype = None
    lib.obtener_estado_celdas.argtypes = []
    lib.obtener_estado_celdas.restype = ctypes.c_char_p                        # char* de C++
    lib.obtener_bitacora.argtypes = []
    lib.obtener_bitacora.restype = ctypes.c_char_p
    lib.obtener_ultimo_error.argtypes = []
    lib.obtener_ultimo_error.restype = ctypes.c_char_p
    return lib


def parsear_estado(cadena):
    """Convierte 'ID,ESTADO,PLACA,HORA|ID,ESTADO,PLACA,HORA|...' en una lista de diccionarios.
    Ignora registros mal formados en lugar de romper la interfaz."""
    celdas = []
    for registro in cadena.split("|"):
        if not registro:
            continue
        partes = registro.split(",")
        if len(partes) != 4:
            continue
        num, estado, placa, hora = partes
        celdas.append({"id": num, "ocupada": estado == "1", "placa": placa, "hora": hora})
    return celdas


class VisualizadorParqueaderoGUI:
    def __init__(self, root, lib, num_celdas=NUM_CELDAS, puerto=PUERTO_SOCKET):
        self.root = root      # Guarda la referencia a la ventana principal de Tkinter
        self.lib = lib        # Guarda la instancia cargada de la libreria en memoria
        self.num_celdas = num_celdas
        self.puerto = puerto
        self._ultima_bitacora = None
        self._after_id = None
        self._cerrando = False

        self.root.title("Monitor de Parqueadero - C++ DLL & Python")  # Titulo superior
        self.root.geometry("860x700")                                 # Dimensiones de la ventana
        self.root.configure(bg=COLOR_FONDO)                           # Color de fondo oscuro
        self.root.protocol("WM_DELETE_WINDOW", self.cerrar)           # Cierra el servidor C++ al salir

        # Titulo principal del dashboard
        self.titulo = tk.Label(
            root, text="SISTEMA DISTRIBUIDO DE PARQUEADERO (SOCKET -> DLL -> GUI)",
            font=("Segoe UI", 13, "bold"), fg="#ffffff", bg=COLOR_FONDO, pady=10
        )
        self.titulo.pack()    # Coloca el titulo arriba

        # Texto con resumen estadistico en tiempo real
        self.lbl_resumen = tk.Label(
            root, text="Inicializando conexion con socket C++...",
            font=("Segoe UI", 11), fg="#0be881", bg=COLOR_FONDO, pady=5
        )
        self.lbl_resumen.pack()  # Coloca el resumen debajo del titulo

        # Contenedor tipo cuadricula (Grid) para ubicar las tarjetas de celdas
        self.grid_frame = tk.Frame(root, bg=COLOR_FONDO, padx=15, pady=5)
        self.grid_frame.pack(fill="x")

        self.tarjetas = []    # Lista para almacenar las etiquetas visuales de cada celda
        for i in range(self.num_celdas):
            fila = i // 4     # 4 celdas por fila
            col = i % 4       # Columnas 0 a 3
            lbl = tk.Label(   # Tarjeta visual con color de fondo y bordes
                self.grid_frame,
                text=f"CELDA {i + 1}\nDISPONIBLE\n---\nLibre",
                font=("Consolas", 10, "bold"),
                width=18, height=4,
                bg=COLOR_LIBRE, fg="white",
                relief="ridge", bd=3
            )
            lbl.grid(row=fila, column=col, padx=8, pady=6, sticky="nsew")
            self.tarjetas.append(lbl)
        for c in range(4):
            self.grid_frame.columnconfigure(c, weight=1)

        # Registro de actividad del socket (requerido por el enunciado)
        tk.Label(root, text="REGISTRO DE ACTIVIDAD DEL SOCKET", font=("Segoe UI", 10, "bold"),
                 fg="#ffffff", bg=COLOR_FONDO, pady=4).pack()
        marco_log = tk.Frame(root, bg=COLOR_FONDO, padx=15, pady=5)
        marco_log.pack(expand=True, fill="both")
        self.txt_log = tk.Text(marco_log, height=8, bg="#0f1519", fg="#d2dae2",
                               font=("Consolas", 9), state="disabled", wrap="none")
        scroll = tk.Scrollbar(marco_log, command=self.txt_log.yview)
        self.txt_log.configure(yscrollcommand=scroll.set)
        scroll.pack(side="right", fill="y")
        self.txt_log.pack(side="left", expand=True, fill="both")

        self.actualizar_vista()  # Inicia el ciclo de refresco continuo

    # ------------------------------------------------------------------ actualizacion
    def actualizar_vista(self):
        if self._cerrando:
            return
        try:
            self._actualizar_celdas()
            self._actualizar_bitacora()
        except Exception as e:  # Un error puntual no debe detener el refresco de la GUI
            self.lbl_resumen.config(text=f"Error al leer la libreria: {e}")

        # Programa la funcion para volver a ejecutarse
        self._after_id = self.root.after(REFRESCO_MS, self.actualizar_vista)

    def _actualizar_celdas(self):
        raw = self.lib.obtener_estado_celdas()      # bytes (char* de C++) o None
        cadena = raw.decode("utf-8", errors="ignore") if raw else ""
        if not cadena:
            return
        registros = parsear_estado(cadena)
        ocupadas = 0
        for idx, reg in enumerate(registros):
            if idx >= len(self.tarjetas):           # Evita desbordar las tarjetas visuales
                break
            if reg["ocupada"]:
                ocupadas += 1
                self.tarjetas[idx].config(
                    text=f"CELDA {reg['id']}\nOCUPADA\nPlaca: {reg['placa']}\nHora: {reg['hora']}",
                    bg=COLOR_OCUPADA)
            else:
                self.tarjetas[idx].config(
                    text=f"CELDA {reg['id']}\nDISPONIBLE\n---\nLibre",
                    bg=COLOR_LIBRE)

        libres = self.num_celdas - ocupadas
        self.lbl_resumen.config(
            text=f"Total: {self.num_celdas}  |  Disponibles (Verde): {libres}  |  Ocupadas (Rojo): {ocupadas}"
                 f"  |  Puerto: {self.puerto}")

    def _actualizar_bitacora(self):
        raw = self.lib.obtener_bitacora()
        texto = raw.decode("utf-8", errors="ignore") if raw else ""
        if texto == self._ultima_bitacora:          # Solo repinta si hubo eventos nuevos
            return
        self._ultima_bitacora = texto
        self.txt_log.config(state="normal")
        self.txt_log.delete("1.0", "end")
        self.txt_log.insert("end", texto)
        self.txt_log.see("end")                     # Baja al ultimo evento
        self.txt_log.config(state="disabled")

    # ------------------------------------------------------------------ cierre
    def cerrar(self):
        """Detiene el servidor C++ (join del hilo) y cierra la ventana."""
        self._cerrando = True
        if self._after_id is not None:
            try:
                self.root.after_cancel(self._after_id)
            except Exception:
                pass
        try:
            self.lib.detener_receptor_parqueadero()
        finally:
            self.root.destroy()


def main():
    ap = argparse.ArgumentParser(description="Monitor grafico del parqueadero (C++ DLL + Python)")
    ap.add_argument("--celdas", type=int, default=NUM_CELDAS, help="numero de celdas (por defecto 12)")
    ap.add_argument("--puerto", type=int, default=PUERTO_SOCKET, help="puerto TCP (por defecto 8080)")
    ap.add_argument("--dll", default=ruta_libreria_por_defecto(), help="ruta de la libreria dinamica")
    args = ap.parse_args()

    if not os.path.exists(args.dll):  # Valida si la libreria ya fue compilada
        print(f"[Error] No se encontro la libreria dinamica en: {args.dll}")
        print("Compila primero con compilar.bat (Windows) o 'make' (Linux) antes de lanzar el visualizador.")
        return 1

    try:
        lib = cargar_libreria(args.dll)
    except OSError as e:
        print(f"[Error] No se pudo cargar la libreria: {e}")
        print("Revisa que Python (32/64 bits) coincida con la arquitectura con la que se compilo la DLL.")
        return 1

    root = tk.Tk()

    # Lanza el modelo en C++ y arranca el socket de red en segundo plano
    rc = lib.iniciar_receptor_parqueadero(args.celdas, args.puerto)
    if rc != 0:
        detalle = (lib.obtener_ultimo_error() or b"").decode("utf-8", errors="ignore")
        messagebox.showerror("No se pudo iniciar el servidor",
                             f"Codigo {rc}\n{detalle}\n\nCierra otros monitores abiertos o usa --puerto.")
        root.destroy()
        return 1

    VisualizadorParqueaderoGUI(root, lib, args.celdas, args.puerto)
    root.mainloop()  # Mantiene la aplicacion abierta escuchando eventos
    return 0


if __name__ == "__main__":
    sys.exit(main())
