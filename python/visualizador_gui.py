import ctypes                 # Carga la libreria dinamica (.dll)
import os                     # Rutas del sistema
import tkinter as tk          # Interfaz grafica

RUTA_DLL = os.path.abspath(os.path.join(os.path.dirname(__file__), "../parqueadero.dll"))
NUM_CELDAS = 12
PUERTO_SOCKET = 8080

class VisualizadorParqueaderoGUI:
    def __init__(self, root, lib):
        self.root = root
        self.lib = lib
        self.root.title("Monitor de Parqueadero - C++ DLL & Python")
        self.root.geometry("820x520")
        self.root.configure(bg="#1e272e")

        self.titulo = tk.Label(
            root, text="SISTEMA DE PARQUEADERO (SOCKET C++ -> DLL -> PYTHON)",
            font=("Segoe UI", 13, "bold"), fg="#ffffff", bg="#1e272e", pady=10
        )
        self.titulo.pack()

        self.lbl_resumen = tk.Label(
            root, text="Inicializando celdas y servidor socket...",
            font=("Segoe UI", 11), fg="#0be881", bg="#1e272e", pady=5
        )
        self.lbl_resumen.pack()

        self.grid_frame = tk.Frame(root, bg="#1e272e", padx=15, pady=15)
        self.grid_frame.pack(expand=True, fill="both")

        self.tarjetas = []
        for i in range(NUM_CELDAS):
            fila = i // 4
            col = i % 4
            lbl = tk.Label(
                self.grid_frame,
                text=f"CELDA {i + 1}\nDISPONIBLE\n---\nLibre",
                font=("Consolas", 10, "bold"),
                width=18, height=5,
                bg="#05c46b", fg="white", # Verde = disponible
                relief="ridge", bd=3
            )
            lbl.grid(row=fila, column=col, padx=8, pady=8, sticky="nsew")
            self.tarjetas.append(lbl)

        self.actualizar_vista()

    def actualizar_vista(self):
        raw_ptr = self.lib.obtener_estado_celdas() # Consulta a la DLL de C++
        if raw_ptr:
            cadena = raw_ptr.decode("utf-8", errors="ignore")
            if cadena:
                registros = [r for r in cadena.split("|") if r]
                ocupadas = 0

                for idx, reg in enumerate(registros):
                    if idx >= len(self.tarjetas):
                        break
                    partes = reg.split(",")
                    if len(partes) == 4:
                        num_celda, estado, placa, hora = partes
                        if estado == "1": # 1 = ocupada
                            ocupadas += 1
                            self.tarjetas[idx].config(
                                text=f"CELDA {num_celda}\nOCUPADA\nPlaca: {placa}\nHora: {hora}",
                                bg="#ff3f34" # Rojo = ocupada
                            )
                        else:
                            self.tarjetas[idx].config(
                                text=f"CELDA {num_celda}\nDISPONIBLE\n---\nLibre",
                                bg="#05c46b"
                            )

                libres = NUM_CELDAS - ocupadas
                self.lbl_resumen.config(
                    text=f"Total: {NUM_CELDAS}  |  Disponibles: {libres}  |  Ocupadas: {ocupadas}"
                )

        self.root.after(1000, self.actualizar_vista) # Refresca cada 1 seg

def main():
    if not os.path.exists(RUTA_DLL):
        print(f"[Error] No se encontro la libreria en: {RUTA_DLL}")
        print("Compila primero parqueadero.dll antes de abrir el visualizador.")
        return

    lib = ctypes.CDLL(RUTA_DLL)
    lib.iniciar_receptor_parqueadero.argtypes = [ctypes.c_int, ctypes.c_int]
    lib.obtener_estado_celdas.restype = ctypes.c_char_p

    lib.iniciar_receptor_parqueadero(NUM_CELDAS, PUERTO_SOCKET)

    root = tk.Tk()
    app = VisualizadorParqueaderoGUI(root, lib)
    root.mainloop()

if __name__ == "__main__":
    main()