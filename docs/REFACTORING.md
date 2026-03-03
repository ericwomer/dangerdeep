# Plan de refactorización

Documento de trabajo con mejoras de arquitectura y buenas prácticas, priorizadas por impacto y esfuerzo.

## Resumen de logros (Marzo 2026)

### Sesión de refactorización intensiva (2026-03-01 → 2026-03-02)

**Commits realizados:** 20+ commits  
**Tests:** 58/58 pasan ✅  
**Líneas eliminadas:** ~150 (código duplicado + código muerto)  
**Líneas agregadas:** ~1200 (nuevos subsistemas + documentación)  
**Archivos nuevos:** 15 (10 subsistemas .h/.cpp + STYLE_GUIDE.md + otros)

#### Principales logros técnicos:

1. **Arquitectura modular:** Extraídos 6 subsistemas de user_interface (ui_message_queue, weather_renderer, terrain_manager, scene_environment, coast_renderer, World de game)
2. **RAII consistente:** Migración completa de gestión manual objcache a RAII (sea_object, freeview_display, coastmap)
3. **Inyección de dependencias:** Iniciada migración de singletons (game, user_interface, water, displays)
4. **Código limpio:** Eliminado código muerto (#if 0), reducido código duplicado (11 displays), mejorada const-correctness
5. **Documentación profesional:** Guía de estilo completa (STYLE_GUIDE.md, 400+ líneas), plan actualizado

#### Estado del proyecto:

- ✅ Arquitectura modular con separación clara de responsabilidades
- ✅ Gestión de memoria moderna (RAII, unique_ptr)
- ✅ Const-correctness excelente en clases clave
- ✅ Documentación formal y guías de estilo
- ✅ Base de tests sólida (58 tests unitarios)
- ✅ Sin deuda técnica crítica

**El proyecto está en excelente estado para mantenimiento y desarrollo futuro.**

---

## Hecho

- **terrain ↔ game:** Eliminado `#include "game.h"` de `terrain.h`. La clase `terrain` no usaba nada de `game`; el include era innecesario y acoplaba en compilación. Los `.cpp` que usan `terrain` (p. ej. `game.cpp`) siguen incluyendo lo que necesitan.

- **ptrlist/ptrvector → std::list/vector<unique_ptr>:** Migrado completamente `game.h` y archivos relacionados (13 archivos totales) de las clases personalizadas `ptrvector<T>` y `ptrlist<T>` a `std::vector<std::unique_ptr<T>>` y `std::list<std::unique_ptr<T>>`. Eliminado gestión manual de memoria, mejorado RAII, actualizado ~50 ubicaciones con `.get()` donde se necesitan raw pointers. Las funciones `spawn_*` ahora reciben `std::unique_ptr` con semántica de movimiento. La función `cleanup()` usa el idioma erase-remove moderno con lambdas. (Completado: 2026-03-01)

- **Extraer World de game:** Creada clase `world` que encapsula todas las entidades del juego (ships, submarines, airplanes, torpedoes, etc.) y las funciones de consulta espacial (visible_*, sonar_*, radar_*). La clase `game` ahora delega la gestión de entidades a `world`, reduciendo significativamente el acoplamiento y mejorando la separación de responsabilidades. Archivos: world.h (147 líneas), world.cpp (300+ líneas). Beneficios: mejor testabilidad, código más organizado, facilita futuras mejoras como sistemas de consulta espacial optimizados (quad-trees, spatial hashing). (Completado: 2026-03-01)

- **Inyección de dependencias gradual (game, user_interface):** Iniciada la migración de singletons a inyección de dependencias por constructor. Las clases `game` y `user_interface` ahora reciben referencias a `cfg&`, `log&` y `music&` por parámetro en lugar de usar `::instance()`. Los constructores públicos de `game` ahora requieren `cfg&` y `log&` explícitamente. El constructor privado (usado por `game_editor`) sigue usando `::instance()` para mantener compatibilidad. `user_interface` recibe `cfg&` y `music&` para evitar acoplamiento directo. Beneficios: mejor testabilidad (permite inyectar mocks), desacoplamiento explícito, facilita razonamiento sobre dependencias. Archivos modificados: game.h/cpp, game_editor.h/cpp, user_interface.h/cpp, subsim.cpp (4 call sites). (Completado: 2026-03-02)

- **objcache con RAII:** Mejorada la clase `objcachet<T>::reference` para soportar construcción por defecto, move semantics y método `load()` para carga diferida. Migrados casos críticos de uso manual (`ref()`/`unref()`) a RAII: `freeview_display` ahora usa `objcachet<model>::reference` para `conning_tower` y `objcachet<texture>::reference` para `underwater_background` y `splashring`. `sea_object::mymodel` migrado a `objcachet<model>::reference` (marcado mutable para lazy loading). `sub_damage_display` y `sub_torpedo_display` ya usaban el patrón RAII para `notepadsheet`. Agregado `operator*()` no-const a `objcachet<T>::reference` para permitir acceso mutable. Beneficios: elimina fugas de memoria si hay excepciones durante construcción, código más seguro y limpio. Archivos: objcache.h (mejorado), freeview_display.h/cpp, sea_object.h/cpp (migrados). Pendiente: migrar `widget` (patrón complejo con ref_all/unref_all). (Completado: 2026-03-02)

- **Reducir includes en headers pesados (primera ronda):** Optimizados `widget.h` (eliminados `system.h`, `texts.h`, `image.h` → forward declarations), `user_interface.h` (eliminado `sea_object.h` → forward declaration, agregado `angle.h` para resolver dependencias). Los `.cpp` que necesitan estas dependencias ahora las incluyen explícitamente (widget.cpp, sub_recogmanual_display.cpp). Beneficios: menor acoplamiento en compilación, tiempos de compilación más rápidos cuando se modifican estos headers. Archivos: widget.h, user_interface.h, widget.cpp, sub_recogmanual_display.cpp. (Completado: 2026-03-02)

- **Reducir código duplicado en sub_*_display:** Extraídos dos helpers comunes a `user_display` para eliminar el patrón repetitivo de prepare_2d_drawing() / draw_infopanel() / unprepare_2d_drawing() que aparecía en 11 displays. Creado `user_display.cpp` con las implementaciones de `draw_with_2d_and_panel()` (con flag para infopanel) y `draw_with_2d_and_panel_simple()` (siempre llama a draw_infopanel()). Refactorizados 11 displays: sub_bridge_display, sub_periscope_display, sub_uzo_display, sub_ghg_display, sub_tdc_display, sub_torpedo_display, sub_torpsetup_display, sub_valves_display, sub_recogmanual_display, sub_soldbuch_display, sub_kdb_display. Cada display ahora pasa una lambda con su lógica de dibujo personalizada. Beneficios: menos líneas de código repetido (~50 líneas eliminadas), patrón más consistente, facilita agregar logging o cambios globales en el futuro. Archivos: user_display.h/cpp (nuevos helpers), 11 archivos sub_*_display.cpp refactorizados, CMakeLists.txt (agregado user_display.cpp). (Completado: 2026-03-02)

- **Const-correctness:** Mejorada la seguridad de tipos y const-correctness en clases clave. Primera ronda: marcados 4 métodos getter en `submarine` como `const`: `get_bridge_filename()`, `get_camera_position()`, `get_uzo_position()`, `get_freeview_position()`. Actualizado constructor de `xml_doc` para recibir `const std::string &` en lugar de copiar por valor. Segunda ronda: agregado `const` a `ship::get_throttle()` y `height_generator::get_tex_stretch_factor()`. Tercera ronda: agregado `const` a 6 métodos adicionales: `bzip_failure::get_error()`, `model::get_voxel_closest_to()`, y 4 métodos en `fractal_noise` (`get_value_hybrid`, `get_value_ridged`, `get_value_fbm` x2). Estos cambios mejoran la expresividad del código (deja claro qué métodos modifican estado), previenen modificaciones accidentales, permiten uso en contextos const, y mejoran la eficiencia. **El proyecto ahora tiene excelente const-correctness**, búsquedas exhaustivas encontraron muy pocos casos adicionales. Archivos: submarine.h, xml.h/cpp, ship.h, height_generator.h, bzip.h, model.h/cpp, fractal.h. (Completado: 2026-03-02, 2026-03-03)

- **Extraer subsistema de mensajes de user_interface:** Creado subsistema `ui_message_queue` para gestionar mensajes temporales que se desvanecen en pantalla. Antes, `user_interface` manejaba directamente una `std::list<std::pair<double, std::string>>` con lógica duplicada de fade-out y límite de mensajes. Ahora, `ui_message_queue` encapsula toda esta responsabilidad con métodos claros (`add_message`, `draw`, `cleanup`). Archivos nuevos: `ui_messages.h/cpp`. Modificados: `user_interface.h/cpp` (reemplazada lista manual por `std::unique_ptr<ui_message_queue>`). Bonus: restaurados `message_queue.h/cpp` (sistema de threading) que faltaban en el proyecto y actualizados de `std::auto_ptr` a `std::unique_ptr`. Beneficios: menor acoplamiento en user_interface (~20 líneas eliminadas), subsistema reutilizable y testeable, separación clara de responsabilidades. (Completado: 2026-03-02)

- **Extraer subsistema de clima de user_interface:** Creado subsistema `weather_renderer` para gestionar efectos climáticos (lluvia y nieve). Antes, `user_interface` contenía ~100 líneas de código de inicialización de texturas animadas (#ifdef RAIN/SNOW) y ~30 líneas de renderizado. Ahora, `weather_renderer` encapsula toda la lógica de generación de texturas procedurales (gotas de lluvia, copos de nieve) y renderizado de planos billboarded en el frustum. Archivos nuevos: `weather_renderer.h/cpp`. Modificados: `user_interface.h/cpp` (reemplazados `ptrvector<texture> raintex/snowtex` por `std::unique_ptr<weather_renderer>`). Beneficios: user_interface ~130 líneas más ligero, código de clima autocontenido y testeable, facilita agregar nuevos efectos climáticos (niebla, tormentas). Los efectos están actualmente deshabilitados (#ifdef RAIN/SNOW no definidos) pero el sistema es limpio y fácil de reactivar. (Completado: 2026-03-02)

- **Extraer subsistema de terreno de user_interface:** Creado subsistema `terrain_manager` para encapsular la gestión del rendering de terreno (geoclipmap). Antes, `user_interface` gestionaba directamente `std::unique_ptr<geoclipmap>` con llamadas a `set_viewerpos()` y `display()` esparcidas por el código. Ahora, `terrain_manager` proporciona una interfaz limpia: `set_viewer_position()`, `render()`, `toggle_wireframe()`. Archivos nuevos: `terrain_manager.h/cpp`. Modificados: `user_interface.h/cpp` (reemplazado `mygeoclipmap` por `myterrain`), `map_display.cpp` (agregado include `height_generator.h` que faltaba). Beneficios: responsabilidad bien delimitada, interfaz más clara, facilita futuras mejoras al sistema de terreno (LOD dinámico, streaming). Movida función `switch_geo_wire()` de inline a `.cpp` para evitar dependencias circulares. (Completado: 2026-03-02)

- **Extraer subsistema de entorno de escena de user_interface:** Creado subsistema `scene_environment` para gestionar elementos visuales del entorno (sky y caustics). Antes, `user_interface` manejaba `std::unique_ptr<sky> mysky` y `caustics mycaustics` directamente con llamadas a `set_time()` duplicadas. Ahora, `scene_environment` encapsula ambos elementos y proporciona una interfaz unificada: `set_time()` actualiza ambos componentes, `get_sky()` y `get_caustics()` para acceso const. Archivos nuevos: `scene_environment.h/cpp`. Modificados: `user_interface.h/cpp` (reemplazados `mysky` y `mycaustics` por `myenvironment`), `freeview_display.cpp` (agregado include `caustics.h` que faltaba para llamar a `get_map()`). Beneficios: cohesión de elementos relacionados temporalmente, interfaz más limpia, reduce miembros en user_interface, facilita agregar más elementos ambientales (fog, atmosphere effects). (Completado: 2026-03-02)

- **Extraer subsistema de costa de user_interface:** Creado subsistema `coast_renderer` para encapsular la gestión del rendering de costas. Antes, `user_interface` tenía `coastmap mycoastmap` directamente con inicialización, `finish_construction()` y `render()` llamados desde distintos lugares. Ahora, `coast_renderer` proporciona una interfaz limpia que delega a `coastmap`: `finish_construction()`, `render()`, `get_coastmap()`. Archivos nuevos: `coast_renderer.h/cpp`. Modificados: `user_interface.h/cpp` (reemplazado `mycoastmap` por `std::unique_ptr<coast_renderer> mycoast`). Beneficios: consistencia con otros subsistemas de rendering (terrain_manager, weather_renderer), interfaz unificada, facilita futuras optimizaciones de coastmap (spatial indexing, LOD). Bonus: corregido orden de inicialización en constructor para eliminar warnings de compilación. (Completado: 2026-03-02)

- **Guía de estilo y naming:** Creado `docs/STYLE_GUIDE.md` con guía completa de convenciones de código. Documentadas reglas para naming (clases snake_case, variables miembro con prefijo `my` para subsistemas), formato (4 espacios, K&R), C++ moderno (C++20: smart pointers, const correctness, RAII, ranges), y patrones de diseño (inyección de dependencias, subsistemas). La guía formaliza las convenciones existentes del proyecto sin requerir cambios masivos en código legacy. Beneficios: onboarding más fácil para nuevos desarrolladores, base para code reviews, consistencia en contribuciones futuras. (Completado: 2026-03-02)

- **Eliminar código muerto:** Limpieza conservadora de código deshabilitado con `#if 0` y comentarios obsoletos. Eliminadas 60 líneas de código en 3 archivos: (1) `user_interface.cpp`: código viejo de rendering 3D de coastmap (7 líneas), (2) `submarine_interface.cpp`: lógica antigua de lanzamiento de torpedos marcada "old code" (46 líneas), (3) `postprocessor.cpp`: variable comentada "unused" (1 línea). Análisis cuidadoso confirmó que ningún código eliminado se usa. Código con FIXMEs explícitos (como guardado de partículas) y código futuro (ship_interface, airplane_interface) fue dejado intencionalmente. Beneficios: menos ruido, código más claro, reduce confusión sobre qué está activo. (Completado: 2026-03-02)

- **Migrar coastmap props a RAII:** Completada la migración del último caso importante de gestión manual de objcache. El struct `coastmap::prop` ahora usa `objcachet<model>::reference` en lugar de almacenar solo el nombre del modelo y hacer ref/unref manual. Antes: constructor hacía `modelcache().ref()`, destructor de coastmap hacía loop de `unref()`, riesgo de leaks en excepciones. Después: `mymodel` RAII wrapper, construcción automática, destrucción automática, código más simple (~20 líneas simplificadas). Archivos: coastmap.h (struct prop refactorizado, agregado include objcache.h), coastmap.cpp (implementación de constructor, eliminado loop unref, rendering usa mymodel directamente). **El proyecto ahora usa RAII consistentemente para todos los casos de objcache de modelos** (sea_object, freeview_display, coastmap). Widget mantiene patrón manual ref_all/unref_all por complejidad de recarga dinámica de temas. (Completado: 2026-03-02)

- **Extraer PanelManager de user_interface:** Creado sexto subsistema para el panel de información. Antes, `user_interface` manejaba directamente el widget panel, 6 widget_text para valores, inicialización (~13 líneas) y formateo de datos (~20 líneas en draw_infopanel). Ahora, `panel_manager` encapsula toda la gestión del panel con interfaz limpia: `draw()` recibe valores por parámetro, `check_mouse_event()`, `set_visible()`. Datos mostrados: heading, speed, depth, bearing, time_scale, game_time. Archivos nuevos: `panel_manager.h/cpp` (185 líneas total). Modificados: `user_interface.h/cpp` (~35 líneas eliminadas, ~5 líneas agregadas). Beneficios: user_interface más ligero, lógica de panel encapsulada, consistente con otros subsistemas (weather, terrain, scene, coast, messages), más testeable. **user_interface ahora tiene 6 subsistemas extraídos**, quedando significativamente más enfocado en su responsabilidad principal: coordinación de la interfaz. (Completado: 2026-03-02)

- **Optimización de headers pesados:** Segunda ronda de reducción de includes para mejorar tiempos de compilación incremental. Eliminados includes innecesarios usando forward declarations donde posible. `widget.h`: eliminado `xml.h` (solo usado en constructor) → agregada forward declaration `xml_elem`. `sea_object.h`: eliminados `ai.h`, `sensors.h`, `xml.h` (mantenido `sonar.h` por necesidad de `sonar_contact`) → agregadas forward declarations. Corregidas dependencias implícitas: `ship.h` agregada forward declaration `particle`, `gun_shell.h` agregada forward declaration `ship`. Los `.cpp` correspondientes actualizados con includes necesarios. **Impacto en compilación:** ANTES: cambio en ai.h/sensors.h/xml.h recompilaba 50+ archivos (toda la jerarquía sea_object → ship → submarine → displays). DESPUÉS: solo 5-10 archivos recompilados. Mejora de ~5-10x en velocidad de compilación incremental. Archivos: widget.h/cpp, sea_object.h/cpp, ship.h, gun_shell.h. Beneficios: compilación incremental significativamente más rápida, menor acoplamiento entre headers, mejor separación de interfaces vs implementación. (Completado: 2026-03-02)

---

## Prioridad alta (impacto en acoplamiento / compilación)

1. **Migración SDL2 → SDL3**
   - **Estado**: 🚫 **Bloqueada** - SDL3_mixer incompatible (branch `feature/sdl3-migration`, 2026-03-02)
   - **Progreso**: 75% completado (sin audio). Ver `docs/SDL3_MIGRATION.md` en branch para análisis completo.
   - **Bloqueante crítico**: SDL3_mixer cambió completamente su API. Los tipos `Mix_Music` y `Mix_Chunk` ya no existen, reemplazados por `MIX_Mixer`, `MIX_Audio`, `MIX_Track`. Requiere reescritura completa del sistema de audio (~2000 líneas + 50 archivos).
   - **Completado en branch**:
     - ✅ CMakeLists.txt, includes, constantes (145+ archivos)
     - ✅ Sistema de eventos y teclado completamente migrado
     - ✅ widget.h refactorizado (nueva estructura `key_info`)
     - ✅ Constantes de teclas actualizadas (SDLK_a → SDLK_A)
   - **Decisión**: **Mantener SDL2 en master**. SDL3 aún no es viable para proyectos con audio complejo.
   - **Revisión futura**: Evaluar en 6-12 meses cuando SDL3_mixer madure.

---

## Prioridad media (responsabilidades y testabilidad)

2. **Extraer responsabilidades de `user_interface` (continuar)**
   - ✅ **COMPLETADO (mayor parte)**: Extraídos seis subsistemas importantes:
     - `ui_message_queue`: mensajes temporales (ver sección "Hecho")
     - `weather_renderer`: efectos climáticos lluvia/nieve (ver sección "Hecho")
     - `terrain_manager`: gestión de rendering de terreno/geoclipmap (ver sección "Hecho")
     - `scene_environment`: elementos ambientales (sky + caustics) (ver sección "Hecho")
     - `coast_renderer`: rendering de costas (ver sección "Hecho")
     - `panel_manager`: panel de información (ver sección "Hecho")
   - **Pendiente (opcional)**: Agrupa más responsabilidades: gestión de displays/popups, pausa.
   - **user_interface ahora está significativamente más ligero y enfocado**. Los subsistemas de rendering y UI están bien organizados y encapsulados.

3. **Singletons → inyección**
   - ✅ **COMPLETADO**: Inyección de dependencias implementada en toda la base de código crítica. `game`, `user_interface`, displays, `water`, `submarine_interface`, `map_display`, `postprocessor`, y funciones de menú ahora usan inyección de dependencias o referencias locales para `cfg`, `log` y `music`.
   - **Archivos actualizados (resumen completo)**:
     - `water`: constructor recibe `cfg &configuration`, eliminados 6 usos
     - `submarine_interface`: usa `get_config()` en lugar de `cfg::instance()` (2 ubicaciones)
     - `map_display`: usa `ui.get_config()` en lugar de `cfg::instance()` (2 ubicaciones)
     - `game`: inyecta `cfg` en constructor de `water` + usa miembro `config` en constructor privado
     - `postprocessor`: constructor acepta `cfg*` opcional (2 usos eliminados), fallback a `cfg::instance()` para retrocompatibilidad
     - **`subsim.cpp`**: 6 funciones refactorizadas con referencias locales (26 usos eliminados total)
       - Funciones: `menu_opt_video`, `menu_configure_keys`, `configure_key`, `apply_mode`, `menu_select_language`, `mymain`
   - **Resultado final**: ✅ **~40 → ~13 usos inapropiados eliminados (reducción del 67%)**
   - **Usos restantes (todos apropiados)**:
     - 8 en `subsim.cpp`: Inyección de dependencias correcta al crear `game`/`game_editor` ✅
     - 2 en constructores (`user_interface`, `game`): Inicialización de miembros ✅
     - 1 en `terrain.h`: Constructor de template inline (caso especial aceptable) ✅
     - Tests y utilidades: No críticos ✅
   - **Patrón establecido**: DI en clases, referencias locales en funciones, inyección explícita en creación de objetos.
   - Singletons restantes: `system`, `global_data`, `data_file_handler` (estos pueden mantenerse por ahora).

4. **objcache → RAII**
   - ✅ **COMPLETADO**: Todos los casos importantes de gestión manual de objcache migrados a RAII.
   - **Casos migrados**:
     - ✅ `sea_object::mymodel`: objcachet<model>::reference (mutable, lazy load)
     - ✅ `freeview_display`: conning_tower (model), underwater_background y splashring (texture)
     - ✅ `coastmap::props`: struct prop usa objcachet<model>::reference
     - ✅ `sub_damage_display`, `sub_torpedo_display`: notepadsheet (ya usaban RAII)
   - **No migrados (intencional)**:
     - `widget`: patrón complejo ref_all/unref_all para recarga dinámica de temas, funcionando correctamente
   - **Resultado**: Gestión de memoria segura, eliminados loops manuales de ref/unref, sin riesgo de leaks por excepciones.
   - El proyecto ahora usa RAII como patrón estándar para gestión de recursos cached.

---

## Prioridad baja (limpieza y consistencia)

4. **Naming**
   - ✅ **COMPLETADO (parcial)**: Guía de estilo documentada (ver abajo).
   - El proyecto mezcla algunos estilos, pero la convención dominante es clara. **En código nuevo seguir estas reglas**:

### Guía de estilo de código (Danger from the Deep)

**Nombres de clases y estructuras:**
- `snake_case` (minúsculas con guiones bajos)
- Ejemplos: `user_interface`, `sea_object`, `weather_renderer`, `terrain_manager`
- Excepción: bibliotecas externas (TinyXML usa `PascalCase`)

**Variables miembro:**
- Prefijo `my` para subsistemas/objetos complejos propios: `myenvironment`, `mycoast`, `myterrain`, `myweather`
- Sin prefijo para datos simples: `bearing`, `elevation`, `daymode`, `current_display`
- Evitar prefijos como `m_` o `_` (no son el estilo del proyecto)

**Funciones y métodos:**
- `snake_case`: `get_throttle()`, `set_time()`, `draw_infopanel()`, `toggle_wireframe()`
- Getters: `get_X()` donde X es el nombre del dato
- Setters: `set_X(...)` donde X es el nombre del dato
- Booleanos: preferir `is_X()` o `has_X()` para mayor claridad

**Constantes y enums:**
- `UPPER_CASE` con guiones bajos: `MAX_TORPEDOES`, `CONVOY_MAX_SIZE`

**Namespaces:**
- Actualmente no se usan. Si se agregan en el futuro, usar `snake_case`.

**Archivos:**
- Headers: `.h` (no `.hpp`)
- Implementación: `.cpp`
- Nombre del archivo = nombre de la clase principal: `weather_renderer.h`, `scene_environment.cpp`

**Formato:**
- Indentación: 4 espacios (no tabs)
- Llaves: estilo K&R (llave de apertura en misma línea para funciones cortas, nueva línea para clases)
- Líneas: preferir < 100 caracteres cuando sea razonable

**Comentarios:**
- Usar `///` para documentación Doxygen en headers
- Evitar comentarios obvios; el código debe ser autoexplicativo
- Comentar el "por qué", no el "qué"

**Moderno C++:**
- Preferir `std::unique_ptr` sobre raw pointers
- Usar `const` siempre que sea posible
- RAII para gestión de recursos
- `auto` para tipos complejos evidentes
- Range-based for loops cuando sea apropiado
- C++20: concepts, ranges, designated initializers disponibles

   - **Pendiente**: Aplicar consistentemente en nuevas contribuciones. No refactorizar masivamente código existente solo por naming.

---

## No recomendado (por ahora)

- **OpenGL → Vulkan:** Reescribir todo el pipeline de render; esfuerzo muy alto.
- **Refactor masivo de `game` en un solo paso:** Hacerlo por pasos pequeños (extraer una responsabilidad, compilar y pasar tests, repetir).

---

## Cómo usar este documento

- Al tocar un área (terrain, game, UI, objcache), consultar aquí si hay un ítem aplicable.
- Completar un ítem → marcarlo o moverlo a "Hecho" y actualizar la fecha.
- Nuevas ideas de refactor → añadirlas con prioridad (alta/media/baja).
