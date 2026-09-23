# Tetris — C / Apple Silicon

Written from scratch in C. Targeted at Apple Silicon ARM64.

Implementación incremental de Tetris en C17 y SDL2. **Fase 2 completada:**
ventana redimensionable de 800 × 720, bucle con actualización fija a 60 Hz,
tablero de 10 × 24 (20 filas visibles y 4 ocultas) y representación mediante
bitmasks de los siete tetrominós. Al arrancar aparece una T morada sobre la
cuadrícula centrada. La pieza permanece inmóvil hasta implementar la Fase 3.

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
make test        # eventos de salida y arranque SDL2 sin pantalla
make sanitize    # las mismas comprobaciones con ASan y UBSan
make clean
```

## Controles actuales

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

Consulta [la arquitectura](docs/ARCHITECTURE.md). No hay todavía movimiento,
gravedad, colisiones, rotaciones, bloqueo, audio, benchmarks ni rutinas Assembly.

## Verificación manual

Ejecuta `make run`, comprueba la cuadrícula de 10 × 20 y los cuatro bloques
de la T morada en la parte superior, cambia el tamaño de la ventana y
ciérrala con Esc. Vuelve a abrirla y comprueba el botón de cierre.
`make test` prueba el tablero, las siete formas, el spawn, los eventos y
tres frames con el controlador SDL dummy;
esa prueba no sustituye la comprobación visual de la ventana nativa.

Validación de la Fase 2: debug y release compilados para ARM64 sin warnings;
`make test`, el arranque nativo con `--smoke-test` y la inspección del
renderizado completados correctamente. Las pruebas del motor sin SDL también
pasan con ASan/UBSan (`make MODE=sanitize build/sanitize/test_engine` y
`./build/sanitize/test_engine`).
La ejecución completa con ASan/UBSan no se ha podido validar en este entorno: la SDL2
instalada aborta o queda bloqueada en su inicializador `dllinit`, antes de
entrar en `main`. El objetivo `make sanitize` está disponible, pero esta
comprobación queda pendiente en un entorno compatible.
