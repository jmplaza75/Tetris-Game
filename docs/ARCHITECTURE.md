# Arquitectura de la Fase 2

`main.c` inicializa SDL, crea el renderizador y controla el ciclo de vida.
Ante un fallo devuelve un código distinto de cero y muestra el error de SDL.
La salida normal y la salida por error destruyen primero el renderer, después
la ventana y finalmente los subsistemas SDL.

```text
main → input_process → game_update (paso fijo) → renderer_draw
            ↓                ↓                        ↓
          Game         C sin dependencia SDL       ventana SDL2
```

`Game` contiene la condición de ejecución, el tablero y la pieza activa.
Su función de actualización es el punto de entrada para la futura lógica.
La pieza activa se mantiene separada de las celdas fijadas en el tablero.

`Board` almacena 24 filas mediante `uint16_t`: los diez bits inferiores
representan las columnas de izquierda a derecha. Las filas 0–3 están ocultas;
el renderizador muestra las filas 4–23. `board_is_occupied` devuelve false
fuera del tablero; no es una función de colisión ni autoriza movimientos.

`Piece` almacena tipo, posición, orientación y una máscara de 16 bits.
El bit `y * 4 + x` representa la celda local `(x, y)`, con origen arriba a
la izquierda. Se incluyen las siete formas en orientación inicial; los
estados restantes y SRS se implementarán en la Fase 5. `piece_spawn` valida
el tipo y restablece la pieza sin modificar el tablero ni comprobar colisiones.
La posición inicial es `(3, 4)` para mostrar la forma completa en esta fase
sin gravedad. La futura lógica podrá usar las filas ocultas para el spawn.

El renderizador centra una cuadrícula de 300 × 600 con celdas de 30 píxeles,
omite las filas ocultas y recorta las celdas que quedan fuera del área visible.
Los siete colores se definen en una única tabla. Las celdas fijadas se muestran
en gris: guardar sus colores se resolverá al implementar el bloqueo de piezas.

`input.c` consume la cola de eventos en cada frame y solicita la salida al
recibir Esc o SDL_QUIT. `renderer.c` es propietario de la ventana y del renderer,
usa un color de fondo centralizado y un espacio lógico de 800 × 720.
SDL adapta este espacio al tamaño de ventana y a pantallas de alta densidad.

El reloj de alta resolución alimenta un acumulador en segundos. Cada actualización
consume 1/60 s independientemente de la frecuencia del renderizado. Se limita
el tiempo acumulado por frame a 0,25 s para evitar largas recuperaciones tras
suspender el proceso. No se usa la repetición de teclado del sistema.

Se solicita renderizado acelerado con VSync y se permite software si no está
disponible. Un descanso al final del frame limita el consumo cuando VSync
no espera o la pantalla supera 60 Hz. La lógica no depende de ese descanso.

No hay asignaciones dinámicas propias dentro del bucle. Los recursos SDL se
gestionan explícitamente. `--smoke-test` ejecuta tres frames y termina usando
la misma ruta de inicialización, renderizado y limpieza que la ejecución normal.
