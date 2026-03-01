/*
 * Test para bitstream: roundtrip write/read de bits.
 */
#include "../bitstream.h"
#include <cassert>
#include <cstdio>
#include <sstream>

int main() {
    std::ostringstream os;
    obitstream ob(&os);
    assert(ob.write(static_cast<Uint8>(0xFF), static_cast<Uint8>(8)));
    assert(ob.write(static_cast<Uint8>(0x0F), static_cast<Uint8>(4)));
    ob.last_write();

    std::istringstream is(os.str());
    ibitstream ib(&is);
    Uint16 v = ib.read(8);
    assert(v == 0xFF);
    v = ib.read(4);
    assert(v == 0x0F);

    std::ostringstream os2;
    obitstream ob2(&os2);
    ob2.write(static_cast<Uint16>(0x1234), static_cast<Uint8>(16));
    ob2.last_write();
    std::istringstream is2(os2.str());
    ibitstream ib2(&is2);
    assert(ib2.read(16) == 0x1234);

    printf("bitstream_test ok\n");
    return 0;
}
