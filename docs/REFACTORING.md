# Plan de refactorización

Documento de trabajo con mejoras de arquitectura y buenas prácticas, priorizadas por impacto y esfuerzo.

---

## Hecho

- **terrain ↔ game:** Eliminado `#include "game.h"` de `terrain.h`. La clase `terrain` no usaba nada de `game`; el include era innecesario y acoplaba en compilación. Los `.cpp` que usan `terrain` (p. ej. `game.cpp`) siguen incluyendo lo que necesitan.

- **ptrlist/ptrvector → std::list/vector<unique_ptr>:** Migrado completamente `game.h` y archivos relacionados (13 archivos totales) de las clases personalizadas `ptrvector<T>` y `ptrlist<T>` a `std::vector<std::unique_ptr<T>>` y `std::list<std::unique_ptr<T>>`. Eliminado gestión manual de memoria, mejorado RAII, actualizado ~50 ubicaciones con `.get()` donde se necesitan raw pointers. Las funciones `spawn_*` ahora reciben `std::unique_ptr` con semántica de movimiento. La función `cleanup()` usa el idioma erase-remove moderno con lambdas. (Completado: 2026-03-01)

- **Extraer World de game:** Creada clase `world` que encapsula todas las entidades del juego (ships, submarines, airplanes, torpedoes, etc.) y las funciones de consulta espacial (visible_*, sonar_*, radar_*). La clase `game` ahora delega la gestión de entidades a `world`, reduciendo significativamente el acoplamiento y mejorando la separación de responsabilidades. Archivos: world.h (147 líneas), world.cpp (300+ líneas). Beneficios: mejor testabilidad, código más organizado, facilita futuras mejoras como sistemas de consulta espacial optimizados (quad-trees, spatial hashing). (Completado: 2026-03-01)

- **Inyección de dependencias gradual (game, user_interface):** Iniciada la migración de singletons a inyección de dependencias por constructor. Las clases `game` y `user_interface` ahora reciben referencias a `cfg&`, `log&` y `music&` por parámetro en lugar de usar `::instance()`. Los constructores públicos de `game` ahora requieren `cfg&` y `log&` explícitamente. El constructor privado (usado por `game_editor`) sigue usando `::instance()` para mantener compatibilidad. `user_interface` recibe `cfg&` y `music&` para evitar acoplamiento directo. Beneficios: mejor testabilidad (permite inyectar mocks), desacoplamiento explícito, facilita razonamiento sobre dependencias. Archivos modificados: game.h/cpp, game_editor.h/cpp, user_interface.h/cpp, subsim.cpp (4 call sites). (Completado: 2026-03-02)

- **objcache con RAII:** Mejorada la clase `objcachet<T>::reference` para soportar construcción por defecto, move semantics y método `load()` para carga diferida. Migrados casos críticos de uso manual (`ref()`/`unref()`) a RAII: `freeview_display` ahora usa `objcachet<model>::reference` para `conning_tower` y `objcachet<texture>::reference` para `underwater_background` y `splashring`, eliminando llamadas manuales a `unref()` en el destructor. `sub_damage_display` y `sub_torpedo_display` ya usaban el patrón RAII para `notepadsheet`. Beneficios: elimina fugas de memoria si hay excepciones durante construcción, código más seguro y limpio. Archivos: objcache.h (mejorado), freeview_display.h/cpp (migrado). Pendiente: migrar `widget` (patrón complejo con ref_all/unref_all), `sea_object::mymodel` (asignación diferida en constructores), `coastmap::props`. (Completado: 2026-03-02)

- **Reducir includes en headers pesados:** Optimizados `widget.h` (eliminados `system.h`, `texts.h`, `image.h` → forward declarations), `user_interface.h` (eliminado `sea_object.h` → forward declaration). Los `.cpp` que necesitan estas dependencias ahora las incluyen explícitamente (widget.cpp, sub_recogmanual_display.cpp). Beneficios: menor acoplamiento en compilación, tiempos de compilación más rápidos cuando se modifican estos headers. Archivos: widget.h, user_interface.h, widget.cpp, sub_recogmanual_display.cpp. (Completado: 2026-03-02)

---

## Prioridad alta (impacto en acoplamiento / compilación)

1. **Migración SDL2 → SDL3**
   - **Estado**: ⚠️ **Trabajo en progreso** en branch `feature/sdl3-migration` (2026-03-02).
   - **Progreso**: ~60% completado. CMakeLists.txt actualizado, includes migrados, constantes y eventos actualizados. Ver `docs/SDL3_MIGRATION.md` en el branch para detalles.
   - **Bloqueantes**: widget.h (dependencia SDL_Keysym), music.h (SDL3_mixer includes), constantes de teclas minúsculas.
   - **Decisión**: Migración pausada por complejidad. SDL3 aún está en desarrollo y requiere cambios extensivos en la lógica de eventos. **Recomendación actual**: Mantener SDL2 hasta que SDL3 madure y el ecosistema esté más estable.
   - **Branch**: `feature/sdl3-migration` disponible para continuar cuando SDL3 esté más maduro.
   - **Referencia**: Ver `docs/SDL3_MIGRATION.md` para progreso detallado y pasos siguientes.

---

## Prioridad media (responsabilidades y testabilidad)

2. **Extraer responsabilidades de `user_interface`**
   - Agrupa: pantallas, popups, panel, cielo, costa, geoclipmap, mensajes, pausa.
   - Opciones: composición por "subsistemas" (p. ej. SkyManager, CoastRenderer, MessageQueue) inyectados o creados en el constructor, en lugar de una clase que hace todo.

3. **Singletons → inyección (continuar)**
   - ✅ **INICIADO**: `game` y `user_interface` ahora usan inyección de dependencias para `cfg`, `log` y `music`. Ver sección "Hecho" para detalles.
   - **Pendiente**: Aplicar el mismo patrón a otras clases donde sea beneficioso (displays, popups, sea_object y subclases).
   - Estrategia: En código nuevo, preferir recibir `cfg&` o `log&` por parámetro/constructor donde sea posible; en código existente, ir sustituyendo acceso a `singleton<T>::instance()` por parámetros en funciones clave (sin cambiar toda la base de una vez).
   - Singletons restantes: `system`, `global_data`, `postprocessor`, `data_file_handler` (estos pueden mantenerse por ahora).

---

## Prioridad baja (limpieza y consistencia)

4. **Código duplicado en `sub_*_display`**
   - Varias pantallas repiten patrones de dibujo/input. Revisar helpers en `user_display` (`rotat_tex`, `fix_tex`) y extraer más helpers comunes para reducir duplicación.

5. **Const-correctness**
   - Revisar firmas de métodos que no modifican estado y marcar como `const`; parámetros que no se modifican como `const &`.

6. **Naming**
   - El proyecto mezcla estilos (snake_case en nombres de clase). No cambiar todo de golpe; en código nuevo seguir un estilo coherente (p. ej. el ya usado en `sub_*_display`, `user_display`).

---

## No recomendado (por ahora)

- **OpenGL → Vulkan:** Reescribir todo el pipeline de render; esfuerzo muy alto.
- **Refactor masivo de `game` en un solo paso:** Hacerlo por pasos pequeños (extraer una responsabilidad, compilar y pasar tests, repetir).

---

## Cómo usar este documento

- Al tocar un área (terrain, game, UI, objcache), consultar aquí si hay un ítem aplicable.
- Completar un ítem → marcarlo o moverlo a "Hecho" y actualizar la fecha.
- Nuevas ideas de refactor → añadirlas con prioridad (alta/media/baja).
