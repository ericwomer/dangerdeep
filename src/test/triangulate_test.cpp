/*
 * Test para triangulate.h/cpp: next, is_correct_triangle, is_inside_triangle, compute.
 */
#include "catch_amalgamated.hpp"
#include "../triangulate.h"
#include "../vector2.h"
#include <vector>

TEST_CASE("triangulate - next en lista simple", "[triangulate]") {
    std::vector<unsigned> vl = {0, 1, 2};
    REQUIRE(triangulate::next(vl, 0) == 1);
    REQUIRE(triangulate::next(vl, 1) == 2);
    REQUIRE(triangulate::next(vl, 2) == 0);
}

TEST_CASE("triangulate - next salta entradas borradas", "[triangulate]") {
    std::vector<unsigned> vl = {0, unsigned(-1), 2, 3};
    REQUIRE(triangulate::next(vl, 0) == 2);
    REQUIRE(triangulate::next(vl, 2) == 3);
    REQUIRE(triangulate::next(vl, 3) == 0);
}

TEST_CASE("triangulate - is_correct_triangle", "[triangulate]") {
    vector2 a(0, 0), b(1, 0), c(0.5, 1);
    REQUIRE(triangulate::is_correct_triangle(a, b, c));
    REQUIRE_FALSE(triangulate::is_correct_triangle(a, c, b));  // CCW vs CW
}

TEST_CASE("triangulate - is_inside_triangle", "[triangulate]") {
    vector2 a(0, 0), b(1, 0), c(0.5, 1);
    REQUIRE(triangulate::is_inside_triangle(a, b, c, vector2(0.5, 0.3)));
    REQUIRE(triangulate::is_inside_triangle(a, b, c, vector2(0.5, 0.5)));
    REQUIRE_FALSE(triangulate::is_inside_triangle(a, b, c, vector2(2, 2)));
    REQUIRE_FALSE(triangulate::is_inside_triangle(a, b, c, vector2(-1, -1)));
}

TEST_CASE("triangulate - compute triángulo simple", "[triangulate]") {
    std::vector<vector2> verts = {
        vector2(0, 0),
        vector2(1, 0),
        vector2(0.5, 1)
    };
    std::vector<unsigned> tri = triangulate::compute(verts);
    REQUIRE(tri.size() == 3);
    REQUIRE(tri[0] < 3);
    REQUIRE(tri[1] < 3);
    REQUIRE(tri[2] < 3);
}

TEST_CASE("triangulate - compute polígono cuadrado", "[triangulate]") {
    std::vector<vector2> verts = {
        vector2(0, 0),
        vector2(1, 0),
        vector2(1, 1),
        vector2(0, 1)
    };
    std::vector<unsigned> tri = triangulate::compute(verts);
    REQUIRE(tri.size() == 6);  // 2 triángulos
}

TEST_CASE("triangulate - compute pentágono", "[triangulate]") {
    // Pentágono regular aproximado (centro en 0.5, 0.5)
    std::vector<vector2> verts = {
        vector2(0.5, 0.0),
        vector2(0.95, 0.35),
        vector2(0.77, 0.9),
        vector2(0.23, 0.9),
        vector2(0.05, 0.35)
    };
    std::vector<unsigned> tri = triangulate::compute(verts);
    REQUIRE(tri.size() == 9);  // 3 triángulos
    for (unsigned i = 0; i < tri.size(); ++i) {
        REQUIRE(tri[i] < 5);
    }
}

TEST_CASE("triangulate - compute hexágono", "[triangulate]") {
    std::vector<vector2> verts = {
        vector2(0.5, 0.0),
        vector2(0.93, 0.25),
        vector2(0.93, 0.75),
        vector2(0.5, 1.0),
        vector2(0.07, 0.75),
        vector2(0.07, 0.25)
    };
    std::vector<unsigned> tri = triangulate::compute(verts);
    REQUIRE(tri.size() == 12);  // 4 triángulos
    for (unsigned i = 0; i < tri.size(); ++i) {
        REQUIRE(tri[i] < 6);
    }
}

TEST_CASE("triangulate - compute menos de 3 vértices retorna vacío", "[triangulate]") {
    std::vector<vector2> empty;
    REQUIRE(triangulate::compute(empty).empty());

    std::vector<vector2> one = {vector2(0, 0)};
    REQUIRE(triangulate::compute(one).empty());

    std::vector<vector2> two = {vector2(0, 0), vector2(1, 0)};
    REQUIRE(triangulate::compute(two).empty());
}
