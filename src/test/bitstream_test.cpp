/*
 * Test para bitstream: roundtrip write/read de bits.
 */
#include "catch_amalgamated.hpp"
#include "../bitstream.h"
#include <sstream>

TEST_CASE("bitstream - write/read 8 y 4 bits", "[bitstream]") {
    std::ostringstream os;
    obitstream ob(&os);
    REQUIRE(ob.write(static_cast<Uint8>(0xFF), static_cast<Uint8>(8)));
    REQUIRE(ob.write(static_cast<Uint8>(0x0F), static_cast<Uint8>(4)));
    ob.last_write();

    std::istringstream is(os.str());
    ibitstream ib(&is);
    REQUIRE(ib.read(8) == 0xFF);
    REQUIRE(ib.read(4) == 0x0F);
}

TEST_CASE("bitstream - write/read 16 bits", "[bitstream]") {
    std::ostringstream os2;
    obitstream ob2(&os2);
    ob2.write(static_cast<Uint16>(0x1234), static_cast<Uint8>(16));
    ob2.last_write();
    std::istringstream is2(os2.str());
    ibitstream ib2(&is2);
    REQUIRE(ib2.read(16) == 0x1234);
}
