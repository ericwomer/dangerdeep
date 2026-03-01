/*
 * Test para triangulate.h/cpp: next, is_correct_triangle, is_inside_triangle, compute.
 */
#include "../triangulate.h"
#include <cassert>
#include <cstdio>
#include <vector>

int main() {
    std::vector<unsigned> vl = {0, 1, 2};
    assert(triangulate::next(vl, 0) == 1);
    assert(triangulate::next(vl, 1) == 2);
    assert(triangulate::next(vl, 2) == 0);

    vector2 a(0, 0), b(1, 0), c(0.5, 1);
    assert(triangulate::is_correct_triangle(a, b, c));
    assert(triangulate::is_inside_triangle(a, b, c, vector2(0.5, 0.3)));
    assert(!triangulate::is_inside_triangle(a, b, c, vector2(2, 2)));

    std::vector<vector2> verts;
    verts.push_back(vector2(0, 0));
    verts.push_back(vector2(1, 0));
    verts.push_back(vector2(0.5, 1));
    std::vector<unsigned> tri = triangulate::compute(verts);
    assert(tri.size() == 3);

    printf("triangulate_test ok\n");
    return 0;
}
