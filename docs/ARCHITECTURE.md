# Arquitectura de la Fase 7

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
Su función de actualización aplica repetición horizontal y gravedad mediante
temporizadores en segundos; todos se reinicializan en `game_init`.
La pieza activa se mantiene separada de las celdas fijadas en el tablero.

`Board` almacena 24 filas mediante `uint16_t`: los diez bits inferiores
representan las columnas de izquierda a derecha. Las filas 0–3 están ocultas;
el renderizador muestra las filas 4–23. `board_is_occupied` devuelve false
fuera del tablero; no es una función de colisión ni autoriza movimientos.

`Piece` almacena tipo, posición, orientación y una máscara de 16 bits.
El bit `y * 4 + x` representa la celda local `(x, y)`, con origen arriba a
la izquierda. Se incluyen las siete formas y cuatro orientaciones. `rotation_try` rota
en una caja de 3 × 3 para J/L/S/T/Z y de 4 × 4 para I, usando los centros SRS.
La O conserva posición y forma. Se prueban cinco posiciones, con una tabla
de kicks propia para I y otra compartida para las demás; un fallo conserva
el estado original. Referencia: [SRS](https://tetris.wiki/Super_Rotation_System). `piece_spawn` valida
el tipo y restablece la pieza sin modificar el tablero ni comprobar colisiones.
La posición inicial es `(3, 4)` para mostrar la forma completa desde el inicio.
La futura lógica podrá usar las filas ocultas para el spawn.

`collision_at` comprueba exclusivamente los cuatro bloques de la pieza, no
su caja de 4 × 4. Cada bloque debe estar dentro de las 10 × 24 celdas y no
superponerse a un bit ocupado. Así, una caja puede salir parcialmente del
tablero cuando las celdas que quedan fuera están vacías. La consulta no muta
el tablero ni la pieza; el movimiento solo se aplica si la consulta lo permite.

La gravedad empieza con un intervalo de un segundo por fila, conservando el
tiempo sobrante. Cuando encuentra un obstáculo descarta el tiempo pendiente.
Los intervalos no positivos o no finitos se ignoran; el bucle principal entrega
pasos de 1/60 s. Una llamada con un intervalo mayor se subdivide para ordenar
la caída, el contacto y el bloqueo sin cargar a la nueva pieza tiempo anterior.

Al estar apoyada, la pieza acumula 500 ms de lock delay. Un movimiento lateral o giro
válido reinicia ese plazo hasta 15 veces por pieza; uno rechazado no lo reinicia.
Si deja de estar apoyada se cancela el tiempo de contacto. `board_lock_piece`
valida todos los bloques antes de escribir, de modo que un fallo no modifica
parcialmente el tablero. Cada celda almacena además su tipo más uno en una
matriz de bytes para conservar el color; la ocupación sigue usando bitmasks.

`board_clear_lines` recorre las 24 filas de abajo arriba, omite las máscaras
completas y copia las restantes hacia abajo junto con sus colores. Vacía las
filas superiores sobrantes y devuelve el número eliminado, que `Game` acumula
en `lines_cleared`. Incluye filas ocultas y elimina también filas no contiguas.

Después se toma la primera pieza de la cola NEXT,
se reinician gravedad, lock delay y DAS, y se comprueba su posición inicial.
Si está ocupada, `STATE_GAME_OVER` congela el motor, libera las teclas y oculta
la pieza que no pudo aparecer. Los eventos de salida y el renderizado siguen
activos; el título de la ventana indica el bloqueo. La UI y reinicio completos
se añadirán en la Fase 8.

Cada pulsación horizontal mueve inmediatamente y espera 150 ms (DAS) antes
de repetir cada 40 ms (ARR). La última dirección pulsada tiene prioridad;
al soltarla, la otra dirección mantenida comienza de nuevo. Se ignora la
repetición de eventos de macOS. Perder el foco libera el estado de entrada,
sin pausar la gravedad. Los tiempos se configuran en `game.h`.

El renderizador centra una cuadrícula de 300 × 600 con celdas de 30 píxeles,
omite las filas ocultas y recorta las celdas que quedan fuera del área visible.
Los siete colores se definen en una única tabla. Las celdas fijadas usan el
tipo guardado en el tablero; el gris se reserva para ocupación sin metadatos
de color, por ejemplo en una prueba que establezca bits directamente.

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

La tecla ↓ mueve una fila al pulsar y activa un intervalo de caída de 1/30 s.
Pulsar o soltar reinicia el acumulador para evitar saltos por tiempo pendiente.
Soltar o perder el foco cancela la caída rápida. ↑/X y Z generan un giro por
pulsación, ignorando los eventos de repetición del sistema. La caída rápida
respeta colisiones y lock delay y suma un punto por cada fila recorrida.

## Randomizador y cola

`Randomizer` pertenece a cada partida y genera bolsas con I/J/L/O/S/T/Z.
Fisher-Yates las mezcla con SplitMix64 y selección acotada por rechazo para
no introducir sesgo de módulo. No se usa `rand()` ni estado global. La cola
contiene siempre cinco tipos: al extraer uno se desplazan los restantes y
se repone el último desde la bolsa, sin perder piezas al cambiar de bolsa.
`game_init_seed` permite pruebas reproducibles; `game_init` usa semilla 1,
mientras la aplicación aporta el contador de alta resolución de SDL.

## Hold y previsualizaciones

`held_piece == PIECE_COUNT` indica hold vacío. El primer C guarda la pieza
activa y consume NEXT; los intercambios posteriores no consumen la cola.
`hold_used` impide repetir hasta el siguiente bloqueo. La pieza entrante
recupera posición y orientación iniciales; se reinician gravedad, lock delay,
contador de reinicios y DAS. El spawn ocupado usa la misma salida de game over.

`preview.c` dibuja HOLD y las cinco próximas piezas con la paleta compartida,
centrando sus bloques ocupados. HOLD se atenúa cuando no está disponible.
Las etiquetas utilizan pequeños glifos propios sin depender de SDL_ttf ni
archivos de fuentes. El layout comparte medidas con el tablero y escala con
el espacio lógico de SDL. No se asigna memoria dinámica durante el juego.

## Ghost, hard drop y puntuación

`game_ghost_piece` copia la pieza activa y busca su última posición válida
mediante consultas de colisión. No modifica el tablero, los temporizadores,
la cola ni el randomizador. El renderer dibuja su contorno antes de la pieza
activa, que lo cubre cuando ambas coinciden. No se muestra tras game over.

Espacio llama a `game_hard_drop` una vez por pulsación, ignorando key repeat.
Usa la misma consulta que el ghost, suma dos puntos por fila descendida y
bloquea inmediatamente, incluso si la distancia es cero. Reutiliza la ruta
de limpieza, avance de NEXT, habilitación de hold y comprobación de spawn.

`scoring.c` concentra las reglas sin depender de SDL. `Game.score` es de
64 bits. Single/double/triple/tetris dan 100/300/500/800 puntos multiplicados
por el nivel vigente antes de limpiar. Después se acumulan las líneas y se
calcula `level = 1 + lines_cleared / 10`. No hay bonos avanzados en esta fase.
La gravedad usa `max(1/60, 0.8^(level-1))` segundos por fila; el cálculo está
acotado al alcanzar el mínimo. Soft drop usa el menor intervalo entre esa
gravedad y 1/30 s. Solo las filas recorridas con soft drop activo suman un
punto; la gravedad normal y los intentos bloqueados no puntúan.

`preview.c` reutiliza sus glifos para mostrar SCORE, LINES y LEVEL, con cifras
que reducen su escala si no caben. Los tests verifican aterrizaje de todas las
piezas y orientaciones sobre obstáculos, ausencia de mutación, bloqueo inmediato,
puntuación de 1–4 líneas, cruce de nivel, caídas, gravedad y eventos de Espacio.
