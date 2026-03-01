# Tests unitarios (opcional: -DBUILD_UNIT_TESTS=ON)
set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SRC_PARENT ${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(${SRC_PARENT} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

# ptrlist_test: solo ptrlist.h (header-only)
add_executable(ptrlist_test ${TEST_DIR}/ptrlist_test.cpp)
target_link_libraries(ptrlist_test ${LIBS})
add_test(NAME ptrlist_test COMMAND ptrlist_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# ptrvector_test: solo ptrvector.h (header-only)
add_executable(ptrvector_test ${TEST_DIR}/ptrvector_test.cpp)
target_link_libraries(ptrvector_test ${LIBS})
add_test(NAME ptrvector_test COMMAND ptrvector_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# angle_test: angle.h + vector2.h (header-only)
add_executable(angle_test ${TEST_DIR}/angle_test.cpp)
target_link_libraries(angle_test ${LIBS})
add_test(NAME angle_test COMMAND angle_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# vector2_test: vector2.h (header-only)
add_executable(vector2_test ${TEST_DIR}/vector2_test.cpp)
target_link_libraries(vector2_test ${LIBS})
add_test(NAME vector2_test COMMAND vector2_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# vector3_test: vector3.h + vector2.h
add_executable(vector3_test ${TEST_DIR}/vector3_test.cpp)
target_link_libraries(vector3_test ${LIBS})
add_test(NAME vector3_test COMMAND vector3_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# color_test: color.h (SDL_types)
add_executable(color_test ${TEST_DIR}/color_test.cpp)
target_link_libraries(color_test ${LIBS})
add_test(NAME color_test COMMAND color_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# fixed_test: fixed.h (fixed32)
add_executable(fixed_test ${TEST_DIR}/fixed_test.cpp)
target_link_libraries(fixed_test ${LIBS})
add_test(NAME fixed_test COMMAND fixed_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# random_generator_test: random_generator.h
add_executable(random_generator_test ${TEST_DIR}/random_generator_test.cpp)
target_link_libraries(random_generator_test ${LIBS})
add_test(NAME random_generator_test COMMAND random_generator_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# bivector_test: bivector.h + vector2.h + random_generator.h
add_executable(bivector_test ${TEST_DIR}/bivector_test.cpp)
target_link_libraries(bivector_test ${LIBS})
add_test(NAME bivector_test COMMAND bivector_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# global_data_utils_test: ulog2, nextgteqpow2, ispow2 (inline en global_data.h)
add_executable(global_data_utils_test ${TEST_DIR}/global_data_utils_test.cpp)
target_link_libraries(global_data_utils_test ${LIBS})
add_test(NAME global_data_utils_test COMMAND global_data_utils_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# rnd_global_test: rnd(), seed_global_rnd() (rnd.cpp, solo std)
add_executable(rnd_global_test ${TEST_DIR}/rnd_global_test.cpp ${SRC_PARENT}/rnd.cpp)
target_link_libraries(rnd_global_test ${LIBS})
add_test(NAME rnd_global_test COMMAND rnd_global_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# string_split_test: string_split() (string_split.cpp)
add_executable(string_split_test ${TEST_DIR}/string_split_test.cpp ${SRC_PARENT}/string_split.cpp)
target_link_libraries(string_split_test ${LIBS})
add_test(NAME string_split_test COMMAND string_split_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# vector4_test: vector4.h
add_executable(vector4_test ${TEST_DIR}/vector4_test.cpp)
target_link_libraries(vector4_test ${LIBS})
add_test(NAME vector4_test COMMAND vector4_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# plane_test: plane.h + vector3.h
add_executable(plane_test ${TEST_DIR}/plane_test.cpp)
target_link_libraries(plane_test ${LIBS})
add_test(NAME plane_test COMMAND plane_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# sphere_test: sphere.h
add_executable(sphere_test ${TEST_DIR}/sphere_test.cpp)
target_link_libraries(sphere_test ${LIBS})
add_test(NAME sphere_test COMMAND sphere_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# matrix3_test: matrix3.h + matrix.h
add_executable(matrix3_test ${TEST_DIR}/matrix3_test.cpp)
target_link_libraries(matrix3_test ${LIBS})
add_test(NAME matrix3_test COMMAND matrix3_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# quaternion_test: quaternion.h (matrix4, vector3)
add_executable(quaternion_test ${TEST_DIR}/quaternion_test.cpp)
target_link_libraries(quaternion_test ${LIBS})
add_test(NAME quaternion_test COMMAND quaternion_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# triangle_intersection_test: triangle_intersection.h
add_executable(triangle_intersection_test ${TEST_DIR}/triangle_intersection_test.cpp)
target_link_libraries(triangle_intersection_test ${LIBS})
add_test(NAME triangle_intersection_test COMMAND triangle_intersection_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# keys_test: keys.h + keys.cpp
add_executable(keys_test ${TEST_DIR}/keys_test.cpp ${SRC_PARENT}/keys.cpp)
target_link_libraries(keys_test ${LIBS})
add_test(NAME keys_test COMMAND keys_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# bitstream_test: bitstream.h + bitstream.cpp
add_executable(bitstream_test ${TEST_DIR}/bitstream_test.cpp ${SRC_PARENT}/bitstream.cpp)
target_link_libraries(bitstream_test ${LIBS})
add_test(NAME bitstream_test COMMAND bitstream_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# mutex_test: mutex + error (requieren SDL2)
add_executable(mutex_test ${TEST_DIR}/mutex_test.cpp ${SRC_PARENT}/mutex.cpp ${SRC_PARENT}/error.cpp)
target_link_libraries(mutex_test ${LIBS})
add_test(NAME mutex_test COMMAND mutex_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# parser_test: parser + error; ruta CSV por argv[1] o env DFTD_DATA
add_executable(parser_test ${TEST_DIR}/parser_test.cpp ${SRC_PARENT}/parser.cpp ${SRC_PARENT}/error.cpp)
target_link_libraries(parser_test ${LIBS})
add_test(NAME parser_test COMMAND parser_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# error_test: error.h (excepciones error, file_read_error)
add_executable(error_test ${TEST_DIR}/error_test.cpp)
target_link_libraries(error_test ${LIBS})
add_test(NAME error_test COMMAND error_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# align16_allocator_test: align16_allocator.h
add_executable(align16_allocator_test ${TEST_DIR}/align16_allocator_test.cpp)
target_link_libraries(align16_allocator_test ${LIBS})
add_test(NAME align16_allocator_test COMMAND align16_allocator_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# dmath_test: dmath.h (isfinite)
add_executable(dmath_test ${TEST_DIR}/dmath_test.cpp)
target_link_libraries(dmath_test ${LIBS})
add_test(NAME dmath_test COMMAND dmath_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# binstream_test: binstream.h (roundtrip write/read)
add_executable(binstream_test ${TEST_DIR}/binstream_test.cpp)
target_link_libraries(binstream_test ${LIBS})
add_test(NAME binstream_test COMMAND binstream_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# matrix4_test: matrix4.h
add_executable(matrix4_test ${TEST_DIR}/matrix4_test.cpp)
target_link_libraries(matrix4_test ${LIBS})
add_test(NAME matrix4_test COMMAND matrix4_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# condvar_test: condvar + mutex + error (SDL2)
add_executable(condvar_test ${TEST_DIR}/condvar_test.cpp ${SRC_PARENT}/condvar.cpp ${SRC_PARENT}/mutex.cpp ${SRC_PARENT}/error.cpp)
target_link_libraries(condvar_test ${LIBS})
add_test(NAME condvar_test COMMAND condvar_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# bspline_test: bspline.h
add_executable(bspline_test ${TEST_DIR}/bspline_test.cpp)
target_link_libraries(bspline_test ${LIBS})
add_test(NAME bspline_test COMMAND bspline_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# singleton_test: singleton.h
add_executable(singleton_test ${TEST_DIR}/singleton_test.cpp)
target_link_libraries(singleton_test ${LIBS})
add_test(NAME singleton_test COMMAND singleton_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# objcache_test: objcache.h
add_executable(objcache_test ${TEST_DIR}/objcache_test.cpp)
target_link_libraries(objcache_test ${LIBS})
add_test(NAME objcache_test COMMAND objcache_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# polygon_test: polygon.h
add_executable(polygon_test ${TEST_DIR}/polygon_test.cpp)
target_link_libraries(polygon_test ${LIBS})
add_test(NAME polygon_test COMMAND polygon_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
