import os
import sys
import pandas as pd
import matplotlib.pyplot as plt

def main():
    stats_file = os.getenv("GAME_STATS_FILE")
    output_dir = os.getenv("GRAFICOS_ESTADISTICAS")

    print(f"Leyendo datos de: {stats_file}")
    
    # Manejar errores al leer el csv
    try:
        df = pd.read_csv(stats_file)
    except Exception as e:
        print(f"Error al leer el csv: {e}")
        sys.exit(1)

    if df.empty:
        print("Error, el csv está vacío.")
        sys.exit(0)

    # Asegurar que el directorio de salida exista
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    # 1. Gráfico de victorias por equipo
    plt.figure(figsize=(10, 6))
    if 'WinnerTeam' in df.columns:
        victory_counts = df['WinnerTeam'].value_counts()
        victory_counts.plot(kind='bar', color='skyblue', edgecolor='black')
        plt.title('Victorias por equipo')
        plt.xlabel('Equipo')
        plt.ylabel('Cantidad de victorias')
        plt.xticks(rotation=45)
        plt.tight_layout()
        plt.savefig(os.path.join(output_dir, 'victorias_por_equipo.png'))
        plt.close()
        print("Generado: victorias_por_equipo.png")

    # 2. Gráfico de turnos vs duración
    plt.figure(figsize=(10, 6))
    if 'TotalTurns' in df.columns and 'DurationSeconds' in df.columns:
        plt.scatter(df['TotalTurns'], df['DurationSeconds'], alpha=0.7, c='green')
        plt.title('Relación turnos vs duración')
        plt.xlabel('Total de turnos')
        plt.ylabel('Duración (segundos)')
        plt.grid(True, linestyle='--', alpha=0.6)
        plt.tight_layout()
        plt.savefig(os.path.join(output_dir, 'turnos_vs_duracion.png'))
        plt.close()
        print("Generado: turnos_vs_duracion.png")

    # 3. Gráfico de cantidad de partidas por número de equipos
    plt.figure(figsize=(10, 6))
    if 'NumTeams' in df.columns:
        games_per_team_count = df['NumTeams'].value_counts().sort_index()
        games_per_team_count.plot(kind='bar', color='orange', edgecolor='black')
        plt.title('Cantidad de partidas por número de equipos')
        plt.xlabel('Número de equipos')
        plt.ylabel('Cantidad de partidas')
        plt.xticks(rotation=0)
        plt.grid(axis='y', linestyle='--', alpha=0.6)
        plt.tight_layout()
        plt.savefig(os.path.join(output_dir, 'partidas_por_num_equipos.png'))
        plt.close()
        print("Generado: partidas_por_num_equipos.png")

    # 4. Gráfico de promedio de turnos por número de equipos
    plt.figure(figsize=(10, 6))
    if 'NumTeams' in df.columns and 'TotalTurns' in df.columns:
        avg_turns_per_team_count = df.groupby('NumTeams')['TotalTurns'].mean()
        avg_turns_per_team_count.plot(kind='bar', color='purple', edgecolor='black')
        plt.title('Promedio de turnos por número de equipos')
        plt.xlabel('Número de equipos')
        plt.ylabel('Promedio de turnos')
        plt.xticks(rotation=0)
        plt.grid(axis='y', linestyle='--', alpha=0.6)
        plt.tight_layout()
        plt.savefig(os.path.join(output_dir, 'promedio_turnos_por_equipos.png'))
        plt.close()
        print("Generado: promedio_turnos_por_equipos.png")

    print(f"Todos los gráficos han sido generados en: {output_dir}")

main()
