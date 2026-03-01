/*
 * Test para bv_tree.h: leaf_data, param (tipos y métodos básicos).
 */
#include "../bv_tree.h"
#include <cassert>
#include <cstdio>
#include <vector>

int main() {
    bv_tree::leaf_data leaf;
    assert(leaf.tri_idx[0] == uint32_t(-1) && leaf.tri_idx[1] == uint32_t(-1));

    std::vector<vector3f> verts;
    verts.push_back(vector3f(0, 0, 0));
    verts.push_back(vector3f(1, 0, 0));
    verts.push_back(vector3f(0.5f, 1, 0));
    leaf.tri_idx[0] = 0;
    leaf.tri_idx[1] = 1;
    leaf.tri_idx[2] = 2;
    vector3f c = leaf.get_center(verts);
    assert(c.x > 0.4f && c.x < 0.6f && c.z == 0);

    printf("bv_tree_leaf_test ok\n");
    return 0;
}
