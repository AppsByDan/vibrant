#include "test-common.h"

#ifndef VIBRANT_NO_PARSE

static const char* long_string(void);
static const char* capitalize(const char* str);

TEST(vbt_parse_basic) {
  const char* input[] = {
      "#fff",
      "#ffff",
      "#ffffff",
      "#ffffffff",
      "rgb(255, 255, 255)",
      "rgb(255,255,255)",
      "rgb(255 255 255)",
      "rgb(255 255 255 / 1)",
      "rgb(255 255 255/1)",
      "rgb(255 255 255 / 100%)",
      "rgb(255, 255, 255 / 1)",
      "rgb(255,255,255/1)",
      "rgb(100%, 100%, 100%)",
      "rgba(255, 255, 255, 1)",
      "rgba(255 255 255 1)",
      "rgba(100%, 100%, 100%, 100%)",
      "hsl(0, 0%, 100%)",
      "hsl(0, 0, 100)",
      "hsl(0 0 100)",
      "hsl(0 0 100 / 1)",
      "hsla(0, 0%, 100%, 100%)",
      "hsla(0, 0, 100, 1)",
      "hsla(0 0 100 1)",
      "hwb(0, 100%, 0%)",
      "hwb(0, 100, 0)",
      "hwb(0 100 0)",
      "hwb(0 100 0 / 1)",
      "hwba(0, 100%, 0%, 100%)",
      "hwba(0, 100, 0, 1)",
      "hwba(0 100 0 1)",
      "white",
  };

  for (size_t i = 0; i < vu_arr_len(input); i++) {
    CASE(input[i]) {
      vbt_recv_t recv = vbt_recv_init();
      int err = vbt_parse(input[i], strlen(input[i]), &recv);
      ASSERT_RECV_U8(err, recv, 255, 255, 255, 255);
    }
  }
}

TEST(vbt_parse_invalid_args) {
  const char* in = "#fff";
  vbt_recv_t recv = vbt_recv_init();
  int err;

  CASE("recv = NULL") {
    err = vbt_parse(in, strlen(in), NULL);
    ASSERT_EQ(err, VBT_ERR);
  }

  CASE("value = NULL") {
    err = vbt_parse(NULL, 0, &recv);
    ASSERT_EQ(err, VBT_ERR);
  }

  CASE("value, len, recv = 0") {
    err = vbt_parse(NULL, 0, NULL);
    ASSERT_EQ(err, VBT_ERR);
  }

  CASE("len = 0") {
    err = vbt_parse(in, 0, &recv);
    ASSERT_EQ(err, VBT_ERR);
  }

  CASE("empty string") {
    err = vbt_parse("", 0, &recv);
    ASSERT_EQ(err, VBT_ERR);
  }

  CASE("long string") {
    in = long_string();
    err = vbt_parse(in, strlen(in), &recv);
    ASSERT_EQ(err, VBT_ERR);
  }
}

TEST(vbt_parse_z_invalid_args) {
  const char* in = "#fff";
  vbt_recv_t recv = vbt_recv_init();
  int err;

  CASE("recv = NULL") {
    err = vbt_parse_z(in, NULL);
    ASSERT_EQ(err, VBT_ERR);
  }

  CASE("value = NULL") {
    err = vbt_parse_z(NULL, &recv);
    ASSERT_EQ(err, VBT_ERR);
  }

  CASE("value, recv = 0") {
    err = vbt_parse_z(NULL, NULL);
    ASSERT_EQ(err, VBT_ERR);
  }

  CASE("empty string") {
    err = vbt_parse_z("", &recv);
    ASSERT_EQ(err, VBT_ERR);
  }

  CASE("long string") {
    in = long_string();
    err = vbt_parse_z(in, &recv);
    ASSERT_EQ(err, VBT_ERR);
  }
}

