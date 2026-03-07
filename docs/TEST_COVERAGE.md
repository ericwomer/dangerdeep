# Tests y Cobertura de Código

Documentación sobre la suite de tests unitarios y la cobertura de cada archivo fuente.

---

## Índice

1. [Resumen ejecutivo](#resumen-ejecutivo)
2. [Tests por archivo fuente](#tests-por-archivo-fuente)
3. [Cobertura por archivo](#cobertura-por-archivo)
4. [Generar reporte de cobertura](#generar-reporte-de-cobertura)
5. [Archivos sin tests directos](#archivos-sin-tests-directos)

---

## Resumen ejecutivo

| Métrica | Valor |
|---------|-------|
| **Total tests** | 102 |
| **Framework** | Catch2 v3.5.2 (102 tests) |
| **Cobertura líneas (src/)** | ~58.7% |
| **Cobertura funciones (src/)** | ~70.1% |
| **Cobertura branches (src/)** | ~34.9% |
| **Tiempo ejecución** | ~0.11 s |

Los tests se compilan con `-DBUILD_UNIT_TESTS=ON` (por defecto ON) y se ejecutan con `ctest` o `./check.sh --unit`.

---

## Tests por archivo fuente

Mapeo de cada test a los archivos fuente que ejercita directamente:

| Test | Archivos fuente cubiertos |
|------|---------------------------|
| `align16_allocator_test` | align16_allocator.h |
| `angle_test` | angle.h, vector2.h |
| `binstream_test` | binstream.h |
| `bitstream_test` | bitstream.h, bitstream.cpp |
| `bivector_test` | bivector.h, vector2.h, random_generator.h |
| `bspline_test` | bspline.h, rnd.cpp |
| `bv_tree_test` | bv_tree.cpp |
| `bv_tree_leaf_test` | bv_tree.h (leaf) |
| `bzip_test` | bzip.h |
| `caustics_test` | caustics.h/cpp (smoke) |
| `cfg_key_test` | cfg.h (struct key) |
| `cfg_load_test` | cfg.cpp, xml.cpp, keys.cpp, log.cpp, mutex.cpp, error.cpp |
| `color_test` | color.h |
| `condvar_test` | condvar.cpp, mutex.cpp, error.cpp |
| `coast_renderer_test` | coast_renderer.h (smoke) |
| `datadirs_test` | datadirs.cpp, filehelper.cpp, log.cpp, mutex.cpp, error.cpp |
| `date_test` | date.cpp, xml.cpp |
| `daysky_test` | daysky.h (smoke) |
| `depth_charge_test` | depth_charge.h (smoke) |
| `error_test` | error.h, error.cpp (sdl_error) |
| `event_manager_test` | event_manager.h (stub) |
| `event_test` | event.h |
| `faulthandler_test` | faulthandler.cpp |
| `filehelper_test` | filehelper.cpp, error.cpp |
| `fixed_test` | fixed.h |
| `fractal_test` | fractal.h, simplex_noise.cpp |
| `framebufferobject_test` | framebufferobject.h (smoke) |
| `frustum_test` | frustum.h, frustum.cpp |
| `global_data_math_test` | global_data.h (myfmod, myfrac, mysgn, myclamp, myinterpolate) |
| `global_data_utils_test` | global_data.h (ulog2, nextgteqpow2, ispow2) |
| `global_data_test` | global_data.h |
| `global_constants_test` | global_constants.h |
| `height_generator_map_test` | height_generator_map.h (smoke) |
| `height_generator_test` | height_generator.h |
| `highscorelist_test` | highscorelist.h (smoke) |
| `job_scheduler_test` | job_scheduler.h (smoke) |
| `keys_test` | keys.h, keys.cpp |
| `log_test` | log.cpp, mutex.cpp, error.cpp |
| `logbook_test` | logbook.cpp |
| `logbook_display_test` | logbook_display.h (smoke) |
| `matrix_test` | matrix.h |
| `matrix3_test` | matrix3.h, matrix.h |
| `matrix4_test` | matrix4.h |
| `morton_bivector_test` | morton_bivector.h |
| `moon_test` | moon.h (smoke) |
| `mutex_test` | mutex.cpp, error.cpp |
| `objcache_test` | objcache.h |
| `ocean_wave_generator_test` | ocean_wave_generator.h |
| `parser_test` | parser.cpp, error.cpp |
| `perlinnoise_test` | perlinnoise.cpp, rnd.cpp |
| `physics_system_test` | physics_system.h (stub) |
| `ping_manager_test` | ping_manager.cpp, xml.cpp |
| `plane_test` | plane.h, vector3.h |
| `player_info_test` | player_info.h (smoke) |
| `polygon_test` | polygon.h |
| `postprocessor_test` | postprocessor.h (smoke) |
| `ptrlist_test` | ptrlist.h |
| `ptrvector_test` | ptrvector.h |
| `quaternion_test` | quaternion.h |
| `random_generator_test` | random_generator.h |
| `rnd_test` | rnd.cpp |
| `scene_environment_test` | scene_environment.h (smoke) |
| `scoring_manager_test` | scoring_manager.cpp, date.cpp, xml.cpp |
| `date_test` | date.cpp, xml.cpp |
| `ship_interface_test` | ship_interface.h (smoke) |
| `ships_sunk_display_test` | ships_sunk_display.h (smoke) |
| `simplex_noise_test` | simplex_noise.cpp |
| `singleton_test` | singleton.h |
| `sphere_test` | sphere.h |
| `stars_test` | stars.h (smoke) |
| `str_utils_test` | global_data.h (str, str_wf) |
| `string_split_test` | string_split.cpp |
| `sub_bg_display_test` | sub_bg_display.h (smoke) |
| `sub_bridge_display_test` | sub_bridge_display.h (smoke) |
| `sub_control_popup_test` | sub_control_popup.h (smoke) |
| `sub_ecard_popup_test` | sub_ecard_popup.h (smoke) |
| `sub_recogmanual_display_test` | sub_recogmanual_display.h (smoke) |
| `sub_recogmanual_popup_test` | sub_recogmanual_popup.h (smoke) |
| `sub_soldbuch_display_test` | sub_soldbuch_display.h (smoke) |
| `sub_tdc_popup_test` | sub_tdc_popup.h (smoke) |
| `sub_torpsetup_display_test` | (smoke) |
| `sub_uzo_display_test` | sub_uzo_display.h (smoke) |
| `sub_valves_display_test` | sub_valves_display.h (smoke) |
| `terrain_test` | terrain.h |
| `thread_test` | thread.cpp, condvar.cpp, mutex.cpp, error.cpp, log.cpp |
| `triangulate_test` | triangulate.cpp, rnd.cpp |
| `triangulate_test` | triangulate.cpp, rnd.cpp |
| `tile_cache_test` | tile_cache.h |
| `tile_test` | tile.h |
| `time_freezer_test` | time_freezer.h (stub) |
| `torpedo_camera_display_test` | torpedo_camera_display.h (smoke) |
| `trail_manager_test` | trail_manager.h |
| `triangle_intersection_test` | triangle_intersection.h |
| `triangulate_test` | triangulate.cpp |
| `ui_messages_test` | ui_messages.h (smoke) |
| `user_popup_test` | user_popup.h (smoke) |
| `vector2_test` | vector2.h |
| `vector3_test` | vector3.h, vector2.h |
| `vector4_test` | vector4.h |
| `vertexbufferobject_test` | vertexbufferobject.h (smoke) |
| `visibility_manager_test` | visibility_manager.cpp |
| `weather_renderer_test` | weather_renderer.h (smoke) |
| `xml_attr_test` | xml.cpp (attri, attrf) |
| `xml_exception_test` | xml.h (excepciones) |
| `airplane_interface_test` | airplane_interface.h (smoke) |

---

## Cobertura por archivo

Cobertura de líneas (L), branches (B) y funciones (F) para archivos en `src/`. Datos de `./check.sh --coverage` (lcov).

### Cobertura alta (≥ 90% líneas)

| Archivo | L | B | F |
|---------|---|---|---|
| align16_allocator.h | 100% | - | - |
| angle.h | 100% | 54.5% | 100% |
| binstream.h | 100% | 55.3% | 100% |
| color.h | 100% | - | 100% |
| condvar.h | 100% | 50% | - |
| datadirs.h | 100% | - | 100% |
| error.h | 100% | 50% | 100% |
| event_manager.h | 100% | 50% | - |
| fractal.h | 95.8% | 83.3% | 100% |
| frustum.h | 100% | - | - |
| global_data.h | 100% | 50% | 100% |
| logbook.cpp | 100% | 75% | 100% |
| logbook.h | 100% | 50% | 100% |
| matrix.h | 90.5% | 93.8% | 70% |
| matrix3.h | 100% | 50% | 100% |
| mutex.h | 100% | 28.6% | - |
| objcache.h | 92.2% | 49.2% | 100% |
| parser.h | 100% | 50% | - |
| ping_manager.h | 100% | 50% | - |
| ptrlist.h | 100% | 62.5% | 100% |
| ptrvector.h | 100% | 55.4% | 100% |
| quaternion.h | 100% | - | 100% |
| random_generator.h | 100% | 50% | 100% |
| rnd.cpp | 100% | 80% | 100% |
| scoring_manager.h | 100% | 50% | 100% |
| singleton.h | 95% | 60% | 100% |
| sphere.h | 95% | 46.7% | 100% |
| string_split.cpp | 100% | 70% | 100% |
| time_freezer.h | 100% | 50% | - |
| trail_manager.h | 100% | 100% | - |
| vector4.h | 92.9% | 50% | 100% |
| visibility_manager.cpp | 100% | - | 100% |
| visibility_manager.h | 100% | 50% | - |

### Cobertura media (75–90% líneas)

| Archivo | L | B | F |
|---------|---|---|---|
| bitstream.cpp | 74.2% | 53.7% | 90% |
| bitstream.h | 83.3% | 37.5% | 100% |
| bivector.h | 80.6% | 46% | 100% |
| bspline.h | 81.2% | 55.4% | 71.4% |
| cfg.h | 85.7% | 14.3% | 50% |
| plane.h | 84.6% | 62.5% | 66.7% |
| morton_bivector.h | 76.9% | 31% | 100% |
| parser.cpp | 80.5% | 48.9% | 80% |
| simplex_noise.h | 75% | 38.9% | 0% |
| triangulate.h | 77.8% | 50% | 100% |
| vector2.h | 79.3% | 48.2% | 75% |

### Cobertura baja (< 75% líneas)

| Archivo | L | B | F |
|---------|---|---|---|
| bv_tree.cpp | 11.9% | 10.7% | 10% |
| bv_tree.h | 33.3% | 17.5% | 33.3% |
| bzip.h | 20% | 10% | 100% |
| cfg.cpp | 54.7% | ~20% | ~55% |
| datadirs.cpp | 12.8% | 0% | 25% |
| date.cpp | 24.4% | 18.4% | 26.7% |
| error.cpp | 37.5% | - | 50% |
| faulthandler.cpp | 4.8% | 0% | 33.3% |
| filehelper.cpp | 38.5% | 23.1% | 28.6% |
| fixed.h | 55% | 2.8% | - |
| frustum.cpp | 17.9% | 23.3% | 20% |
| log.cpp | 61.1% | 32.7% | 71.4% |
| matrix4.h | 62.8% | 41.7% | 37.5% |
| perlinnoise.cpp | 62.5% | ~15% | ~45% |
| perlinnoise.h | 50% | - | - |
| ping_manager.cpp | 27.5% | 13.6% | 42.9% |
| polygon.h | 43.4% | 33.1% | 83.3% |
| scoring_manager.cpp | 21.6% | 5.6% | 33.3% |
| simplex_noise.cpp | 52% | 33.9% | 71.4% |
| thread.cpp | 45.1% | 20.6% | 53.8% |
| thread.h | 50% | 50% | 33.3% |
| triangle_intersection.h | 21.8% | 6.2% | 100% |
| triangulate.cpp | 29.9% | 11.4% | 66.7% |
| vector3.h | 68.9% | 51.9% | 84.2% |
| xml.cpp | 35.8% | 12.9% | 39.5% |
| xml.h | 69.2% | 34.4% | 100% |

---

## Generar reporte de cobertura

```bash
# Compilar con coverage y ejecutar tests
./check.sh --coverage

# El reporte HTML se genera en:
# build/coverage/html/index.html
```

Para ver solo los archivos de `src/`:

```
build/coverage/html/src/index.html
```

El reporte incluye:
- Cobertura por archivo (líneas, branches, funciones)
- Código fuente anotado línea a línea
- Ordenación por tasa de cobertura

---

## Archivos sin tests directos

Archivos en `src/` que **no** tienen tests unitarios dedicados (solo se ejercitan indirectamente o en el juego):

| Categoría | Archivos |
|-----------|----------|
| **Render/OpenGL** | texture.cpp, shader.cpp, image.cpp, model.cpp, primitives.cpp, font.cpp, framebufferobject.cpp, vertexbufferobject.cpp |
| **Simulación** | subsim.cpp, ai.cpp, ship.cpp, submarine.cpp, airplane.cpp, sea_object.cpp, torpedo.cpp, gun_shell.cpp, depth_charge.cpp |
| **Escena** | water.cpp, sky.cpp, daysky.cpp, stars.cpp, moon.cpp, caustics.cpp, particle.cpp, sonar.cpp |
| **Terreno** | height_generator_map.cpp, geoclipmap.cpp, coastmap.cpp, coast_renderer.cpp |
| **UI/Display** | freeview_display.cpp, user_interface.cpp, y la mayoría de *\_display.cpp, *\_popup.cpp |
| **Otros** | music.cpp, make_mesh.cpp, viewmodel.cpp, portal.cpp, texts.cpp, data.cpp |

Estos módulos tienen dependencias pesadas (OpenGL, SDL, sistema de ventanas) y requieren stubs o tests de integración para aumentar la cobertura.

---

**Última actualización**: 2026-03-07  
**Fuente de cobertura**: `./check.sh --coverage` (lcov)

### Mejoras recientes (2026-03-07)
- **perlinnoise.cpp**: 2.5% → 62.5% (perlinnoise_test ampliado con Catch2)
- **error.cpp**: 0% → 37.5% (sdl_error test, error_test migrado a Catch2)
- **cfg.cpp**: 33.7% → 54.7% (cfg_load_test: roundtrip save/load)
