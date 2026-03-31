# Laboratorio 2: Ejecución de versiones en C y Go

El Laboratorio 2 presenta implementaciones equivalentes en los lenguajes C y Go para la resolución de tareas similares. A continuación se describen los procedimientos para compilar y ejecutar ambas versiones. La idea es practicar el uso de punteros para la manipulación de strings mediante modificaciones in-place.

## Estructura de carpetas

- `Lab2/c/`: Contiene el código fuente en C.
- `Lab2/go/`: Contiene el código fuente en Go.

---

## Versión en C

Para compilar la versión en C, se debe acceder a la carpeta `Lab2/c/` y ejecutar el siguiente comando:

```bash
cd Lab2/c
gcc -o main main.c task1.c task2.c task3.c validations.c
```

El proceso de compilación genera el ejecutable denominado `main`. La ejecución del programa se realiza proporcionando el texto a procesar como argumento:

```bash
./main "texto a procesar"
```

Ejemplo de uso:

```bash
./main "árbol"
```

---

## Versión en Go

Para compilar la versión en Go, se debe acceder a la carpeta `Lab2/go/` y ejecutar el siguiente comando:

```bash
cd Lab2/go
go build -o main main.go task1.go task2.go task3.go validations.go
```

El proceso de compilación genera el ejecutable denominado `main`. La ejecución del programa se realiza proporcionando el texto a procesar como argumento:

```bash
./main "texto a procesar"
```

Ejemplo de uso:

```bash
./main "árbol"
```

---

## Notas

- Ambas versiones reciben el texto a procesar como argumento en la línea de comandos.
- Es necesario contar con GCC instalado para compilar la versión en C y con Go instalado para la versión en Go.
- En caso de inconvenientes con caracteres acentuados, se recomienda verificar que la terminal utilizada soporte UTF-8.