TEST(vbt_parse_z_parse_number_limits) {
  const char* in;
  vbt_recv_t recv = vbt_recv_init();
  int err;

  CASE("maximum digits after decimal") {
    in = "hsl(119.999999999, 50.000000001%, 50%)";
    err = vbt_parse_z(in, &recv);
    ASSERT_RECV_U8(err, recv, 64, 191, 64, 255);
  }

  CASE("max int (16777216 mods to 136deg)") {
    in = "hsl(16777216, 50%, 50%)";
    err = vbt_parse_z(in, &recv);
    ASSERT_RECV_U8(err, recv, 64, 191, 98, 255);
  }

  CASE("maxxed") {
    in = "hsl(16777216.999999999, 50%, 50%)";
    err = vbt_parse_z(in, &recv);
    ASSERT_RECV_U8(err, recv, 64, 191, 98, 255);
  }
}

TEST(vbt_parse_z_parse_err) {
  const char* in;
  vbt_recv_t recv = vbt_recv_init();

  CASE("empty string") {
    in = "";
    ASSERT_EQ(vbt_parse_z(in, &recv), VBT_ERR);
  }

  CASE("not a valid css color") {
    in = "unknown";
    ASSERT_EQ(vbt_parse_z(in, &recv), VBT_ERR);
  }

  CASE("# - no digits") {
    in = "#";
    ASSERT_EQ(vbt_parse_z(in, &recv), VBT_ERR);
  }

  CASE("# - too short") {
    in = "#f";
    ASSERT_EQ(vbt_parse_z(in, &recv), VBT_ERR);
  }

  CASE("# - too long") {
    in = "#ffffffffffff";
    ASSERT_EQ(vbt_parse_z(in, &recv), VBT_ERR);
  }

  CASE("# - space after") {
    in = "#fff ;";
    ASSERT_EQ(vbt_parse_z(in, &recv), VBT_ERR);
  }

  // clang-format off
  // go through failure points of hex parsing
  const char* hex_in[] = {
      "#!!!",
      "#f!!",
      "#ff!",
      "#!!!!",
      "#f!!!",
      "#ff!!",
      "#fff!",
      "#!!!!!!",
      "#f!!!!!",
      "#ff!!!!",
      "#fff!!!",
      "#ffff!!",
      "#fffff!",
      "#!!!!!!!!",
      "#f!!!!!!!",
      "#ff!!!!!!",
      "#fff!!!!!",
      "#ffff!!!!",
      "#fffff!!!",
      "#ffffff!!",
      "#fffffff!",
  };
  // clang-format on

  for (size_t i = 0; i < vu_arr_len(hex_in); i++) {
    CASE(hex_in[i]) {
      ASSERT_EQ(vbt_parse_z(hex_in[i], &recv), VBT_ERR);
    }
  }

  CASE("unknown function name") {
    in = "xxx(0, 0, 0)";
    ASSERT_EQ(vbt_parse_z(in, &recv), VBT_ERR);
  }

  // argument parsing
  const char* params_in[] = {
      "rgb()",
      "rgb(0)",
      "rgb(0, 0)",
      "rgb(0, 0, 0, 0)",
      "rgb(0, 0, 0, 0, 0)",
      "rgb(x)",
      "rgb(x, x, x)",
      "rbg(0, x, x)",
      "rgb(0, 0, x)",
      "rgb(0, 0, 0,)",
      "rgb(0, 0, 0x)",
      "rgb(0, 0x, 0x)",
      "rgb(0x, 0, 0x)",
      "rgba(0, 0, 0)",
      "rgba(0, 0, 0, 0, 0)",
      "rgba(0, 0, 0 / 0)",
      "rgb",
      "rgb(",
      "rgb(0",
      "rgb(0,",
      "rgb(0, 0",
      "rgb(0, 0,",
      "rgb(0, 0, 0",
      "rgb(0, 0, 0 /",
  };

  for (size_t i = 0; i < vu_arr_len(params_in); i++) {
    CASE(params_in[i]) {
      ASSERT_EQ(vbt_parse_z(params_in[i], &recv), VBT_ERR);
    }
  }

  CASE("more than 9 digits after decimal") {
    in = "hsl(119.9999999999, 50%, 50%)";
    ASSERT_EQ(vbt_parse_z(in, &recv), VBT_ERR);
  }

  CASE("exceeds int max") {
    in = "hsl(16777217, 50%, 50%)";
    ASSERT_EQ(vbt_parse_z(in, &recv), VBT_ERR);
  }

  CASE("invalid type after int") {
    in = "hsl(12a, 50%, 50%)";
    ASSERT_EQ(vbt_parse_z(in, &recv), VBT_ERR);
  }

  CASE("invalid number") {
    in = "hsl(a, 50%, 50%)";
    ASSERT_EQ(vbt_parse_z(in, &recv), VBT_ERR);
  }

  CASE("invalid type after number") {
    in = "hsl(12.1a, 50%, 50%)";
    ASSERT_EQ(vbt_parse_z(in, &recv), VBT_ERR);
  }

  CASE("invalid type after number 2") {
    in = "hsl(12.a, 50%, 50%)";
    ASSERT_EQ(vbt_parse_z(in, &recv), VBT_ERR);
  }
}

