/*
 * Test para binstream.h: roundtrip write/read con stringstream.
 */
#include "../binstream.h"
#include <cassert>
#include <cstdio>
#include <sstream>

int main() {
    std::stringstream buf;

    write_i32(buf, -12345);
    write_u32(buf, 40000u);
    write_float(buf, 3.14f);
    write_double(buf, -2.5);
    write_bool(buf, true);
    write_bool(buf, false);
    write_string(buf, "hello");
    write_string(buf, "");

    buf.seekg(0);
    assert(read_i32(buf) == -12345);
    assert(read_u32(buf) == 40000u);
    assert(std::fabs(read_float(buf) - 3.14f) < 1e-5f);
    assert(std::fabs(read_double(buf) - (-2.5)) < 1e-12);
    assert(read_bool(buf) == true);
    assert(read_bool(buf) == false);
    assert(read_string(buf) == "hello");
    assert(read_string(buf).empty());

    std::stringstream buf2;
    vector2 v2(1.0, 2.0);
    write_vector2(buf2, v2);
    buf2.seekg(0);
    vector2 v2r = read_vector2(buf2);
    assert(std::fabs(v2r.x - 1.0) < 1e-12 && std::fabs(v2r.y - 2.0) < 1e-12);

    printf("binstream_test ok\n");
    return 0;
}
