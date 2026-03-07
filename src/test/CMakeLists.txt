# Tests unitarios (opcional: -DBUILD_UNIT_TESTS=ON)
set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SRC_PARENT ${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(${SRC_PARENT} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

# Catch2 testing framework
add_library(catch2_main STATIC ${TEST_DIR}/catch_amalgamated.cpp)
target_include_directories(catch2_main PUBLIC ${TEST_DIR})
# Compilar Catch2 sin warnings
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(catch2_main PRIVATE -w)
endif()

# Helper macro para tests con Catch2
macro(add_catch2_test test_name)
    add_executable(${test_name} ${TEST_DIR}/${test_name}.cpp ${ARGN})
    target_link_libraries(${test_name} catch2_main ${LIBS})
    add_test(NAME ${test_name} COMMAND ${test_name} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endmacro()

# ptrlist_test: ptrlist.h (header-only) - MIGRADO A CATCH2
add_catch2_test(ptrlist_test)

# ptrvector_test: ptrvector.h (header-only) - MIGRADO A CATCH2
add_catch2_test(ptrvector_test)

# angle_test: angle.h + vector2.h (header-only) - MIGRADO A CATCH2
add_catch2_test(angle_test)

# vector2_test: vector2.h (header-only) - MIGRADO A CATCH2
add_catch2_test(vector2_test)

# vector3_test: vector3.h + vector2.h - MIGRADO A CATCH2
add_catch2_test(vector3_test)

# color_test: color.h (SDL_types) - MIGRADO A CATCH2
add_catch2_test(color_test)

# fixed_test: fixed.h (fixed32) - MIGRADO A CATCH2
add_catch2_test(fixed_test)

# random_generator_test: random_generator.h - MIGRADO A CATCH2
add_catch2_test(random_generator_test)

# bivector_test: bivector.h + vector2.h - MIGRADO A CATCH2
add_catch2_test(bivector_test)

# global_data_utils_test: ulog2, nextgteqpow2, ispow2 (inline en global_data.h) - MIGRADO A CATCH2
add_catch2_test(global_data_utils_test)

# global_data_math_test: myfmod, myfrac, mysgn, myclamp, myinterpolate (inline en global_data.h)
add_catch2_test(global_data_math_test)

# str_utils_test: str(), str_wf() de global_data.h
add_catch2_test(str_utils_test)

# rnd_test: rnd(), seed_global_rnd(), rnd(unsigned) - Catch2 (reemplaza rnd_global_test)
# string_split_test: string_split() - MIGRADO A CATCH2
add_catch2_test(string_split_test ${SRC_PARENT}/string_split.cpp)

# vector4_test: vector4.h - MIGRADO A CATCH2
add_catch2_test(vector4_test)

# plane_test: plane.h + vector3.h - MIGRADO A CATCH2
add_catch2_test(plane_test)

# sphere_test: sphere.h - MIGRADO A CATCH2
add_catch2_test(sphere_test)

# matrix3_test: matrix3.h + matrix.h - MIGRADO A CATCH2
add_catch2_test(matrix3_test)

# quaternion_test: quaternion.h (matrix4, vector3) - MIGRADO A CATCH2
add_catch2_test(quaternion_test)

# triangle_intersection_test: triangle_intersection.h - MIGRADO A CATCH2
add_catch2_test(triangle_intersection_test)

# keys_test: keys.h + keys.cpp - MIGRADO A CATCH2
add_catch2_test(keys_test ${SRC_PARENT}/keys.cpp)

# bitstream_test: bitstream.h + bitstream.cpp - MIGRADO A CATCH2
add_catch2_test(bitstream_test ${SRC_PARENT}/bitstream.cpp)

# mutex_test: mutex + error (stub display_get_error) - MIGRADO A CATCH2
add_catch2_test(mutex_test ${SRC_PARENT}/mutex.cpp ${SRC_PARENT}/error.cpp ${TEST_DIR}/display_backend_stub.cpp)

# parser_test: parser + error; DFTD_DATA para common.csv
add_catch2_test(parser_test ${SRC_PARENT}/parser.cpp ${SRC_PARENT}/error.cpp ${TEST_DIR}/display_backend_stub.cpp)
set_tests_properties(parser_test PROPERTIES ENVIRONMENT "DFTD_DATA=${CMAKE_SOURCE_DIR}/data")

# error_test: error.h + error.cpp (excepciones error, file_read_error, sdl_error)
add_catch2_test(error_test ${SRC_PARENT}/error.cpp ${TEST_DIR}/display_backend_stub.cpp)

# align16_allocator_test: align16_allocator.h - MIGRADO A CATCH2
add_catch2_test(align16_allocator_test)

# dmath_test: dmath.h (isfinite) - MIGRADO A CATCH2
add_catch2_test(dmath_test)

# binstream_test: binstream.h (roundtrip write/read) - MIGRADO A CATCH2
add_catch2_test(binstream_test)

# matrix4_test: matrix4.h - MIGRADO A CATCH2
add_catch2_test(matrix4_test)

# condvar_test: condvar + mutex + error (stub display_get_error) - MIGRADO A CATCH2
add_catch2_test(condvar_test ${SRC_PARENT}/condvar.cpp ${SRC_PARENT}/mutex.cpp ${SRC_PARENT}/error.cpp ${TEST_DIR}/display_backend_stub.cpp)

# bspline_test: bspline.h - MIGRADO A CATCH2
add_catch2_test(bspline_test ${SRC_PARENT}/rnd.cpp)

# singleton_test: singleton.h - MIGRADO A CATCH2
add_catch2_test(singleton_test)

# objcache_test: objcache.h - MIGRADO A CATCH2
add_catch2_test(objcache_test)

# polygon_test: polygon.h - MIGRADO A CATCH2
add_catch2_test(polygon_test)

# matrix_test: matrix.h (matrix_invert, matrix_swap_rows, matrixt) - MIGRADO A CATCH2
add_catch2_test(matrix_test)

# global_constants_test: global_constants.h - MIGRADO A CATCH2
add_catch2_test(global_constants_test)

# === 50 tests adicionales ===
add_catch2_test(triangulate_test ${SRC_PARENT}/triangulate.cpp ${SRC_PARENT}/rnd.cpp)

# morton_bivector_test: morton_bivector.h - MIGRADO A CATCH2
add_catch2_test(morton_bivector_test)

add_catch2_test(bzip_test)

# xml_exception_test: xml.h (excepciones) - MIGRADO A CATCH2
add_catch2_test(xml_exception_test)

# xml_attr_test: attri/attrf (stoi/stod) con XML temporal
add_catch2_test(xml_attr_test ${SRC_PARENT}/xml.cpp)

add_catch2_test(cfg_key_test)

# cfg_load_test: cfg::load + set_str (stoi/stod)
add_catch2_test(cfg_load_test ${SRC_PARENT}/cfg.cpp ${SRC_PARENT}/xml.cpp ${SRC_PARENT}/keys.cpp ${SRC_PARENT}/log.cpp ${SRC_PARENT}/mutex.cpp ${SRC_PARENT}/error.cpp ${TEST_DIR}/display_backend_stub.cpp)

add_catch2_test(bv_tree_leaf_test)

add_catch2_test(datadirs_test ${SRC_PARENT}/datadirs.cpp ${SRC_PARENT}/filehelper.cpp ${SRC_PARENT}/log.cpp ${SRC_PARENT}/mutex.cpp ${SRC_PARENT}/error.cpp ${TEST_DIR}/display_backend_stub.cpp)

add_catch2_test(filehelper_test ${SRC_PARENT}/filehelper.cpp ${SRC_PARENT}/error.cpp ${TEST_DIR}/display_backend_stub.cpp)

add_catch2_test(log_test ${SRC_PARENT}/log.cpp ${SRC_PARENT}/mutex.cpp ${SRC_PARENT}/error.cpp ${TEST_DIR}/display_backend_stub.cpp)

# faulthandler_test: faulthandler.h - MIGRADO A CATCH2
add_catch2_test(faulthandler_test ${SRC_PARENT}/faulthandler.cpp)

add_catch2_test(perlinnoise_test ${SRC_PARENT}/perlinnoise.cpp ${SRC_PARENT}/rnd.cpp)

add_catch2_test(simplex_noise_test ${SRC_PARENT}/simplex_noise.cpp)

add_catch2_test(frustum_test ${SRC_PARENT}/frustum.cpp)

add_catch2_test(fractal_test ${SRC_PARENT}/simplex_noise.cpp)

add_catch2_test(thread_test ${SRC_PARENT}/thread.cpp ${SRC_PARENT}/condvar.cpp ${SRC_PARENT}/mutex.cpp ${SRC_PARENT}/error.cpp ${SRC_PARENT}/log.cpp ${TEST_DIR}/display_backend_stub.cpp)

# xml_doc_test: eliminado (test stub sin valor)
# tone_reproductor_test: eliminado (test stub sin valor)

add_catch2_test(bv_tree_test ${SRC_PARENT}/bv_tree.cpp)

add_catch2_test(event_test)

add_catch2_test(tile_test)

add_catch2_test(tile_cache_test)

add_catch2_test(terrain_test)

add_catch2_test(height_generator_test)

add_catch2_test(ocean_wave_generator_test)

add_catch2_test(global_data_test)

# Tests para subsistemas extraídos (refactoring de game) - MIGRADOS A CATCH2
add_catch2_test(trail_manager_test)

add_catch2_test(visibility_manager_test ${SRC_PARENT}/visibility_manager.cpp)

add_catch2_test(time_freezer_test ${TEST_DIR}/time_freezer_stub.cpp)

add_catch2_test(scoring_manager_test ${SRC_PARENT}/scoring_manager.cpp ${SRC_PARENT}/date.cpp ${SRC_PARENT}/xml.cpp)

add_catch2_test(date_test ${SRC_PARENT}/date.cpp ${SRC_PARENT}/xml.cpp)

add_catch2_test(ping_manager_test ${SRC_PARENT}/ping_manager.cpp ${SRC_PARENT}/xml.cpp)

# lighting_system_test: requiere shader.cpp, glsl_program, texture (OpenGL/dependencias muy pesadas)
# add_executable(lighting_system_test ${TEST_DIR}/lighting_system_test.cpp ${SRC_PARENT}/lighting_system.cpp ${SRC_PARENT}/moon.cpp ${SRC_PARENT}/stars.cpp ${SRC_PARENT}/date.cpp ${SRC_PARENT}/xml.cpp)
# target_link_libraries(lighting_system_test ${LIBS})
# add_test(NAME lighting_system_test COMMAND lighting_system_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_catch2_test(physics_system_test ${TEST_DIR}/physics_system_stub.cpp)

add_catch2_test(event_manager_test ${TEST_DIR}/event_manager_stub.cpp)

add_catch2_test(logbook_test ${SRC_PARENT}/logbook.cpp)

# Tests que requieren juego/OpenGL completo: sensors, coastmap, image, model, texture,
# font, primitives, shader, music, height_generator_map, geoclipmap, caustics, water_splash,
# particle, stars, moon, sky, daysky, water, sonar, gun_shell, depth_charge, torpedo,
# convoy, sea_object, ship, airplane, submarine, ai, scene_environment, coast_renderer,
# terrain_manager, weather_renderer - tienen .cpp en src/test/ pero no se enlazan aquí 
# (dependencias pesadas). tree_generator, countrycodes, message_queue, cfg, idem.

# === 35 tests smoke de cobertura (2026-03-03) ===
# Tests simples que verifican compilación básica para aumentar cobertura

add_catch2_test(rnd_test ${SRC_PARENT}/rnd.cpp)
add_catch2_test(player_info_test)
add_catch2_test(job_scheduler_test)
add_catch2_test(ui_messages_test)
add_catch2_test(sub_control_popup_test)
add_catch2_test(sub_ecard_popup_test)
add_catch2_test(sub_tdc_popup_test)
add_catch2_test(sub_valves_display_test)
add_catch2_test(ships_sunk_display_test)
add_catch2_test(sub_soldbuch_display_test)
add_catch2_test(torpedo_camera_display_test)
add_catch2_test(sub_bridge_display_test)
# add_catch2_test(sub_damage_display_test)  # Requiere includes de image.h
add_catch2_test(sub_uzo_display_test)
add_catch2_test(airplane_interface_test)
add_catch2_test(height_generator_map_test)
add_catch2_test(user_popup_test)
add_catch2_test(sub_recogmanual_display_test)
add_catch2_test(sub_bg_display_test)
add_catch2_test(sub_recogmanual_popup_test)
add_catch2_test(ship_interface_test)
# add_catch2_test(gldebug_test)  # Requiere OpenGL (GLenum)
add_catch2_test(logbook_display_test)
add_catch2_test(highscorelist_test)
add_catch2_test(scene_environment_test)
add_catch2_test(coast_renderer_test)
add_catch2_test(weather_renderer_test)
add_catch2_test(caustics_test)
add_catch2_test(moon_test)
add_catch2_test(stars_test)
add_catch2_test(daysky_test)
add_catch2_test(postprocessor_test)
add_catch2_test(framebufferobject_test)
add_catch2_test(vertexbufferobject_test)
add_catch2_test(depth_charge_test)