TEST(vbt_parse_hex) {
  const char* in;
  vbt_recv_t recv = vbt_recv_init();
  int err;

  CASE("lower3 - distinct components") {
    in = "#2ae";
    err = vbt_parse_z(in, &recv);
    ASSERT_RECV_U8(err, recv, 0x22, 0xaa, 0xee, 0xff);
  }

  CASE("lower4 - distinct components") {
    in = "#22aaee";
    err = vbt_parse_z(in, &recv);
    ASSERT_RECV_U8(err, recv, 0x22, 0xaa, 0xee, 0xff);
  }

  CASE("upper3 - distinct components") {
    in = "#2AE";
    err = vbt_parse_z(in, &recv);
    ASSERT_RECV_U8(err, recv, 0x22, 0xaa, 0xee, 0xff);
  }

  CASE("upper4 - distinct components") {
    in = "#22AAEE";
    err = vbt_parse_z(in, &recv);
    ASSERT_RECV_U8(err, recv, 0x22, 0xaa, 0xee, 0xff);
  }
}

typedef struct {
  const char* name;
  const char* hex_value;
} css_color;

TEST(vbt_parse_color_name_parse) {
  static const css_color colors[] = {
      {"aliceblue", "#f0f8ff"},
      {"antiquewhite", "#faebd7"},
      {"aqua", "#00ffff"},
      {"aquamarine", "#7fffd4"},
      {"azure", "#f0ffff"},
      {"beige", "#f5f5dc"},
      {"bisque", "#ffe4c4"},
      {"black", "#000000"},
      {"blanchedalmond", "#ffebcd"},
      {"blue", "#0000ff"},
      {"blueviolet", "#8a2be2"},
      {"brown", "#a52a2a"},
      {"burlywood", "#deb887"},
      {"cadetblue", "#5f9ea0"},
      {"chartreuse", "#7fff00"},
      {"chocolate", "#d2691e"},
      {"coral", "#ff7f50"},
      {"cornflowerblue", "#6495ed"},
      {"cornsilk", "#fff8dc"},
      {"crimson", "#dc143c"},
      {"cyan", "#00ffff"},
      {"darkblue", "#00008b"},
      {"darkcyan", "#008b8b"},
      {"darkgoldenrod", "#b8860b"},
      {"darkgray", "#a9a9a9"},
      {"darkgreen", "#006400"},
      {"darkgrey", "#a9a9a9"},
      {"darkkhaki", "#bdb76b"},
      {"darkmagenta", "#8b008b"},
      {"darkolivegreen", "#556b2f"},
      {"darkorange", "#ff8c00"},
      {"darkorchid", "#9932cc"},
      {"darkred", "#8b0000"},
      {"darksalmon", "#e9967a"},
      {"darkseagreen", "#8fbc8f"},
      {"darkslateblue", "#483d8b"},
      {"darkslategray", "#2f4f4f"},
      {"darkslategrey", "#2f4f4f"},
      {"darkturquoise", "#00ced1"},
      {"darkviolet", "#9400d3"},
      {"deeppink", "#ff1493"},
      {"deepskyblue", "#00bfff"},
      {"dimgray", "#696969"},
      {"dimgrey", "#696969"},
      {"dodgerblue", "#1e90ff"},
      {"firebrick", "#b22222"},
      {"floralwhite", "#fffaf0"},
      {"forestgreen", "#228b22"},
      {"fuchsia", "#ff00ff"},
      {"gainsboro", "#dcdcdc"},
      {"ghostwhite", "#f8f8ff"},
      {"goldenrod", "#daa520"},
      {"gold", "#ffd700"},
      {"gray", "#808080"},
      {"green", "#008000"},
      {"greenyellow", "#adff2f"},
      {"grey", "#808080"},
      {"honeydew", "#f0fff0"},
      {"hotpink", "#ff69b4"},
      {"indianred", "#cd5c5c"},
      {"indigo", "#4b0082"},
      {"ivory", "#fffff0"},
      {"khaki", "#f0e68c"},
      {"lavenderblush", "#fff0f5"},
      {"lavender", "#e6e6fa"},
      {"lawngreen", "#7cfc00"},
      {"lemonchiffon", "#fffacd"},
      {"lightblue", "#add8e6"},
      {"lightcoral", "#f08080"},
      {"lightcyan", "#e0ffff"},
      {"lightgoldenrodyellow", "#fafad2"},
      {"lightgray", "#d3d3d3"},
      {"lightgreen", "#90ee90"},
      {"lightgrey", "#d3d3d3"},
      {"lightpink", "#ffb6c1"},
      {"lightsalmon", "#ffa07a"},
      {"lightseagreen", "#20b2aa"},
      {"lightskyblue", "#87cefa"},
      {"lightslategray", "#778899"},
      {"lightslategrey", "#778899"},
      {"lightsteelblue", "#b0c4de"},
      {"lightyellow", "#ffffe0"},
      {"lime", "#00ff00"},
      {"limegreen", "#32cd32"},
      {"linen", "#faf0e6"},
      {"magenta", "#ff00ff"},
      {"maroon", "#800000"},
      {"mediumaquamarine", "#66cdaa"},
      {"mediumblue", "#0000cd"},
      {"mediumorchid", "#ba55d3"},
      {"mediumpurple", "#9370db"},
      {"mediumseagreen", "#3cb371"},
      {"mediumslateblue", "#7b68ee"},
      {"mediumspringgreen", "#00fa9a"},
      {"mediumturquoise", "#48d1cc"},
      {"mediumvioletred", "#c71585"},
      {"midnightblue", "#191970"},
      {"mintcream", "#f5fffa"},
      {"mistyrose", "#ffe4e1"},
      {"moccasin", "#ffe4b5"},
      {"navajowhite", "#ffdead"},
      {"navy", "#000080"},
      {"oldlace", "#fdf5e6"},
      {"olive", "#808000"},
      {"olivedrab", "#6b8e23"},
      {"orange", "#ffa500"},
      {"orangered", "#ff4500"},
      {"orchid", "#da70d6"},
      {"palegoldenrod", "#eee8aa"},
      {"palegreen", "#98fb98"},
      {"paleturquoise", "#afeeee"},
      {"palevioletred", "#db7093"},
      {"papayawhip", "#ffefd5"},
      {"peachpuff", "#ffdab9"},
      {"peru", "#cd853f"},
      {"pink", "#ffc0cb"},
      {"plum", "#dda0dd"},
      {"powderblue", "#b0e0e6"},
      {"purple", "#800080"},
      {"rebeccapurple", "#663399"},
      {"red", "#ff0000"},
      {"rosybrown", "#bc8f8f"},
      {"royalblue", "#4169e1"},
      {"saddlebrown", "#8b4513"},
      {"salmon", "#fa8072"},
      {"sandybrown", "#f4a460"},
      {"seagreen", "#2e8b57"},
      {"seashell", "#fff5ee"},
      {"sienna", "#a0522d"},
      {"silver", "#c0c0c0"},
      {"skyblue", "#87ceeb"},
      {"slateblue", "#6a5acd"},
      {"slategray", "#708090"},
      {"slategrey", "#708090"},
      {"snow", "#fffafa"},
      {"springgreen", "#00ff7f"},
      {"steelblue", "#4682b4"},
      {"tan", "#d2b48c"},
      {"teal", "#008080"},
      {"thistle", "#d8bfd8"},
      {"tomato", "#ff6347"},
      {"transparent", "#00000000"},
      {"turquoise", "#40e0d0"},
      {"violet", "#ee82ee"},
      {"wheat", "#f5deb3"},
      {"white", "#ffffff"},
      {"whitesmoke", "#f5f5f5"},
      {"yellow", "#ffff00"},
      {"yellowgreen", "#9acd32"},
  };

  for (size_t i = 0; i < sizeof(colors) / sizeof(*colors); i++) {
    CASE(colors[i].name) {
      vbt_recv_t recv;
      vbt_recv_t expected = vbt_recv_init();
      int err;

      // expected values
      ASSERT_EQ(vbt_parse_z(colors[i].hex_value, &expected), VBT_SUCCESS);

      // try lowercase
      recv = vbt_recv_init();
      err = vbt_parse_z(colors[i].name, &recv);
      ASSERT_RECV_U8(err, recv, expected.u.val.u8.r, expected.u.val.u8.g,
                     expected.u.val.u8.b, expected.u.val.u8.a);

      // try uppercase
      recv = vbt_recv_init();
      err = vbt_parse_z(capitalize(colors[i].name), &recv);
      ASSERT_RECV_U8(err, recv, expected.u.val.u8.r, expected.u.val.u8.g,
                     expected.u.val.u8.b, expected.u.val.u8.a);
    }
  }
}

