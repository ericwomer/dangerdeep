/*
 * Test para bv_tree.h: leaf_data, param (tipos y métodos básicos).
 */
#include "../bv_tree.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <vector>

static bool near(float a, float b, float eps = 1e-5f) {
    return std::fabs(a - b) <= eps;
}

int main() {
    // Test 1: Constructor inicializa tri_idx en -1
    bv_tree::leaf_data leaf;
    assert(leaf.tri_idx[0] == uint32_t(-1));
    assert(leaf.tri_idx[1] == uint32_t(-1));
    assert(leaf.tri_idx[2] == uint32_t(-1));
    
    // Test 2: Triángulo simple - centro correcto
    std::vector<vector3f> verts;
    verts.push_back(vector3f(0, 0, 0));
    verts.push_back(vector3f(1, 0, 0));
    verts.push_back(vector3f(0.5f, 1, 0));
    
    leaf.tri_idx[0] = 0;
    leaf.tri_idx[1] = 1;
    leaf.tri_idx[2] = 2;
    
    vector3f c = leaf.get_center(verts);
    assert(near(c.x, 0.5f));
    assert(near(c.y, 1.0f / 3.0f));
    assert(near(c.z, 0.0f));
    
    // Test 3: Triángulo equilátero
    std::vector<vector3f> verts2;
    verts2.push_back(vector3f(0, 0, 0));
    verts2.push_back(vector3f(2, 0, 0));
    verts2.push_back(vector3f(1, std::sqrt(3.0f), 0));
    
    bv_tree::leaf_data leaf2;
    leaf2.tri_idx[0] = 0;
    leaf2.tri_idx[1] = 1;
    leaf2.tri_idx[2] = 2;
    
    vector3f c2 = leaf2.get_center(verts2);
    assert(near(c2.x, 1.0f));
    assert(near(c2.y, std::sqrt(3.0f) / 3.0f));
    assert(near(c2.z, 0.0f));
    
    // Test 4: Triángulo en 3D
    std::vector<vector3f> verts3;
    verts3.push_back(vector3f(0, 0, 0));
    verts3.push_back(vector3f(3, 0, 0));
    verts3.push_back(vector3f(0, 3, 3));
    
    bv_tree::leaf_data leaf3;
    leaf3.tri_idx[0] = 0;
    leaf3.tri_idx[1] = 1;
    leaf3.tri_idx[2] = 2;
    
    vector3f c3 = leaf3.get_center(verts3);
    assert(near(c3.x, 1.0f));
    assert(near(c3.y, 1.0f));
    assert(near(c3.z, 1.0f));
    
    // Test 5: Triángulo degenerado (colineal)
    std::vector<vector3f> verts4;
    verts4.push_back(vector3f(0, 0, 0));
    verts4.push_back(vector3f(1, 1, 1));
    verts4.push_back(vector3f(2, 2, 2));
    
    bv_tree::leaf_data leaf4;
    leaf4.tri_idx[0] = 0;
    leaf4.tri_idx[1] = 1;
    leaf4.tri_idx[2] = 2;
    
    vector3f c4 = leaf4.get_center(verts4);
    assert(near(c4.x, 1.0f));
    assert(near(c4.y, 1.0f));
    assert(near(c4.z, 1.0f));
    
    // Test 6: Triángulo con valores negativos
    std::vector<vector3f> verts5;
    verts5.push_back(vector3f(-1, -1, -1));
    verts5.push_back(vector3f(1, -1, -1));
    verts5.push_back(vector3f(0, 1, -1));
    
    bv_tree::leaf_data leaf5;
    leaf5.tri_idx[0] = 0;
    leaf5.tri_idx[1] = 1;
    leaf5.tri_idx[2] = 2;
    
    vector3f c5 = leaf5.get_center(verts5);
    assert(near(c5.x, 0.0f));
    assert(near(c5.y, -1.0f / 3.0f));
    assert(near(c5.z, -1.0f));
    
    // Test 7: Múltiples hojas con mismo vector de vértices
    std::vector<vector3f> shared_verts;
    shared_verts.push_back(vector3f(0, 0, 0));  // 0
    shared_verts.push_back(vector3f(1, 0, 0));  // 1
    shared_verts.push_back(vector3f(1, 1, 0));  // 2
    shared_verts.push_back(vector3f(0, 1, 0));  // 3
    
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
    assert(!near(ca.x, cb.x) || !near(ca.y, cb.y));
    
    printf("bv_tree_leaf_test ok (7 tests)\n");
    return 0;
}
