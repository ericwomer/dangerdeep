/*
 * Stub para tests unitarios que usan error.cpp.
 * error.cpp llama display_get_error() pero los tests no vinculan el display backend.
 */
#include "display_backend.h"

const char* display_get_error() {
    return "(test stub)";
}
