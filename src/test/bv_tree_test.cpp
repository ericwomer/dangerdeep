/* Test bv_tree.h/cpp: create() from vertices + leaf_data. */
#include "../bv_tree.h"
#include <cassert>
#include <cstdio>
#include <list>
#include <vector>
int main() {
    std::vector<vector3f> verts;
    verts.push_back(vector3f(0,0,0));
    verts.push_back(vector3f(1,0,0));
    verts.push_back(vector3f(0.5f,1,0));
    std::list<bv_tree::leaf_data> leaves;
    bv_tree::leaf_data l;
    l.tri_idx[0]=0; l.tri_idx[1]=1; l.tri_idx[2]=2;
    leaves.push_back(l);
    std::unique_ptr<bv_tree> tree = bv_tree::create(verts, leaves);
    assert(tree.get() != nullptr);
    printf("bv_tree_test ok\n");
    return 0;
}
