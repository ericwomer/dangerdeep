/*
 * Test para frustum.h/cpp: construcción desde polygon + vector3 + znear.
 */
#include "../frustum.h"
#include "../polygon.h"
#include <cassert>
#include <cstdio>

int main() {
    polygon view;
    view.points.push_back(vector3(0, 0, 0));
    view.points.push_back(vector3(1, 0, 0));
    view.points.push_back(vector3(1, 1, 0));
    view.points.push_back(vector3(0, 1, 0));
    vector3 viewp(0.5, 0.5, -10);
    frustum f(view, viewp, 1.0);
    assert(!f.planes.empty());
    assert(f.znear == 1.0);
    printf("frustum_test ok\n");
    return 0;
}
