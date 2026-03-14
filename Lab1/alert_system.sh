#!/bin/bash

# Configuración de rutas y archivos
LOG_DIR="$HOME/system_monitor_logs"
ALERT_LOG="$LOG_DIR/alerts.log"
# Archivo temporal para controlar el anti-flooding
LAST_ALERT_TIME="/tmp/last_alert_check"
# Cargar configuración (Tarea 2 integración)
CONFIG_FILE="$HOME/system_monitor_logs/monitor.conf"

# Valores por defecto por si el archivo no existe
RAM_THRESHOLD=90
CPU_THRESHOLD=5
DISK_THRESHOLD=85

# Si el archivo existe, sobrescribe los valores
if [ -f "$CONFIG_FILE" ]; then
  source "$CONFIG_FILE"
fi

# Colores para la consola
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Asegurar que el log de alertas exista
mkdir -p "$LOG_DIR"
touch "$ALERT_LOG"

send_alert() {
  local level=$1 # CRITICAL o WARNING
  local message=$2
  local color=$3
  local timestamp, alert_id, last_time, now
  timestamp=$(date '+%Y-%m-%d %H:%M:%S')

  #Prevenir inundación de alertas (mínimo 5 min entre alertas iguales)
  alert_id=$(echo "$message" | cksum | awk '{print $1}')
  last_time=$(grep "$alert_id" "$LAST_ALERT_TIME" 2> /dev/null | awk '{print $2}')
  now=$(date +%s)

  if [[ -z "$last_time" ]] || ((now - last_time > 300)); then
    # Imprimir en consola con color
    echo -e "${color}[$level] $message${NC}"

    # Loguear alerta
    echo "$timestamp [$level] $message" >> "$ALERT_LOG"

    # Actualizar timestamp de última alerta de este tipo
    sed -i "/$alert_id/d" "$LAST_ALERT_TIME" 2> /dev/null
    echo "$alert_id $now" >> "$LAST_ALERT_TIME"
  fi
}

check_alerts() {
  local ram_usage cpu_load disk_usage interface net_status
  #RAM usage > 90%
  ram_usage=$(free | grep Mem | awk '{print $3/$2 * 100.0}')
  if (($(echo "$ram_usage > $RAM_THRESHOLD" | bc -l))); then
    send_alert "WARNING" "RAM usage is high: ${ram_usage}%" "$YELLOW"
  fi

  #CPU load average > 5
  cpu_load=$(uptime | awk -F'load average:' '{ print $2 }' | cut -d, -f1 | xargs)
  if (($(echo "$cpu_load > $CPU_THRESHOLD" | bc -l))); then
    send_alert "CRITICAL" "CPU Load Average is high: $cpu_load" "$RED"
  fi

  #Disk usage > 85% on root partition
  disk_usage=$(df / | grep / | awk '{ print $5 }' | sed 's/%//')
  if [ "$disk_usage" -gt "$DISK_THRESHOLD" ]; then
    send_alert "CRITICAL" "Disk usage on root partition is > $DISK_THRESHOLD% ($disk_usage%)" "$RED"
  fi

  #Network interface down detection
  interface=$(ip route | grep default | awk '{print $5}' | head -n1)
  net_status=$(cat /sys/class/net/"$interface"/operstate)
  if [ "$net_status" != "up" ]; then
    send_alert "CRITICAL" "Network interface $interface is DOWN" "$RED"
  fi
}

check_alerts
