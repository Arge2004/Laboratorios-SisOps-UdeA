#!/bin/bash

# Importar o definir las rutas de los otros scripts
LOG_DIR="$HOME/system_monitor_logs"
DAEMON_SCRIPT="./advanced_system_monitor.sh" # El script que hicimos en la Tarea 1
ALERT_SCRIPT="./alert_system.sh"             # El script de la Tarea 2
REPORT_SCRIPT="./generate_report.sh"         # El script de la Tarea 3

# Colores para el menú
BLUE='\033[0;34m'
GREEN='\033[0;32m'
CYAN='\033[0;36m'
YELLOW='\033[1;33m'
NC='\033[0m'

show_menu() {
  clear
  echo -e "${BLUE}==========================================${NC}"
  echo -e "${BLUE}   SISTEMA DE MONITOREO AVANZADO - LINUX  ${NC}"
  echo -e "${BLUE}==========================================${NC}"
  echo -e "1. ${CYAN}Monitoreo en Tiempo Real (Una ejecución)${NC}"
  echo -e "2. ${CYAN}Iniciar Servicio en Segundo Plano (Daemon)${NC}"
  echo -e "3. ${CYAN}Detener Servicio en Segundo Plano${NC}"
  echo -e "4. ${CYAN}Ejecutar Análisis de Alertas Manual${NC}"
  echo -e "5. ${CYAN}Generar Reportes (TXT, CSV, HTML)${NC}"
  echo -e "6. ${YELLOW}Ver Últimas Alertas Registradas${NC}"
  echo -e "7. ${GREEN}Salir${NC}"
  echo -e "${BLUE}------------------------------------------${NC}"
  echo -n "Seleccione una opción [1-7]: "
}

manage_daemon() {
  case $1 in
    start)
      if pgrep -f "advanced_system_monitor.sh --daemon" > /dev/null; then
        echo -e "${YELLOW}El servicio ya está corriendo.${NC}"
      else
        nohup bash "$DAEMON_SCRIPT" --daemon > /dev/null 2>&1 &
        echo -e "${GREEN}Servicio iniciado correctamente en segundo plano.${NC}"
      fi
      ;;
    stop)
      pkill -f "advanced_system_monitor.sh --daemon"
      echo -e "${YELLOW}Servicio detenido.${NC}"
      ;;
  esac
  read -p "Presione Enter para continuar..."
}

# --- Bucle Principal del Menú ---
while true; do
  show_menu
  read opcion
  case $opcion in
    1)
      bash "$DAEMON_SCRIPT" # Ejecuta la función recolect_data una vez
      read -p "Presione Enter para continuar..."
      ;;
    2)
      manage_daemon start
      ;;
    3)
      manage_daemon stop
      ;;
    4)
      bash "$ALERT_SCRIPT"
      read -p "Presione Enter para continuar..."
      ;;
    5)
      bash "$REPORT_SCRIPT"
      read -p "Presione Enter para continuar..."
      ;;
    6)
      echo -e "${YELLOW}--- Últimas 10 Alertas ---${NC}"
      [ -f "$LOG_DIR/alerts.log" ] && tail -n 10 "$LOG_DIR/alerts.log" || echo "No hay alertas registradas."
      read -p "Presione Enter para continuar..."
      ;;
    7)
      echo "Saliendo del sistema..."
      exit 0
      ;;
    *)
      echo -e "${RED}Opción no válida.${NC}"
      sleep 1
      ;;
  esac
done