TEST(vbt_parse_lch) {
  // clang-format off
  const char* input[] = {
      "lch(53.23 104.55 40)",
      "lch(53.23%, 104.55, 40)",
      "lch(53.23 104.55 40 / 1)",
      "lch(53.23% 104.55 40 / 100%)",
      "lcha(53.23, 104.55, 40, 1)",
      "lcha(53.23% 104.55 40 1)",
  };
  // clang-format on

  for (size_t i = 0; i < vu_arr_len(input); i++) {
    CASE(input[i]) {
      vbt_recv_t recv = vbt_recv_init();
      int err = vbt_parse(input[i], strlen(input[i]), &recv);
      ASSERT_RECV_U8(err, recv, 255, 0, 0, 255);
    }
  }
}

TEST(vbt_parse_lab) {
  // clang-format off
  const char* red_input[] = {
      "lab(53.23 80.11 67.22)",
      "lab(53.23%, 80.11, 67.22)",
      "lab(53.23 80.11 67.22 / 1)",
      "lab(53.23% 80.11 67.22 / 100%)",
      "laba(53.23, 80.11, 67.22, 1)",
      "laba(53.23% 80.11 67.22 1)",
  };
  // clang-format on

  for (size_t i = 0; i < vu_arr_len(red_input); i++) {
    CASE(red_input[i]) {
      vbt_recv_t recv = vbt_recv_init();
      int err = vbt_parse(red_input[i], strlen(red_input[i]), &recv);
      ASSERT_RECV_U8(err, recv, 255, 0, 0, 255);
    }
  }

  const char* green_input[] = {
      "lab(87.73 -86.18 83.18)",
      "lab(87.73% -86.18 83.18 / 1)",
      "laba(87.73, -86.18, 83.18, 1)",
  };

  for (size_t i = 0; i < vu_arr_len(green_input); i++) {
    CASE(green_input[i]) {
      vbt_recv_t recv = vbt_recv_init();
      int err = vbt_parse(green_input[i], strlen(green_input[i]), &recv);
      ASSERT_RECV_U8(err, recv, 0, 255, 0, 255);
    }
  }

  const char* blue_input[] = {
      "lab(32.3 79.19 -107.86)",
      "lab(32.3% 79.19 -107.86 / 1)",
      "laba(32.3, 79.19, -107.86, 1)",
  };

  for (size_t i = 0; i < vu_arr_len(blue_input); i++) {
    CASE(blue_input[i]) {
      vbt_recv_t recv = vbt_recv_init();
      int err = vbt_parse(blue_input[i], strlen(blue_input[i]), &recv);
      ASSERT_RECV_U8(err, recv, 0, 0, 255, 255);
    }
  }

  const char* gray_input[] = {
      "lab(53.59 0 0)",
      "lab(53.59% 0 0 / 1)",
      "laba(53.59, 0, 0, 1)",
  };

  for (size_t i = 0; i < vu_arr_len(gray_input); i++) {
    CASE(gray_input[i]) {
      vbt_recv_t recv = vbt_recv_init();
      int err = vbt_parse(gray_input[i], strlen(gray_input[i]), &recv);
      ASSERT_RECV_U8(err, recv, 128, 128, 128, 255);
    }
  }
}

