# Tests unitarios (opcional: -DBUILD_UNIT_TESTS=ON)
set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SRC_PARENT ${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(${SRC_PARENT} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

# ptrlist_test: solo ptrlist.h (header-only)
add_executable(ptrlist_test ${TEST_DIR}/ptrlist_test.cpp)
target_link_libraries(ptrlist_test ${LIBS})
add_test(NAME ptrlist_test COMMAND ptrlist_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# mutex_test: mutex + error (requieren SDL2)
add_executable(mutex_test ${TEST_DIR}/mutex_test.cpp ${SRC_PARENT}/mutex.cpp ${SRC_PARENT}/error.cpp)
target_link_libraries(mutex_test ${LIBS})
add_test(NAME mutex_test COMMAND mutex_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# parser_test: parser + error; ruta CSV por argv[1] o env DFTD_DATA
add_executable(parser_test ${TEST_DIR}/parser_test.cpp ${SRC_PARENT}/parser.cpp ${SRC_PARENT}/error.cpp)
target_link_libraries(parser_test ${LIBS})
add_test(NAME parser_test COMMAND parser_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
