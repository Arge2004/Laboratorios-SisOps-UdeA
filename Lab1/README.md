# Lab 1 - Guia de ejecucion

Este laboratorio monta un mini sistema de monitoreo en Bash para Linux. La idea es simple: recolectar metricas del sistema, lanzar alertas cuando algo se sale de rango y generar reportes faciles de leer.

Incluye:

- Recoleccion de metricas de CPU, RAM y red
- Ejecucion puntual o continua en modo daemon
- Alertas con umbrales configurables
- Reportes en TXT, CSV y HTML

## Scripts incluidos

- `main_monitor.sh`: punto de entrada (menu y modo por argumentos)
- `advanced_system_monitor.sh`: captura metricas, las guarda en CSV y en modo puntual tambien las muestra en consola
- `alert_system.sh`: valida umbrales y registra alertas
- `generate_report.sh`: genera reportes a partir del historial

## Requisitos

Sistema Linux con Bash y utilidades:

- `awk`, `grep`, `sed`, `df`, `free`, `ps`, `tail`, `sort`
- `bc`
- `ip`, `ss` (paquete iproute2)
- `mpstat` (paquete sysstat)

Instalacion rapida (Debian/Ubuntu):

```bash
sudo apt update
sudo apt install -y bc iproute2 sysstat procps
```

## Preparacion

Desde la carpeta `Lab1`:

```bash
cd Lab1
chmod +x main_monitor.sh advanced_system_monitor.sh alert_system.sh generate_report.sh
```

Nota: ejecuta `main_monitor.sh` desde esta carpeta para que encuentre correctamente los scripts auxiliares (`./advanced_system_monitor.sh`, etc.).

## Ejecucion recomendada

### 1) Modo interactivo (menu)

```bash
./main_monitor.sh
```

Opciones del menu:

- Iniciar/Detener daemon de monitoreo
- Ver estadisticas en tiempo real (recoleccion puntual + impresion en consola)
- Generar reportes
- Configurar umbrales
- Ver historial de alertas

### 2) Modo por argumentos

```bash
./main_monitor.sh --daemon   # inicia monitor continuo (cada 5 min)
./main_monitor.sh --report   # genera reportes
./main_monitor.sh --alert    # ejecuta una verificacion de alertas
./main_monitor.sh --config   # edita umbrales
```

## Flujo sugerido de uso

1. Inicializa configuracion y arranca el monitoreo:

```bash
./main_monitor.sh --config
./main_monitor.sh --daemon
```

2. Deja correr el daemon para acumular datos historicos.
	Mientras corre, tambien ejecuta automaticamente el chequeo de alertas en cada ciclo.
3. Ejecuta alertas cuando quieras validar estado actual:

```bash
./main_monitor.sh --alert
```

4. Genera reportes:

```bash
./main_monitor.sh --report
```

## Que trae el reporte hoy

El generador de reportes ya incluye:

- Uptime del sistema
- Carga promedio de 1, 5 y 15 minutos
- Pico de memoria (%) y su timestamp
- Top 5 procesos por CPU promedio (muestreo corto)
- Resumen de trafico de red total (RX y TX en MB)

Se exporta en:

- `daily_report.txt` (version legible en consola)
- `daily_report.csv` (resumen acumulado)
- `daily_report.html` (version visual)

## Archivos generados

Todos se guardan en:

```text
$HOME/system_monitor_logs
```

Archivos principales:

- `system_stats.csv`: historial de metricas recolectadas
- `monitor.conf`: umbrales configurables
- `alerts.log`: historial de alertas
- `daily_report.txt`: reporte en texto con secciones de sistema, memoria, red y top procesos
- `daily_report.csv`: resumen acumulado de reportes
- `daily_report.html`: reporte visual en HTML
- `network_errors.log`: errores de red detectados por el monitor avanzado

## Umbrales por defecto

Si no existe configuracion previa, se crea `monitor.conf` con:

- `RAM_THRESHOLD=90`
- `CPU_THRESHOLD=5`
- `DISK_THRESHOLD=85`

## Detener el daemon

Si esta activo, puedes detenerlo desde el menu (opcion 1) o ejecutando de nuevo la opcion de control del daemon en `main_monitor.sh`.

## Troubleshooting rapido

- Error con `mpstat`: instala `sysstat`.
- Error con `bc`: instala `bc`.
- No aparecen reportes: primero genera datos con el daemon o una recoleccion puntual.
- No se detecta interfaz de red: verifica que exista ruta por defecto (`ip route`).

## Nota practica

Si ejecutas el monitor avanzado sin `--daemon`, hace una recoleccion unica: guarda la fila en `system_stats.csv` y ademas imprime la muestra en consola (timestamp, CPU, RAM, RX y TX).
