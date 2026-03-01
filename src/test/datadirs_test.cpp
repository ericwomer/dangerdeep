/*
 * Test para datadirs.h/cpp: set_data_dir, get_data_dir, get_texture_dir, etc.
 */
#include "../datadirs.h"
#include <cassert>
#include <cstdio>
#include <string>

int main() {
    set_data_dir("/tmp/test_data");
    assert(get_data_dir() == "/tmp/test_data");
    std::string tex = get_texture_dir();
    assert(tex.find("textures") != std::string::npos);
    assert(get_font_dir().find("fonts") != std::string::npos);
    assert(get_model_dir().find("models") != std::string::npos);
    assert(get_sound_dir().find("sounds") != std::string::npos);
    printf("datadirs_test ok\n");
    return 0;
}
