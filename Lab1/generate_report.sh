#!/bin/bash

# Configuración de archivos
LOG_FILE="$HOME/system_monitor_logs/system_stats.csv"
REPORT_DIR="$HOME/system_monitor_logs"
REPORT_TXT="$REPORT_DIR/daily_report.txt"
REPORT_CSV="$REPORT_DIR/daily_report.csv"
REPORT_HTML="$REPORT_DIR/daily_report.html"

# Verificar si hay datos
if [ ! -f "$LOG_FILE" ]; then
  echo "Error: No hay datos históricos para generar el reporte."
  exit 1
fi

# --- LÓGICA DE RANGO (Tarea 4) ---
echo "Seleccione tipo de reporte:"
echo "1) Hoy"
echo "2) Rango personalizado (YYYY-MM-DD)"
read -p "Opción: " R_OPC

if [ "$R_OPC" == "2" ]; then
  read -p "Fecha Inicio: " START
  read -p "Fecha Fin: " END
  DATA_SOURCE=$(awk -F',' -v s="$START" -v e="$END" '$1 >= s && $1 <= e' "$LOG_FILE")
  TITULO="REPORTE RANGO $START a $END"
  FECHA_CSV="${START}_to_${END}"
else
  FECHA_ACTUAL=$(date '+%Y-%m-%d')
  DATA_SOURCE=$(grep "$FECHA_ACTUAL" "$LOG_FILE")
  TITULO="REPORTE DIARIO - $FECHA_ACTUAL"
  FECHA_CSV="$FECHA_ACTUAL"
fi

if [ -z "$DATA_SOURCE" ]; then
  echo "No se encontraron datos para el periodo seleccionado."
  exit 1
fi

# --- 1. Extracción de métricas ---
UPTIME=$(uptime -p)
LOAD_1=$(uptime | awk -F'load average:' '{print $2}' | cut -d',' -f1)

# Pico de RAM sobre los datos filtrados
PEAK_LINE=$(echo "$DATA_SOURCE" | sort -t',' -k4 -nr | head -n1)
PEAK_VAL=$(echo "$PEAK_LINE" | cut -d',' -f4)
PEAK_TIME=$(echo "$PEAK_LINE" | cut -d',' -f1)

# Tráfico (Diferencia entre el primer y último registro del filtro)
FIRST_RX=$(echo "$DATA_SOURCE" | head -n 1 | cut -d',' -f5)
LAST_RX=$(echo "$DATA_SOURCE" | tail -n 1 | cut -d',' -f5)
TOTAL_RX_MB=$(echo "scale=2; ($LAST_RX - $FIRST_RX) / 1024 / 1024" | bc -l)

# --- 2. Formato CSV ---
[ ! -f "$REPORT_CSV" ] && echo "Periodo,Uptime,Carga_1min,Pico_RAM_%,Hora_Pico,Total_RX_MB" > "$REPORT_CSV"
echo "$FECHA_CSV,$UPTIME,$LOAD_1,$PEAK_VAL,$PEAK_TIME,$TOTAL_RX_MB" >> "$REPORT_CSV"

# --- 3. Formato Texto ---
{
  echo "=========================================="
  echo "     $TITULO"
  echo "=========================================="
  echo "Uptime: $UPTIME"
  echo "Carga promedio: $LOAD_1"
  echo "Pico de Memoria: $PEAK_VAL% registrado a las $PEAK_TIME"
  echo "Tráfico Red Recibido: ${TOTAL_RX_MB} MB"
  echo "------------------------------------------"
} > "$REPORT_TXT"

# --- 4. Formato HTML ---
cat << EOF > "$REPORT_HTML"
<html>
<head>
    <title>$TITULO</title>
    <style>
        body { font-family: 'Segoe UI', sans-serif; margin: 40px; background-color: #f0f2f5; }
        .card { background: white; padding: 20px; border-radius: 8px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }
        h1 { color: #1a73e8; border-bottom: 2px solid #1a73e8; padding-bottom: 10px; }
        .stat { font-size: 1.2em; margin: 15px 0; color: #3c4043; }
        .label { font-weight: bold; color: #5f6368; }
    </style>
</head>
<body>
    <div class="card">
        <h1>$TITULO</h1>
        <div class="stat"><span class="label">Tiempo de actividad:</span> $UPTIME</div>
        <div class="stat"><span class="label">Carga del Sistema:</span> $LOAD_1</div>
        <div class="stat"><span class="label">Uso máximo de RAM:</span> $PEAK_VAL% ($PEAK_TIME)</div>
        <div class="stat"><span class="label">Datos recibidos:</span> ${TOTAL_RX_MB} MB</div>
    </div>
</body>
</html>
EOF

echo "¡Hecho! Reportes generados en $REPORT_DIR"
