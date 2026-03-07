/*
 * Test para datadirs.h/cpp: set_data_dir, get_data_dir, get_texture_dir, etc.
 */
#include "catch_amalgamated.hpp"
#include "../datadirs.h"
#include <string>

TEST_CASE("datadirs - set_data_dir y get_data_dir", "[datadirs]") {
    set_data_dir("/tmp/test_data");
    REQUIRE(get_data_dir() == "/tmp/test_data");
}

TEST_CASE("datadirs - get_texture_dir, get_font_dir, get_model_dir, get_sound_dir", "[datadirs]") {
    set_data_dir("/tmp/test_data");
    REQUIRE(get_texture_dir().find("textures") != std::string::npos);
    REQUIRE(get_font_dir().find("fonts") != std::string::npos);
    REQUIRE(get_model_dir().find("models") != std::string::npos);
    REQUIRE(get_sound_dir().find("sounds") != std::string::npos);
}