TEST(vbt_parse_oklch) {
  const char* red_input[] = {
      "oklch(0.627955 0.25766 29.233)",
      "oklch(62.7955% 0.25766 29.233)",
      "oklch(0.627955 0.25766 29.233 / 1)",
      "oklch(62.7955% 0.25766 29.233 / 100%)",
      "oklcha(0.627955, 0.25766, 29.233, 1)",
      "oklcha(62.7955% 0.25766 29.233 1)",
  };

  for (size_t i = 0; i < vu_arr_len(red_input); i++) {
    CASE(red_input[i]) {
      vbt_recv_t recv = vbt_recv_init();
      int err = vbt_parse(red_input[i], strlen(red_input[i]), &recv);
      ASSERT_RECV_U8(err, recv, 255, 0, 0, 255);
    }
  }

  const char* green_input[] = {
      "oklch(0.866440 0.2948 142.5)",
      "oklch(86.6440% 0.2948 142.5 / 1)",
      "oklcha(0.866440, 0.2948, 142.5, 1)",
  };

  for (size_t i = 0; i < vu_arr_len(green_input); i++) {
    CASE(green_input[i]) {
      vbt_recv_t recv = vbt_recv_init();
      int err = vbt_parse(green_input[i], strlen(green_input[i]), &recv);
      ASSERT_RECV_U8(err, recv, 0, 255, 0, 255);
    }
  }

  const char* blue_input[] = {
      "oklch(0.452014 0.3132 264.05)",
      "oklch(45.2014% 0.3132 264.05 / 1)",
      "oklcha(0.452014, 0.3132, 264.05, 1)",
  };

  for (size_t i = 0; i < vu_arr_len(blue_input); i++) {
    CASE(blue_input[i]) {
      vbt_recv_t recv = vbt_recv_init();
      int err = vbt_parse(blue_input[i], strlen(blue_input[i]), &recv);
      ASSERT_RECV_U8(err, recv, 0, 0, 255, 255);
    }
  }

  const char* gray_input[] = {
      "oklch(0.5978 0 0)",
      "oklch(59.78% 0 0 / 1)",
      "oklcha(0.5978, 0, 0, 1)",
  };

  for (size_t i = 0; i < vu_arr_len(gray_input); i++) {
    CASE(gray_input[i]) {
      vbt_recv_t recv = vbt_recv_init();
      int err = vbt_parse(gray_input[i], strlen(gray_input[i]), &recv);
      ASSERT_RECV_U8(err, recv, 127, 127, 127, 255);
    }
  }
}

