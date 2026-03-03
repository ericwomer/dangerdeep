/*
 * Test para matrix.h: matrix_swap_rows, matrix_invert, matrixt.
 */
#include "catch_amalgamated.hpp"
#include "../matrix.h"
#include <cmath>

inline bool near(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) <= eps;
}

TEST_CASE("matrix - Inversión matriz identidad 2x2", "[matrix]") {
    double m2_id[4] = {1, 0, 0, 1};
    matrix_invert<double, 2>(m2_id);
    
    REQUIRE(near(m2_id[0], 1));
    REQUIRE(near(m2_id[3], 1));
    REQUIRE(near(m2_id[1], 0));
    REQUIRE(near(m2_id[2], 0));
}

TEST_CASE("matrix - Inversión matriz diagonal", "[matrix]") {
    double m2_diag[4] = {2, 0, 0, 2};
    matrix_invert<double, 2>(m2_diag);
    
    REQUIRE(near(m2_diag[0], 0.5));
    REQUIRE(near(m2_diag[3], 0.5));
}

TEST_CASE("matrix - Intercambio de filas", "[matrix]") {
    double m2_swap[4] = {1, 2, 3, 4};
    matrix_swap_rows<double, 2>(m2_swap, 0, 1);
    
    REQUIRE(near(m2_swap[0], 3));
    REQUIRE(near(m2_swap[1], 4));
    REQUIRE(near(m2_swap[2], 1));
    REQUIRE(near(m2_swap[3], 2));
}

TEST_CASE("matrixt - Matriz identidad", "[matrix]") {
    matrixt<double, 2> i2 = matrixt<double, 2>::one();
    
    REQUIRE(near(i2.elem(0, 0), 1));
    REQUIRE(near(i2.elem(1, 1), 1));
    REQUIRE(near(i2.elem(0, 1), 0));
    REQUIRE(near(i2.elem(1, 0), 0));
}

TEST_CASE("matrixt - Transpuesta", "[matrix]") {
    matrixt<double, 2> a;
    a.elem(0, 0) = 1;
    a.elem(1, 0) = 2;
    a.elem(0, 1) = 3;
    a.elem(1, 1) = 4;
    
    matrixt<double, 2> at = a.transpose();
    
    REQUIRE(near(at.elem(0, 0), 1));
    REQUIRE(near(at.elem(0, 1), 2));
    REQUIRE(near(at.elem(1, 0), 3));
    REQUIRE(near(at.elem(1, 1), 4));
}

TEST_CASE("matrixt - Escalado por escalar", "[matrix]") {
    matrixt<double, 2> i2 = matrixt<double, 2>::one();
    matrixt<double, 2> i2x2 = i2 * 3.0;
    
    REQUIRE(near(i2x2.elem(0, 0), 3));
    REQUIRE(near(i2x2.elem(1, 1), 3));
}

TEST_CASE("matrixt - Suma de matrices", "[matrix]") {
    matrixt<double, 2> i2 = matrixt<double, 2>::one();
    matrixt<double, 2> sum = i2 + i2;
    
    REQUIRE(near(sum.elem(0, 0), 2));
    REQUIRE(near(sum.elem(1, 1), 2));
}
