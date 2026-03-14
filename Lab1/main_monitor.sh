#!/bin/bash

# Rutas de scripts y archivos
LOG_DIR="$HOME/system_monitor_logs"
DAEMON_SCRIPT="./advanced_system_monitor.sh"
ALERT_SCRIPT="./alert_system.sh"
REPORT_SCRIPT="./generate_report.sh"
CONFIG_FILE="$LOG_DIR/monitor.conf"

# Colores
BLUE='\033[0;34m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

# --- Funciones de Lógica ---

init_config() {
  if [ ! -f "$CONFIG_FILE" ]; then
    echo "RAM_THRESHOLD=90" > "$CONFIG_FILE"
    echo "CPU_THRESHOLD=5" >> "$CONFIG_FILE"
    echo "DISK_THRESHOLD=85" >> "$CONFIG_FILE"
  fi
}

edit_config() {
  echo -e "${YELLOW}--- Configuración de Umbrales ---${NC}"
  read -p "Nuevo umbral RAM (%): " ram
  read -p "Nuevo umbral CPU (Load): " cpu
  read -p "Nuevo umbral Disco (%): " disk
  echo -e "RAM_THRESHOLD=$ram\nCPU_THRESHOLD=$cpu\nDISK_THRESHOLD=$disk" > "$CONFIG_FILE"
  echo -e "${GREEN}Configuración actualizada.${NC}"
}

manage_daemon() {
  case $1 in
    start)
      nohup bash "$DAEMON_SCRIPT" --daemon > /dev/null 2>&1 &
      echo -e "${GREEN}Daemon iniciado.${NC}"
      ;;
    stop)
      pkill -f "advanced_system_monitor.sh --daemon"
      echo -e "${RED}Daemon detenido.${NC}"
      ;;
  esac
}

# --- Manejo de Argumentos (Punto 2) ---
if [[ $# -gt 0 ]]; then
  case "$1" in
    --daemon)
      manage_daemon start
      exit 0
      ;;
    --report)
      bash "$REPORT_SCRIPT"
      exit 0
      ;;
    --alert)
      bash "$ALERT_SCRIPT"
      exit 0
      ;;
    --config)
      edit_config
      exit 0
      ;;
    *)
      echo "Uso: $0 {--daemon|--report|--alert|--config}"
      exit 1
      ;;
  esac
fi

# --- Menú Interactivo (Punto 1) ---
init_config
while true; do
  clear
  echo -e "${BLUE}=== PANEL DE CONTROL DE MONITOREO ===${NC}"
  echo "1. Iniciar/Detener Daemon"
  echo "2. Ver estadísticas en tiempo real"
  echo "3. Generar Reportes (Texto/CSV/HTML)"
  echo "4. Configurar umbrales de alerta"
  echo "5. Ver historial de alertas"
  echo "6. Salir"
  read -p "Seleccione: " opt

  case $opt in
    1)
      if pgrep -f "advanced_system_monitor.sh --daemon" > /dev/null; then
        manage_daemon stop
      else
        manage_daemon start
      fi
      sleep 2
      ;;
    2)
      bash "$DAEMON_SCRIPT"
      read -p "Enter..."
      ;;
    3)
      bash "$REPORT_SCRIPT"
      read -p "Enter..."
      ;;
    4)
      edit_config
      sleep 2
      ;;
    5)
      [ -f "$LOG_DIR/alerts.log" ] && tail -n 20 "$LOG_DIR/alerts.log" || echo "Sin alertas."
      read -p "Enter..."
      ;;
    6) exit 0 ;;
  esac
done
