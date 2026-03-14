#!/bin/bash

if [ -d ~/system_monitor_logs/ ]; then
  echo "Log directory already exists."
else
  mkdir -p ~/system_monitor_logs/
  echo "Log directory created."
fi

LOG_DIR="$HOME/system_monitor_logs"
LOG_FILE="$LOG_DIR/system_stats.csv"

recolect_data() {
  TIMESTAMP=$(date '+%Y-%m-%d %H:%M:%S')
  CPU_USAGE=$(mpstat | grep 'all' | awk '{print 100-$13}')

  MEM_USAGE=$(free -h | grep 'Mem' | awk '{print $3}')
  MEM_USAGE_PORC=$(free -m | grep 'Mem' | awk '{print $3/$2 * 100}')

  # Red: Interfaz principal
  INTERFACE=$(ip route | grep default | awk '{print $5}' | head -n1)
  RX=$(cat /proc/net/dev | grep "$INTERFACE" | awk '{print $2}')
  TX=$(cat /proc/net/dev | grep "$INTERFACE" | awk '{print $10}')

  # Crear cabecera si el archivo es nuevo
  [ ! -f "$LOG_FILE" ] && echo "Timestamp,CPU,Mem,Mem%,RX,TX" > "$LOG_FILE"

  echo "$TIMESTAMP, $CPU_USAGE, $MEM_USAGE, $MEM_USAGE_PORC, $RX, $TX" >> "$LOG_FILE"
}

analyze_memory() {
  # Calcular promedio de memoria (últimas 12 lecturas = 1 hora)
  local file="$LOG_FILE"
  if [ -f "$file" ]; then
    local avg=$(tail -n 12 "$file" | awk -F', ' '{sum+=$4; count++} END {if (count > 0) print sum/count; else print 0}')
    echo "Promedio de uso de RAM (última hora): $avg%"
  fi
}

detect_leaks() {
  #Comparar uso actual con promedio de 1 hora
  local file="$LOG_FILE"
  local current_usage, avg_hour
  current_usage=$(free -m | grep 'Mem' | awk '{print $3/$2 * 100}')
  avg_hour=$(tail -n 12 "$file" | awk -F', ' '{sum+=$4; count++} END {if (count > 0) print sum/count; else print 0}')

  # Si el uso actual supera el promedio por más de un 15%, alertar posible leak
  if (($(echo "$current_usage > ($avg_hour + 15)" | bc -l))); then
    echo "ALERTA: Posible fuga de memoria detectada. Uso actual: $current_usage%"
  fi
}

historical_top_processes() {
  #Identificar procesos top actuales para comparación histórica
  echo "===== Procesos Top Actuales vs Histórico ====="
  ps aux --sort=-%mem | head -n 6
}

network_advanced() {
  local interface, status, est_conn, rx_stats, rx_errs, rx_drop, tx_errs, tx_drop
  #Identificar interfaz principal y estado
  interface=$(ip route | grep default | awk '{print $5}' | head -n1)
  status=$(ip link show "$interface" | grep -oP 'state \K\w+')

  # Conexiones establecidas
  est_conn=$(ss -tun | grep -c "ESTAB")

  # Errores y paquetes perdidos (Drops)
  # columna 4 (errs) y 5 (drop) de RX, y la 12 (errs) y 13 (drop) de TX
  rx_stats=$(cat /proc/net/dev | grep "$interface")
  rx_errs=$(echo $rx_stats | awk '{print $4}')
  rx_drop=$(echo $rx_stats | awk '{print $5}')
  tx_errs=$(echo $rx_stats | awk '{print $12}')
  tx_drop=$(echo $rx_stats | awk '{print $13}')

  echo "===== Red Avanzada ($interface) ====="
  echo "Estado: $status"
  echo "Conexiones Establecidas: $est_conn"
  echo "Errores (RX/TX): $rx_errs / $tx_errs"
  echo "Paquetes perdidos (RX/TX): $rx_drop / $tx_drop"

  # Log errores si existen
  if [ "$rx_errs" -gt 0 ] || [ "$tx_errs" -gt 0 ]; then
    echo "$(date '+%Y-%m-%d %H:%M:%S') - ERROR detectado en $interface. RX_err: $rx_errs, TX_err: $tx_errs" >> ~/system_monitor_logs/network_errors.log
  fi
}

# Monitorización continua si se pasa el argumento --daemon
if [[ "$1" == "--daemon" ]]; then
  echo "Iniciando modo monitorización continua (cada 5 min)..."
  while true; do
    recolect_data
    sleep 300 # Pausa de 5 minutos
  done
else
  recolect_data
  echo "Datos recolectados una única vez."
fi
