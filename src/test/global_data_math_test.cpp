/*
 * Test para utilidades matemáticas de global_data.h: myfmod, myfrac, mysgn, myclamp, myinterpolate.
 * Son inline en el header; verificamos comportamiento para evitar regresiones.
 */
#include "catch_amalgamated.hpp"
#include "../global_data.h"
#include <cmath>

TEST_CASE("myfmod - positivo", "[global_data_math]") {
    REQUIRE(myfmod(7.0, 3.0) == Catch::Approx(1.0));
    REQUIRE(myfmod(10.0, 5.0) == Catch::Approx(0.0));
    REQUIRE(myfmod(2.5, 1.0) == Catch::Approx(0.5));
}

TEST_CASE("myfmod - negativo (diferente de std::fmod)", "[global_data_math]") {
    // myfmod(-1, 3) = -1 - floor(-1/3)*3 = -1 - (-1)*3 = 2
    REQUIRE(myfmod(-1.0, 3.0) == Catch::Approx(2.0));
    REQUIRE(myfmod(-4.0, 3.0) == Catch::Approx(2.0));
}

TEST_CASE("myfrac - parte fraccionaria", "[global_data_math]") {
    REQUIRE(myfrac(3.14) == Catch::Approx(0.14));
    REQUIRE(myfrac(1.0) == Catch::Approx(0.0));
    REQUIRE(myfrac(-2.7) == Catch::Approx(0.3)); // -2.7 - (-3) = 0.3
}

TEST_CASE("mysgn - signo", "[global_data_math]") {
    REQUIRE(mysgn(5.0) == 1.0);
    REQUIRE(mysgn(-3.0) == -1.0);
    REQUIRE(mysgn(0.0) == 0.0);
}

TEST_CASE("myclamp - dentro de rango", "[global_data_math]") {
    REQUIRE(myclamp(5, 0, 10) == 5);
    REQUIRE(myclamp(0.5, 0.0, 1.0) == Catch::Approx(0.5));
}

TEST_CASE("myclamp - por debajo del mínimo", "[global_data_math]") {
    REQUIRE(myclamp(-5, 0, 10) == 0);
    REQUIRE(myclamp(-0.5, 0.0, 1.0) == Catch::Approx(0.0));
}

TEST_CASE("myclamp - por encima del máximo", "[global_data_math]") {
    REQUIRE(myclamp(15, 0, 10) == 10);
    REQUIRE(myclamp(1.5, 0.0, 1.0) == Catch::Approx(1.0));
}

TEST_CASE("myinterpolate - extremos", "[global_data_math]") {
    REQUIRE(myinterpolate(0.0, 10.0, 0.0) == Catch::Approx(0.0));
    REQUIRE(myinterpolate(0.0, 10.0, 1.0) == Catch::Approx(10.0));
}

TEST_CASE("myinterpolate - punto medio", "[global_data_math]") {
    REQUIRE(myinterpolate(0.0, 10.0, 0.5) == Catch::Approx(5.0));
    REQUIRE(myinterpolate(10.0, 20.0, 0.5) == Catch::Approx(15.0));
}