TEST(vbt_parse_oklab) {
  const char* red_input[] = {
      "oklab(0.627955 0.224863 0.125846)",
      "oklab(62.7955% 0.224863 0.125846)",
      "oklab(0.627955 0.224863 0.125846 / 1)",
      "oklab(62.7955% 0.224863 0.125846 / 100%)",
      "oklaba(0.627955, 0.224863, 0.125846, 1)",
      "oklaba(62.7955% 0.224863 0.125846 1)",
  };

  for (size_t i = 0; i < vu_arr_len(red_input); i++) {
    CASE(red_input[i]) {
      vbt_recv_t recv = vbt_recv_init();
      int err = vbt_parse(red_input[i], strlen(red_input[i]), &recv);
      ASSERT_RECV_U8(err, recv, 255, 0, 0, 255);
    }
  }

  const char* green_input[] = {
      "oklab(0.866440 -0.233887 0.179498)",
      "oklab(86.6440% -0.233887 0.179498 / 1)",
      "oklaba(0.866440, -0.233887, 0.179498, 1)",
  };

  for (size_t i = 0; i < vu_arr_len(green_input); i++) {
    CASE(green_input[i]) {
      vbt_recv_t recv = vbt_recv_init();
      int err = vbt_parse(green_input[i], strlen(green_input[i]), &recv);
      ASSERT_RECV_U8(err, recv, 0, 255, 0, 255);
    }
  }

  const char* blue_input[] = {
      "oklab(0.452014 -0.032457 -0.311528)",
      "oklab(45.2014% -0.032457 -0.311528 / 1)",
      "oklaba(0.452014, -0.032457, -0.311528, 1)",
  };

  for (size_t i = 0; i < vu_arr_len(blue_input); i++) {
    CASE(blue_input[i]) {
      vbt_recv_t recv = vbt_recv_init();
      int err = vbt_parse(blue_input[i], strlen(blue_input[i]), &recv);
      ASSERT_RECV_U8(err, recv, 0, 0, 255, 255);
    }
  }

  const char* gray_input[] = {
      "oklab(0.5978 0 0)",
      "oklab(59.78% 0 0 / 1)",
      "oklaba(0.5978, 0, 0, 1)",
  };

  for (size_t i = 0; i < vu_arr_len(gray_input); i++) {
    CASE(gray_input[i]) {
      vbt_recv_t recv = vbt_recv_init();
      int err = vbt_parse(gray_input[i], strlen(gray_input[i]), &recv);
      ASSERT_RECV_U8(err, recv, 127, 127, 127, 255);
    }
  }
}

// string that exceeds vibrant's parser string limit of 128. returned value
// is from static memory.
static const char* long_string(void) {
  static char buffer[256];

  snprintf(buffer, vu_arr_len(buffer), "%*s)", -(int)(vu_arr_len(buffer) - 2),
           "rgb(255, 255, 255");

  return buffer;
}

// capitalize a string. returned value is a copy from static memory that can
// be overwritten by the next call to capitalize.
static const char* capitalize(const char* str) {
  static char buffer[256];
  size_t len = strlen(str);

  if (len >= vu_arr_len(buffer)) {
    // cannot assert because rk assert return void.
    abort();
  }

  // try uppercase
  for (size_t j = 0; j < len; j++) {
    char c = str[j];
    buffer[j] = (c >= 'a' && c <= 'z') ? (char)(c & ~0x20) : c;
  }

  buffer[len] = '\0';

  return buffer;
}

#endif  // VIBRANT_NO_PARSE
