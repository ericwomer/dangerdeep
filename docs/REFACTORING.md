# Plan de refactorización

Documento de trabajo con mejoras de arquitectura y buenas prácticas, priorizadas por impacto y esfuerzo.

---

## Hecho

- **terrain ↔ game:** Eliminado `#include "game.h"` de `terrain.h`. La clase `terrain` no usaba nada de `game`; el include era innecesario y acoplaba en compilación. Los `.cpp` que usan `terrain` (p. ej. `game.cpp`) siguen incluyendo lo que necesitan.

- **ptrlist/ptrvector → std::list/vector<unique_ptr>:** Migrado completamente `game.h` y archivos relacionados (13 archivos totales) de las clases personalizadas `ptrvector<T>` y `ptrlist<T>` a `std::vector<std::unique_ptr<T>>` y `std::list<std::unique_ptr<T>>`. Eliminado gestión manual de memoria, mejorado RAII, actualizado ~50 ubicaciones con `.get()` donde se necesitan raw pointers. Las funciones `spawn_*` ahora reciben `std::unique_ptr` con semántica de movimiento. La función `cleanup()` usa el idioma erase-remove moderno con lambdas. (Completado: 2026-03-01)

---

## Prioridad alta (impacto en acoplamiento / compilación)

1. **Reducir includes en headers “pesados”**
   - **widget.h:** Depende de system, model, image, objcache. Valorar forward declarations para model/system donde solo se usen punteros o referencias.
   - **sea_object.h:** Muchos includes del proyecto. Introducir forwards donde el tipo solo se use como puntero/ref.
   - **user_interface.h:** Depende de caustics, coastmap, geoclipmap, sea_object. Valorar interfaz mínima (p. ej. “terrain provider”) en lugar de incluir todo el render.

2. **objcache y RAII**
   - `objcache` devuelve punteros raw y usa refcount manual; el código ya comenta la necesidad de un “reference handler”.
   - Objetivo: envolver usos en algo tipo `std::shared_ptr` o un handle con destructor que decremente refcount, para evitar fugas si hay excepciones.

---

## Prioridad media (responsabilidades y testabilidad)

3. **Extraer responsabilidades de `game`**
   - `game` concentra: simulación, entidades, colisiones, sonar, radar, visibilidad, spawn, jobs, red, agua/terreno, guardado.
   - Opciones:
     - Extraer un **World** o **Simulation**: lista de entidades, tiempo, colisiones; `game` lo tendría como miembro y delegaría.
     - Extraer **Sonar/Radar** a un módulo que reciba el mundo (o una interfaz de “objetos visibles”) y no al revés.
   - El comentario en `game.h` (“do NOT include user_interface”) ya evita acoplamiento directo; mantener esa regla.

4. **Extraer responsabilidades de `user_interface`**
   - Agrupa: pantallas, popups, panel, cielo, costa, geoclipmap, mensajes, pausa.
   - Opciones: composición por “subsistemas” (p. ej. SkyManager, CoastRenderer, MessageQueue) inyectados o creados en el constructor, en lugar de una clase que hace todo.

5. **Singletons → inyección (gradual)**
   - Singletons actuales: `system`, `global_data`, `cfg`, `log`, `postprocessor`, `music`, `data_file_handler`.
   - Para mejorar testabilidad: en código nuevo, preferir recibir `cfg&` o `log&` por parámetro/constructor donde sea posible; en código existente, ir sustituyendo acceso a `singleton<T>::instance()` por parámetros en funciones clave (sin cambiar toda la base de una vez).

---

## Prioridad baja (limpieza y consistencia)

6. **Código duplicado en `sub_*_display`**
   - Varias pantallas repiten patrones de dibujo/input. Revisar helpers en `user_display` (`rotat_tex`, `fix_tex`) y extraer más helpers comunes para reducir duplicación.

7. **Const-correctness**
   - Revisar firmas de métodos que no modifican estado y marcar como `const`; parámetros que no se modifican como `const &`.

8. **Naming**
   - El proyecto mezcla estilos (snake_case en nombres de clase). No cambiar todo de golpe; en código nuevo seguir un estilo coherente (p. ej. el ya usado en `sub_*_display`, `user_display`).

---

## No recomendado (por ahora)

- **OpenGL → Vulkan:** Reescribir todo el pipeline de render; esfuerzo muy alto.
- **Refactor masivo de `game` en un solo paso:** Hacerlo por pasos pequeños (extraer una responsabilidad, compilar y pasar tests, repetir).

---

## Cómo usar este documento

- Al tocar un área (terrain, game, UI, objcache), consultar aquí si hay un ítem aplicable.
- Completar un ítem → marcarlo o moverlo a “Hecho” y actualizar la fecha.
- Nuevas ideas de refactor → añadirlas con prioridad (alta/media/baja).
