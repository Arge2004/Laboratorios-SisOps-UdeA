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

trim() {
  echo "$1" | xargs
}

get_top5_avg_cpu() {
  local tmp_file
  tmp_file=$(mktemp)

  # Muestreamos CPU por proceso 3 veces para estimar promedio corto.
  for _ in 1 2 3; do
    ps -eo pid,comm,%cpu --no-headers >> "$tmp_file"
    sleep 1
  done

  awk '
    {
      key = $1 "|" $2
      sum[key] += $3
      count[key] += 1
    }
    END {
      for (k in sum) {
        split(k, p, "|")
        avg = sum[k] / count[k]
        printf "%s,%s,%.2f\n", p[1], p[2], avg
      }
    }
  ' "$tmp_file" | sort -t',' -k3 -nr | head -n 5

  rm -f "$tmp_file"
}


echo "Seleccione tipo de reporte:"
echo "1) Hoy"
echo "2) Rango personalizado (YYYY-MM-DD)"
read -p "Opción: " R_OPC

if [ "$R_OPC" == "2" ]; then
  read -p "Fecha Inicio: " START
  read -p "Fecha Fin: " END
  DATA_SOURCE=$(awk -F',' -v s="$START" -v e="$END" 'NR>1 { d=substr($1,1,10); gsub(/^ +| +$/, "", d); if (d >= s && d <= e) print }' "$LOG_FILE")
  TITULO="REPORTE RANGO $START a $END"
  FECHA_CSV="${START}_to_${END}"
else
  FECHA_ACTUAL=$(date '+%Y-%m-%d')
  DATA_SOURCE=$(awk -F',' -v d="$FECHA_ACTUAL" 'NR>1 { ts=$1; gsub(/^ +| +$/, "", ts); if (index(ts, d) == 1) print }' "$LOG_FILE")
  TITULO="REPORTE DIARIO - $FECHA_ACTUAL"
  FECHA_CSV="$FECHA_ACTUAL"
fi

if [ -z "$DATA_SOURCE" ]; then
  echo "No se encontraron datos para el periodo seleccionado."
  exit 1
fi

# --- 1. Extracción de métricas ---
UPTIME=$(uptime -p)
LOAD_ALL=$(uptime | awk -F'load average:' '{print $2}' | xargs)
LOAD_1=$(echo "$LOAD_ALL" | cut -d',' -f1 | xargs)
LOAD_5=$(echo "$LOAD_ALL" | cut -d',' -f2 | xargs)
LOAD_15=$(echo "$LOAD_ALL" | cut -d',' -f3 | xargs)

# Pico de RAM sobre los datos filtrados
PEAK_LINE=$(echo "$DATA_SOURCE" | awk -F', *' 'BEGIN {max=-1} {v=$4+0; if (v>max) {max=v; line=$0}} END {print line}')
PEAK_VAL=$(trim "$(echo "$PEAK_LINE" | awk -F', *' '{print $4}')")
PEAK_TIME=$(trim "$(echo "$PEAK_LINE" | awk -F', *' '{print $1}')")

# Tráfico (Diferencia entre el primer y último registro del filtro)
FIRST_RX=$(echo "$DATA_SOURCE" | awk -F', *' 'NR==1 {print $5}')
LAST_RX=$(echo "$DATA_SOURCE" | awk -F', *' 'END {print $5}')
FIRST_TX=$(echo "$DATA_SOURCE" | awk -F', *' 'NR==1 {print $6}')
LAST_TX=$(echo "$DATA_SOURCE" | awk -F', *' 'END {print $6}')

DELTA_RX=$((LAST_RX - FIRST_RX))
DELTA_TX=$((LAST_TX - FIRST_TX))
[ "$DELTA_RX" -lt 0 ] && DELTA_RX=0
[ "$DELTA_TX" -lt 0 ] && DELTA_TX=0

TOTAL_RX_MB=$(echo "scale=2; $DELTA_RX / 1024 / 1024" | bc -l)
TOTAL_TX_MB=$(echo "scale=2; $DELTA_TX / 1024 / 1024" | bc -l)

