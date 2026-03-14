#!/bin/bash

# Configuración de archivos
LOG_FILE="$HOME/system_monitor_logs/system_stats.csv"
REPORT_TXT="$HOME/system_monitor_logs/daily_report.txt"
REPORT_CSV="$HOME/system_monitor_logs/daily_report.csv"
REPORT_HTML="$HOME/system_monitor_logs/daily_report.html"

# Verificar si hay datos
if [ ! -f "$LOG_FILE" ]; then
  echo "Error: No hay datos históricos para generar el reporte."
  exit 1
fi

# --- 1. Extracción de métricas ---
FECHA_REPORTE=$(date '+%Y-%m-%d')
UPTIME=$(uptime -p)
# Promedio de carga de los últimos 1, 5 y 15 min
LOAD_1=$(uptime | awk -F'load average:' '{print $2}' | cut -d',' -f1)
LOAD_5=$(uptime | awk -F'load average:' '{print $2}' | cut -d',' -f2)

# Pico de Memoria (Columna 4 del log original)
PEAK_MEM_LINE=$(sort -t',' -k4 -nr "$LOG_FILE" | head -n1)
PEAK_VAL=$(echo "$PEAK_MEM_LINE" | cut -d',' -f4)
PEAK_TIME=$(echo "$PEAK_MEM_LINE" | cut -d',' -f1)

# Tráfico de Red Total (Calculado por diferencia entre el primer y último registro)
START_RX=$(head -n 2 "$LOG_FILE" | tail -n 1 | cut -d',' -f5)
END_RX=$(tail -n 1 "$LOG_FILE" | cut -d',' -f5)
TOTAL_RX_MB=$(echo "scale=2; ($END_RX - $START_RX) / 1024 / 1024" | bc -l)

# --- 2. Generación del Formato CSV (Task 3.2) ---
# Creamos la cabecera si el archivo no existe
[ ! -f "$REPORT_CSV" ] && echo "Fecha,Uptime,Carga_1min,Pico_RAM_%,Hora_Pico,Total_RX_MB" > "$REPORT_CSV"

# Añadimos la línea de datos del día
echo "$FECHA_REPORTE,$UPTIME,$LOAD_1,$PEAK_VAL,$PEAK_TIME,$TOTAL_RX_MB" >> "$REPORT_CSV"

# --- 3. Generación del Formato Texto (Task 3.1) ---
{
  echo "=========================================="
  echo "     REPORTE SISTEMA - $FECHA_REPORTE"
  echo "=========================================="
  echo "Uptime: $UPTIME"
  echo "Carga promedio (1 min): $LOAD_1"
  echo "Pico de Memoria: $PEAK_VAL% registrado a las $PEAK_TIME"
  echo "Tráfico Red Recibido hoy: ${TOTAL_RX_MB} MB"
  echo "------------------------------------------"
} > "$REPORT_TXT"

# --- 4. Generación del Formato HTML (Bonus) ---
cat << EOF > "$REPORT_HTML"
<html>
<head>
    <title>Reporte $FECHA_REPORTE</title>
    <style>
        body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; margin: 40px; background-color: #f0f2f5; }
        .card { background: white; padding: 20px; border-radius: 8px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }
        h1 { color: #1a73e8; border-bottom: 2px solid #1a73e8; padding-bottom: 10px; }
        .stat { font-size: 1.2em; margin: 15px 0; color: #3c4043; }
        .label { font-weight: bold; color: #5f6368; }
    </style>
</head>
<body>
    <div class="card">
        <h1>Resumen de Monitoreo: $FECHA_REPORTE</h1>
        <div class="stat"><span class="label">Tiempo de actividad:</span> $UPTIME</div>
        <div class="stat"><span class="label">Carga del Sistema:</span> $LOAD_1</div>
        <div class="stat"><span class="label">Uso máximo de RAM:</span> $PEAK_VAL% ($PEAK_TIME)</div>
        <div class="stat"><span class="label">Datos recibidos:</span> ${TOTAL_RX_MB} MB</div>
    </div>
</body>
</html>
EOF

echo "Se han generado 3 archivos en $HOME/system_monitor_logs/:"
echo "1. daily_report.txt"
echo "2. daily_report.csv"
echo "3. daily_report.html"
