import sys
import pandas as pd
import matplotlib.pyplot as plt
import os

def main():
    if len(sys.argv) < 2:
        print("Uso: python grafico_rendimiento.py <archivo_csv>")
        sys.exit(1)

    csv_file = sys.argv[1]
    
    # Manejar errores
    try:
        df = pd.read_csv(csv_file)
    except Exception as e:
        print(f"Error leyendo CSV: {e}")
        sys.exit(1)

    if 'threads' not in df.columns or 'time_ms' not in df.columns:
        print("El CSV debe tener columnas 'threads' y 'time_ms'")
        sys.exit(1)

    plt.figure(figsize=(10, 6))
    plt.plot(df['threads'], df['time_ms'], marker='o', linestyle='-')
    plt.title('Rendimiento: Tiempo vs Threads')
    plt.xlabel('Cantidad de Threads')
    plt.ylabel('Tiempo (ms)')
    plt.grid(True)
    
    # Guardar acá
    output_dir = "Analisis_estadistica/graficas"
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
        
    output_file = os.path.join(output_dir, "rendimiento.png")
    plt.savefig(output_file)
    print(f"Gráfico guardado en: {output_file}")

if __name__ == "__main__":
    main()
