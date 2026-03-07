/* Test bv_tree.h/cpp: create() from vertices + leaf_data. */
#include "catch_amalgamated.hpp"
#include "../bv_tree.h"
#include <list>
#include <vector>

TEST_CASE("bv_tree - create desde vertices y leaf_data", "[bv_tree]") {
    std::vector<vector3f> verts = {
        vector3f(0, 0, 0),
        vector3f(1, 0, 0),
        vector3f(0.5f, 1, 0)
    };
    std::list<bv_tree::leaf_data> leaves;
    bv_tree::leaf_data l;
    l.tri_idx[0] = 0;
    l.tri_idx[1] = 1;
    l.tri_idx[2] = 2;
    leaves.push_back(l);
    std::unique_ptr<bv_tree> tree = bv_tree::create(verts, leaves);
    REQUIRE(tree.get() != nullptr);
}
