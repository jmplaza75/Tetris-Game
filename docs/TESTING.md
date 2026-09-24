# Pruebas de la Fase 9

Ejecuta `make test` desde la raíz para comprobar el motor, los eventos SDL,
el renderizador software y tres frames de la aplicación con vídeo dummy.
Los tests usan `assert` y fallan con código distinto de cero. El Makefile
aplica `-UNDEBUG` a los ejecutables de prueba para conservar las comprobaciones
incluso si se aporta `-DNDEBUG`. No se necesitan assets ni interacción manual.

## Comandos

```sh
make test
make test-engine
make test-sdl
make MODE=release test
make sanitize-engine
make sanitize
```

`test-engine` compila y enlaza únicamente C del motor; no consulta sdl2-config.
`test-sdl` requiere SDL2. El renderizador y la aplicación usan vídeo dummy para
funcionar sin una ventana visible. Las salidas se separan por configuración.

## Cobertura funcional

- `test_engine.c`: inicialización, tablero vacío, formas de las siete piezas,
  spawn y tipos inválidos; ejecuta las suites del motor.
- `test_motion.c`: límites, obstáculos, filas ocultas, coordenadas extremas,
  gravedad independiente del paso, tiempos inválidos y DAS/ARR.
- `test_lines.c`: eliminación de 1–4 filas, filas no contiguas, tablero lleno,
  conservación de colores, lock delay, spawn bloqueado y bloqueo atómico.
- `test_rotation.c`: cuatro giros en ambos sentidos, pieza O, kicks de T e I,
  paredes y suelo, rechazo sin mutación y caída rápida.
- `test_randomizer.c`: 3.200 bolsas sin duplicados, semillas reproducibles,
  continuidad de NEXT, hold y spawn bloqueado al intercambiar.
- `test_scoring.c`: ghost sin mutación, hard/soft drop, puntuación de líneas,
  cambio de nivel, intervalos de gravedad y rechazo tras game over/salida.
- `test_states.c`: pausa, temporizadores congelados, reanudación del bloqueo,
  reinicio completo desde todos los estados y salida irreversible.
- `test_properties.c`: 1.024 tableros deterministas contra un modelo matricial
  que elimina filas una a una; 12.180 posiciones que incluyen todas las piezas,
  cuatro giros, límites, filas ocultas y obstáculos. Verifica la ocupación,
  colores y ausencia de escrituras parciales cuando el bloqueo falla. También
  agota los 15 reinicios del lock delay mediante movimientos reales.
- `test_lifecycle.c`: eventos de movimiento, giro, hold, hard drop, foco,
  pausa, reinicio y salida, incluyendo repetición automática ignorada.
- `test_renderer.c`: píxeles del ghost y pieza activa, panel de pausa,
  ocultación de pieza en game over, ausencia de mutación del juego,
  puntuación de 64 bits, redimensionado y destrucción repetida.

Los modelos usan una secuencia fija para que cualquier fallo sea reproducible.
Estas cantidades describen casos ejecutados, no porcentajes de cobertura de
líneas ni una prueba exhaustiva de todas las partidas o tablas SRS.

## Resultado y límites de la validación

En macOS ARM64 pasan las suites completas debug y release y el motor con
AddressSanitizer/UndefinedBehaviorSanitizer. Se verificó una compilación nueva
sin SDL con `make BUILD_DIR=build/no-sdl SDL_CONFIG=/nonexistent test-engine`.

El intento de `make sanitize` de esta fase quedó bloqueado en el arranque de
`test_lifecycle`, con errores de servicios macOS (`_LSModifyNotification` y
`com.apple.hiservices-xpcservice`). Se terminó el grupo de procesos tras 25 s.
No se considera validada la integración SDL bajo sanitizadores en este equipo.

Los tests de píxeles y vídeo dummy no sustituyen una comprobación manual de
legibilidad, teclado físico, VSync, HiDPI o ventana nativa. Los pasos manuales
para jugar, pausar, reiniciar y cambiar de tamaño están en el README.
