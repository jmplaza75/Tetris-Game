# Arquitectura de la Fase 1

`main.c` inicializa SDL, crea el renderizador y controla el ciclo de vida.
Ante un fallo devuelve un código distinto de cero y muestra el error de SDL.
La salida normal y la salida por error destruyen primero el renderer, después
la ventana y finalmente los subsistemas SDL.

```text
main → input_process → game_update (paso fijo) → renderer_draw
            ↓                ↓                        ↓
          Game         C sin dependencia SDL       ventana SDL2
```

`Game` contiene únicamente la condición de ejecución en esta fase. Su función
de actualización es el punto de entrada para la futura lógica. No se crean
módulos vacíos para reglas que aún no existen.

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
