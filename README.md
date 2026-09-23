# Tetris — C / Apple Silicon

Written from scratch in C. Targeted at Apple Silicon ARM64.

Implementación incremental de Tetris en C17 y SDL2. **Fase 5 y caída rápida implementadas:**
ventana redimensionable de 800 × 720, bucle con actualización fija a 60 Hz,
tablero de 10 × 24 (20 filas visibles y 4 ocultas) y representación mediante
bitmasks de los siete tetrominós. Al arrancar aparece una T morada sobre la
cuadrícula centrada. Se mueve con las flechas izquierda/derecha y cae una
fila por segundo, respetando los límites y las celdas ocupadas.
Tras tocar el suelo o una pieza, hay 500 ms para ajustar la posición antes
del bloqueo. Los bloques conservan su color, las filas completas desaparecen
y las superiores bajan. A continuación aparece otra pieza.

La secuencia provisional es T → Z → I → J → L → O → S, repetida;
el randomizador 7-bag llegará en la Fase 6. Si una nueva pieza no cabe,
se detiene la partida y se avisa en el título de la ventana. Por ahora hay
que salir y volver a abrirla para reiniciar. La interfaz de game over y el
reinicio corresponden a la Fase 8; puntuación y niveles, a la Fase 7.

## Compilar y ejecutar

Requisitos: macOS Apple Silicon, herramientas de desarrollo de Xcode (`xcode-select --install`)
y SDL2 (`brew install sdl2`).

```sh
make
make run
```

`make` compila en modo debug con Clang, C17 y `-arch arm64` en macOS.
SDL2 se busca en `/opt/homebrew` y después en el PATH.
Se puede configurar con `make SDL_PREFIX=/otra/ruta` o
`make SDL_CONFIG=/ruta/a/sdl2-config`.

```sh
make debug       # -O0 -g, warnings tratados como errores
make release     # -O2; salida independiente en build/release/
make MODE=release run
make test        # motor, colisiones, movimiento, eventos y arranque SDL2
make sanitize    # las mismas comprobaciones con ASan y UBSan
make clean
```

## Controles actuales

- **↑ / X**: rotación horaria.
- **Z**: rotación antihoraria.
- **↓ mantenida**: caída rápida a 30 filas por segundo; al soltar vuelve a la gravedad normal.

Las rotaciones usan SRS, con ajustes junto a paredes y suelo. La pieza O mantiene su forma.

Pulsa **← / →** para mover; mantenlas para repetir (DAS 150 ms, ARR 40 ms).
Si mantienes ambas, tiene prioridad la última pulsada; al soltarla se retoma
la otra con un movimiento inmediato y un nuevo DAS. Al perder el foco se
liberan ambas teclas; la gravedad continúa. Los tiempos están en `include/game.h`.

Pulsa **Esc** o cierra la ventana para salir. Puedes redimensionarla;
el renderizador conserva la proporción del espacio lógico de 800 × 720.

## Estructura

```text
src/        Aplicación, estado, eventos y renderizado
include/    Interfaces de los módulos
tests/      Comprobaciones del ciclo de vida y eventos
docs/       Arquitectura de la fase actual
assets/     Reservado para audio y fuentes opcionales
asm/        Reservado para experimentos ARM64 de fases posteriores
build/      Binarios y objetos separados por configuración (ignorado)
```

Consulta [la arquitectura](docs/ARCHITECTURE.md). No hay todavía hard drop, audio, benchmarks ni rutinas Assembly.

## Verificación manual

Ejecuta `make run`, comprueba la cuadrícula de 10 × 20 y los cuatro bloques
de la T morada en la parte superior. Comprueba su caída, pulsa y mantén
las flechas hasta ambos laterales y espera a que se fije en el suelo y
aparezca una Z. Completa una fila para verla desaparecer; los bloques que
estaban encima deben descender manteniendo su color.
Cambia de aplicación mientras mantienes una flecha: al volver no debe
seguir moviéndose lateralmente. Cambia el tamaño de la ventana y
ciérrala con Esc. Vuelve a abrirla y comprueba el botón de cierre.
`make test` prueba el tablero, las siete formas, el spawn, colisiones,
gravedad, DAS/ARR, bloqueo, limpieza de filas y colores, spawn bloqueado,
liberación de teclas, eventos y
tres frames con el controlador SDL dummy;
esa prueba no sustituye la comprobación visual de la ventana nativa.

Validación de rotación y caída rápida: debug y release compilados para ARM64 sin warnings;
`make test` completado correctamente. Las pruebas del motor sin SDL también
pasan con ASan/UBSan (`make MODE=sanitize build/sanitize/test_engine` y
`./build/sanitize/test_engine`).
La ejecución completa con ASan/UBSan no se ha podido validar en este entorno: la SDL2
instalada aborta o queda bloqueada en su inicializador `dllinit`, antes de
entrar en `main`. El objetivo `make sanitize` está disponible, pero esta
comprobación queda pendiente en un entorno compatible.
