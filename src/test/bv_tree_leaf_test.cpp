/*
 * Test para bv_tree.h: leaf_data, param (tipos y métodos básicos).
 */
#include "catch_amalgamated.hpp"
#include "../bv_tree.h"
#include <cmath>
#include <vector>

inline bool near(float a, float b, float eps = 1e-5f) {
    return std::fabs(a - b) <= eps;
}

TEST_CASE("bv_tree::leaf_data - Constructor inicializa tri_idx en -1", "[bv_tree]") {
    bv_tree::leaf_data leaf;
    REQUIRE(leaf.tri_idx[0] == uint32_t(-1));
    REQUIRE(leaf.tri_idx[1] == uint32_t(-1));
    REQUIRE(leaf.tri_idx[2] == uint32_t(-1));
}

TEST_CASE("bv_tree::leaf_data - Triángulo simple centro correcto", "[bv_tree]") {
    std::vector<vector3f> verts;
    verts.push_back(vector3f(0, 0, 0));
    verts.push_back(vector3f(1, 0, 0));
    verts.push_back(vector3f(0.5f, 1, 0));
    
    bv_tree::leaf_data leaf;
    leaf.tri_idx[0] = 0;
    leaf.tri_idx[1] = 1;
    leaf.tri_idx[2] = 2;
    
    vector3f c = leaf.get_center(verts);
    REQUIRE(near(c.x, 0.5f));
    REQUIRE(near(c.y, 1.0f / 3.0f));
    REQUIRE(near(c.z, 0.0f));
}

TEST_CASE("bv_tree::leaf_data - Triángulo equilátero", "[bv_tree]") {
    std::vector<vector3f> verts;
    verts.push_back(vector3f(0, 0, 0));
    verts.push_back(vector3f(2, 0, 0));
    verts.push_back(vector3f(1, std::sqrt(3.0f), 0));
    
    bv_tree::leaf_data leaf;
    leaf.tri_idx[0] = 0;
    leaf.tri_idx[1] = 1;
    leaf.tri_idx[2] = 2;
    
    vector3f c = leaf.get_center(verts);
    REQUIRE(near(c.x, 1.0f));
    REQUIRE(near(c.y, std::sqrt(3.0f) / 3.0f));
    REQUIRE(near(c.z, 0.0f));
}

TEST_CASE("bv_tree::leaf_data - Triángulo en 3D", "[bv_tree]") {
    std::vector<vector3f> verts;
    verts.push_back(vector3f(0, 0, 0));
    verts.push_back(vector3f(3, 0, 0));
    verts.push_back(vector3f(0, 3, 3));
    
    bv_tree::leaf_data leaf;
    leaf.tri_idx[0] = 0;
    leaf.tri_idx[1] = 1;
    leaf.tri_idx[2] = 2;
    
    vector3f c = leaf.get_center(verts);
    REQUIRE(near(c.x, 1.0f));
    REQUIRE(near(c.y, 1.0f));
    REQUIRE(near(c.z, 1.0f));
}

TEST_CASE("bv_tree::leaf_data - Triángulo degenerado (colineal)", "[bv_tree]") {
    std::vector<vector3f> verts;
    verts.push_back(vector3f(0, 0, 0));
    verts.push_back(vector3f(1, 1, 1));
    verts.push_back(vector3f(2, 2, 2));
    
    bv_tree::leaf_data leaf;
    leaf.tri_idx[0] = 0;
    leaf.tri_idx[1] = 1;
    leaf.tri_idx[2] = 2;
    
    vector3f c = leaf.get_center(verts);
    REQUIRE(near(c.x, 1.0f));
    REQUIRE(near(c.y, 1.0f));
    REQUIRE(near(c.z, 1.0f));
}

TEST_CASE("bv_tree::leaf_data - Triángulo con valores negativos", "[bv_tree]") {
    std::vector<vector3f> verts;
    verts.push_back(vector3f(-1, -1, -1));
    verts.push_back(vector3f(1, -1, -1));
    verts.push_back(vector3f(0, 1, -1));
    
    bv_tree::leaf_data leaf;
    leaf.tri_idx[0] = 0;
    leaf.tri_idx[1] = 1;
    leaf.tri_idx[2] = 2;
    
    vector3f c = leaf.get_center(verts);
    REQUIRE(near(c.x, 0.0f));
    REQUIRE(near(c.y, -1.0f / 3.0f));
    REQUIRE(near(c.z, -1.0f));
}

TEST_CASE("bv_tree::leaf_data - Múltiples hojas con mismo vector de vértices", "[bv_tree]") {
    std::vector<vector3f> shared_verts;
    shared_verts.push_back(vector3f(0, 0, 0));
    shared_verts.push_back(vector3f(1, 0, 0));
    shared_verts.push_back(vector3f(1, 1, 0));
    shared_verts.push_back(vector3f(0, 1, 0));
    
    bv_tree::leaf_data leaf_a, leaf_b;
    
    // Primer triángulo: 0-1-2
    leaf_a.tri_idx[0] = 0;
    leaf_a.tri_idx[1] = 1;
    leaf_a.tri_idx[2] = 2;
    vector3f ca = leaf_a.get_center(shared_verts);
    
    // Segundo triángulo: 0-2-3
    leaf_b.tri_idx[0] = 0;
    leaf_b.tri_idx[1] = 2;
    leaf_b.tri_idx[2] = 3;
    vector3f cb = leaf_b.get_center(shared_verts);
    
    // Centros deben ser diferentes
    REQUIRE((!near(ca.x, cb.x) || !near(ca.y, cb.y)));
}
