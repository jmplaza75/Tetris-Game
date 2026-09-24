# Tetris — C / Apple Silicon

Written from scratch in C. Targeted at Apple Silicon ARM64.

Implementación incremental de Tetris en C17 y SDL2. **Fase 8 completada:**
ventana redimensionable de 800 × 720, bucle con actualización fija a 60 Hz,
tablero de 10 × 24 (20 filas visibles y 4 ocultas) y representación mediante
bitmasks de los siete tetrominós. Al arrancar aparece una pieza aleatoria sobre la
cuadrícula centrada. Se mueve con las flechas izquierda/derecha y cae una
fila por segundo en el nivel inicial, respetando los límites y las celdas ocupadas.
Tras tocar el suelo o una pieza, hay 500 ms para ajustar la posición antes
del bloqueo. Los bloques conservan su color, las filas completas desaparecen
y las superiores bajan. A continuación aparece otra pieza.

El randomizador 7-bag mezcla las siete piezas sin repeticiones dentro de cada
bolsa. NEXT muestra las cinco siguientes en orden de arriba abajo. HOLD permite
guardar una pieza con C; se muestra atenuada cuando ya se ha usado hasta el
próximo bloqueo. Si una nueva pieza no cabe,
se detiene la partida y aparece GAME OVER sobre el tablero. Pulsa R para
iniciar otra partida sin cerrar la ventana. P pausa y reanuda el juego;
la pausa conserva el tiempo restante de gravedad y bloqueo. La interfaz
incluye título, controles, HOLD, NEXT, puntos, líneas y nivel.

La ghost piece muestra con un contorno el aterrizaje de la pieza actual, sin
modificar la partida. SCORE, LINES y LEVEL se muestran debajo de HOLD.
Limpiar 1/2/3/4 líneas otorga 100/300/500/800 puntos multiplicados por el
nivel anterior a la limpieza. Soft drop suma 1 punto por fila y hard drop 2;
la gravedad y los movimientos bloqueados no dan puntos.
El nivel empieza en 1 y sube cada 10 líneas. El intervalo de gravedad se
multiplica por 0,8 por nivel, hasta un mínimo de 1/60 s por fila.
Soft drop nunca ralentiza la gravedad de los niveles altos.

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

- **P**: pausar o reanudar.
- **R**: reiniciar desde juego, pausa o game over.
- **Espacio**: hard drop hasta la silueta y bloqueo inmediato.
- **C**: guardar o intercambiar pieza (una vez antes de cada bloqueo).
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

Consulta [la arquitectura](docs/ARCHITECTURE.md). No hay todavía audio, benchmarks ni rutinas Assembly.

## Verificación manual

Ejecuta `make run`, comprueba la cuadrícula de 10 × 20 y los cuatro bloques
de la pieza inicial en la parte superior. Comprueba su caída, pulsa y mantén
las flechas hasta ambos laterales y espera a que se fije en el suelo y
aparezca la primera pieza de NEXT. Pulsa C para guardar una pieza; otro C
antes de bloquearla no debe cambiar nada. Tras el bloqueo, C debe permitir
recuperarla con su orientación inicial sin consumir NEXT. Completa una fila para verla desaparecer; los bloques que
estaban encima deben descender manteniendo su color.
Comprueba que el contorno sigue los movimientos y giros. Pulsa Espacio:
la pieza debe fijarse en ese lugar y sumar dos puntos por fila descendida.
Mantener Espacio no debe soltar más piezas. Verifica los contadores del panel
y la aceleración al alcanzar diez líneas.
Pulsa P durante una caída y junto al suelo: espera unos segundos y reanuda
con P; la pieza debe conservar el tiempo pendiente. R debe vaciar el tablero,
poner los puntos y líneas a cero, volver al nivel 1 y renovar NEXT y HOLD.
Apila piezas hasta GAME OVER y comprueba que R inicia otra partida.
Cambia de aplicación mientras mantienes una flecha: al volver no debe
seguir moviéndose lateralmente. Cambia el tamaño de la ventana y
ciérrala con Esc. Vuelve a abrirla y comprueba el botón de cierre.
`make test` prueba el tablero, las siete formas, el spawn, colisiones,
gravedad, DAS/ARR, bloqueo, limpieza de filas y colores, spawn bloqueado,
7-bag (3200 bolsas), continuidad de NEXT, hold, ghost sin mutación, hard drop,
puntuación de líneas y caídas, niveles, pausa y conservación de temporizadores,
reinicio completo desde todos los estados, liberación de teclas, eventos y
tres frames con el controlador SDL dummy;
esa prueba no sustituye la comprobación visual de la ventana nativa.

Validación de la Fase 8: debug y release compilados para ARM64 sin warnings;
`make test` completado correctamente. Las pruebas del motor sin SDL también
pasan con ASan/UBSan (`make MODE=sanitize build/sanitize/test_engine` y
`./build/sanitize/test_engine`).
La ejecución completa con ASan/UBSan no se ha podido validar en este entorno: la SDL2
instalada aborta o queda bloqueada en su inicializador `dllinit`, antes de
entrar en `main`. El objetivo `make sanitize` está disponible, pero esta
comprobación queda pendiente en un entorno compatible.