TOP5_TABLE=$(get_top5_avg_cpu)
TOP5_ROWS_HTML=$(echo "$TOP5_TABLE" | awk -F',' '
  NF==3 { printf "<tr><td>%s</td><td>%s</td><td>%.2f%%</td></tr>\n", $1, $2, $3 }
')

if [ -z "$TOP5_ROWS_HTML" ]; then
  TOP5_ROWS_HTML="<tr><td colspan=\"3\">Sin datos de procesos.</td></tr>"
fi

# --- 2. Formato CSV ---
[ ! -f "$REPORT_CSV" ] && echo "Periodo,Uptime,Carga_1min,Carga_5min,Carga_15min,Pico_RAM_%,Hora_Pico,Total_RX_MB,Total_TX_MB" > "$REPORT_CSV"
echo "$FECHA_CSV,$UPTIME,$LOAD_1,$LOAD_5,$LOAD_15,$PEAK_VAL,$PEAK_TIME,$TOTAL_RX_MB,$TOTAL_TX_MB" >> "$REPORT_CSV"

# --- 3. Formato Texto ---
{
  echo "==========================================================================="
  echo "                           $TITULO"
  echo "==========================================================================="
  echo
  echo "[SYSTEM OVERVIEW]"
  echo "Uptime del sistema      : $UPTIME"
  echo "Carga promedio (1/5/15) : $LOAD_1 / $LOAD_5 / $LOAD_15"
  echo
  echo "[MEMORY]"
  echo "Pico de memoria (%)     : $PEAK_VAL"
  echo "Timestamp del pico      : $PEAK_TIME"
  echo
  echo "[NETWORK TRAFFIC SUMMARY]"
  echo "Total recibido (RX MB)  : $TOTAL_RX_MB"
  echo "Total enviado (TX MB)   : $TOTAL_TX_MB"
  echo
  echo "[TOP 5 PROCESSES BY AVG CPU]"
  printf "%-8s %-28s %12s\n" "PID" "Proceso" "CPU Promedio"
  printf "%-8s %-28s %12s\n" "--------" "----------------------------" "------------"

  if [ -n "$TOP5_TABLE" ]; then
    while IFS=',' read -r pid proc avgcpu; do
      [ -z "$pid" ] && continue
      printf "%-8s %-28s %10s%%\n" "$pid" "$proc" "$avgcpu"
    done <<< "$TOP5_TABLE"
  else
    echo "Sin datos de procesos."
  fi

  echo
  echo "==========================================================================="
} > "$REPORT_TXT"

# --- 4. Formato HTML ---
cat << EOF > "$REPORT_HTML"
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>$TITULO</title>
    <style>
    :root {
      --bg-1: #f5f0e8;
      --bg-2: #e3edf7;
      --paper: #fffdf8;
      --ink: #22252b;
      --muted: #5d6470;
      --accent: #0f766e;
      --accent-2: #b45309;
      --line: #d8dde6;
    }

    * { box-sizing: border-box; }

    body {
      margin: 0;
      font-family: "Trebuchet MS", "Gill Sans", sans-serif;
      color: var(--ink);
      background: radial-gradient(circle at 10% 20%, var(--bg-1), transparent 40%),
            radial-gradient(circle at 85% 15%, #f9e7d2, transparent 35%),
            linear-gradient(135deg, var(--bg-2), #f0f6ff 55%, #fff8ef);
      min-height: 100vh;
    }

    .container {
      max-width: 980px;
      margin: 28px auto;
      padding: 0 16px;
    }

    .hero {
      background: linear-gradient(120deg, rgba(15, 118, 110, 0.95), rgba(180, 83, 9, 0.88));
      color: #fff;
      border-radius: 18px;
      padding: 28px 24px;
      box-shadow: 0 14px 30px rgba(26, 38, 57, 0.22);
    }

    .hero h1 {
      margin: 0;
      font-size: clamp(1.4rem, 2.3vw, 2rem);
      letter-spacing: 0.6px;
    }

    .hero p {
      margin: 10px 0 0;
      opacity: 0.92;
    }

    .grid {
      margin-top: 18px;
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(240px, 1fr));
      gap: 14px;
    }

    .card {
      background: var(--paper);
      border: 1px solid var(--line);
      border-radius: 14px;
      padding: 16px;
      box-shadow: 0 8px 18px rgba(29, 34, 44, 0.08);
    }

    .card h2 {
      margin-top: 0;
      margin-bottom: 10px;
      font-size: 1rem;
      letter-spacing: 0.4px;
      color: var(--accent);
      text-transform: uppercase;
    }

    .metric {
      display: flex;
      justify-content: space-between;
      gap: 8px;
      padding: 6px 0;
      border-bottom: 1px dashed #e4e7ed;
    }

    .metric:last-child { border-bottom: none; }
    .label { color: var(--muted); }
    .value { font-weight: 700; }

    .table-wrap {
      margin-top: 14px;
      overflow-x: auto;
    }

    table {
      width: 100%;
      border-collapse: collapse;
      background: var(--paper);
      border-radius: 12px;
      overflow: hidden;
    }

    thead {
      background: #15263c;
      color: #f5f7fb;
    }

    th, td {
      text-align: left;
      padding: 11px 12px;
      border-bottom: 1px solid #e8ebf2;
    }

    tbody tr:nth-child(even) { background: #f8fafc; }

    .footer {
      margin-top: 14px;
      color: var(--muted);
      font-size: 0.9rem;
      text-align: right;
    }
    </style>
</head>
<body>
  <div class="container">
    <section class="hero">
      <h1>$TITULO</h1>
      <p>Resumen operativo del sistema con carga, memoria, procesos y trafico de red.</p>
    </section>

    <section class="grid">
      <article class="card">
        <h2>System Overview</h2>
        <div class="metric"><span class="label">Uptime</span><span class="value">$UPTIME</span></div>
        <div class="metric"><span class="label">Load 1 min</span><span class="value">$LOAD_1</span></div>
        <div class="metric"><span class="label">Load 5 min</span><span class="value">$LOAD_5</span></div>
        <div class="metric"><span class="label">Load 15 min</span><span class="value">$LOAD_15</span></div>
      </article>

      <article class="card">
        <h2>Peak Memory</h2>
        <div class="metric"><span class="label">Peak RAM (%)</span><span class="value">$PEAK_VAL%</span></div>
        <div class="metric"><span class="label">Timestamp</span><span class="value">$PEAK_TIME</span></div>
      </article>

      <article class="card">
        <h2>Network Summary</h2>
        <div class="metric"><span class="label">Total RX</span><span class="value">${TOTAL_RX_MB} MB</span></div>
        <div class="metric"><span class="label">Total TX</span><span class="value">${TOTAL_TX_MB} MB</span></div>
      </article>
    </section>

    <section class="table-wrap">
      <table>
        <thead>
          <tr>
            <th>PID</th>
            <th>Process</th>
            <th>Average CPU</th>
          </tr>
        </thead>
        <tbody>
          $TOP5_ROWS_HTML
        </tbody>
      </table>
    </section>

    <div class="footer">Generado: $(date '+%Y-%m-%d %H:%M:%S')</div>
    </div>
</body>
</html>
EOF

echo "¡Hecho! Reportes generados en $REPORT_DIR"
