#include "test-common.h"

TEST(vbt_rgb_conversion) {
  vbt_recv_t recv;
  int err;

  CASE("Fully opaque red") {
    recv = vbt_recv_init();
    err = vbt_rgb(255, 0, 0, 1, &recv);
    ASSERT_RECV_U8(err, recv, 255, 0, 0, 255);
  }

  CASE("Fully transparent blue") {
    recv = vbt_recv_init();
    err = vbt_rgb(0, 0, 255, 0, &recv);
    ASSERT_RECV_U8(err, recv, 0, 0, 255, 0);
  }

  CASE("Semi-transparent green (alpha = 0.5)") {
    // 0.5 * 255 = 127.5, +.5 and floor = truncated to 128
    recv = vbt_recv_init();
    err = vbt_rgb(0, 255, 0, (vbt_number_t)(0.5), &recv);
    ASSERT_RECV_U8(err, recv, 0, 255, 0, 128);
  }

  CASE("Alpha clamping: alpha > 1.0f") {
    recv = vbt_recv_init();
    err = vbt_rgb(100, 100, 100, 2, &recv);
    ASSERT_RECV_U8(err, recv, 100, 100, 100, 255);
  }

  CASE("Alpha clamping: alpha < 0.0f") {
    recv = vbt_recv_init();
    err = vbt_rgb(50, 50, 50, -1, &recv);
    ASSERT_RECV_U8(err, recv, 50, 50, 50, 0);
  }
}

TEST(vbt_rgb_errors) {
  vbt_recv_t recv = vbt_recv_init();

  CASE("recv = NULL") {
    ASSERT_EQ(vbt_rgb(0, 0, 0, 0, NULL), VBT_ERR);
  }

  CASE("a = NAN") {
    ASSERT_EQ(vbt_rgb(0, 0, 0, NAN, &recv), VBT_ERR);
  }

  CASE("a = INF") {
    ASSERT_EQ(vbt_rgb(0, 0, 0, INF, &recv), VBT_ERR);
  }

  CASE("a = -INF") {
    ASSERT_EQ(vbt_rgb(0, 0, 0, -INF, &recv), VBT_ERR);
  }
}

TEST(vbt_hsl_conversion) {
  vbt_recv_t recv;
  int err;

  CASE("Black") {
    recv = vbt_recv_init();
    err = vbt_hsl(0, 0, 0, 1, &recv);
    ASSERT_RECV_U8(err, recv, 0, 0, 0, 255);
  }

  CASE("White") {
    recv = vbt_recv_init();
    err = vbt_hsl(0, 0, 100, 1, &recv);
    ASSERT_RECV_U8(err, recv, 255, 255, 255, 255);
  }

  CASE("Red") {
    recv = vbt_recv_init();
    err = vbt_hsl(0, 100, 50, 1, &recv);
    ASSERT_RECV_U8(err, recv, 255, 0, 0, 255);
  }

  CASE("Green") {
    recv = vbt_recv_init();
    err = vbt_hsl(120, 100, 50, 1, &recv);
    ASSERT_RECV_U8(err, recv, 0, 255, 0, 255);
  }

  CASE("Blue") {
    recv = vbt_recv_init();
    err = vbt_hsl(240, 100, 50, 1, &recv);
    ASSERT_RECV_U8(err, recv, 0, 0, 255, 255);
  }

  CASE("Gray") {
    recv = vbt_recv_init();
    err = vbt_hsl(0, 0, 50, 1, &recv);
    ASSERT_RECV_U8(err, recv, 128, 128, 128, 255);
  }

  CASE("Orange") {
    recv = vbt_recv_init();
    err = vbt_hsl(30, 100, 50, 1, &recv);
    ASSERT_RECV_U8(err, recv, 255, 128, 0, 255);
  }

  CASE("Semi-transparent Orange") {
    recv = vbt_recv_init();
    err = vbt_hsl(30, 100, 50, (vbt_number_t)(0.5), &recv);
    ASSERT_RECV_U8(err, recv, 255, 128, 0, 128);
  }

  CASE("Red (200% intensity clamped to 100%)") {
    recv = vbt_recv_init();
    err = vbt_hsl(0, 200, 50, 1, &recv);
    ASSERT_RECV_U8(err, recv, 255, 0, 0, 255);
  }

  CASE("Black (-50% intensity clamped to 0%)") {
    recv = vbt_recv_init();
    err = vbt_hsl(0, -50, -50, 1, &recv);
    ASSERT_RECV_U8(err, recv, 0, 0, 0, 255);
  }

  CASE("White (hue = 720deg)") {
    recv = vbt_recv_init();
    err = vbt_hsl(720, 0, 100, 1, &recv);
    ASSERT_RECV_U8(err, recv, 255, 255, 255, 255);
  }
}

TEST(vbt_hsl_errors) {
  vbt_recv_t recv = vbt_recv_init();

  CASE("recv = NULL") {
    ASSERT_EQ(vbt_hsl(0, 0, 0, 0, NULL), VBT_ERR);
  }

  CASE("arg = NAN") {
    ASSERT_EQ(vbt_hsl(NAN, 0, 0, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_hsl(0, NAN, 0, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_hsl(0, 0, NAN, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_hsl(0, 0, 0, NAN, &recv), VBT_ERR);
  }

  CASE("arg = INF") {
    ASSERT_EQ(vbt_hsl(INF, 0, 0, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_hsl(0, INF, 0, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_hsl(0, 0, INF, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_hsl(0, 0, 0, INF, &recv), VBT_ERR);
  }

  CASE("arg = -INF") {
    ASSERT_EQ(vbt_hsl(-INF, 0, 0, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_hsl(0, -INF, 0, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_hsl(0, 0, -INF, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_hsl(0, 0, 0, -INF, &recv), VBT_ERR);
  }
}

TEST(vbt_hwb_conversion) {
  vbt_recv_t recv;
  int err;

  CASE("Black") {
    recv = vbt_recv_init();
    err = vbt_hwb(0, 0, 100, 1, &recv);
    ASSERT_RECV_U8(err, recv, 0, 0, 0, 255);
  }

  CASE("White") {
    recv = vbt_recv_init();
    err = vbt_hwb(0, 100, 0, 1, &recv);
    ASSERT_RECV_U8(err, recv, 255, 255, 255, 255);
  }

  CASE("Red") {
    recv = vbt_recv_init();
    err = vbt_hwb(0, 0, 0, 1, &recv);
    ASSERT_RECV_U8(err, recv, 255, 0, 0, 255);
  }

  CASE("Green") {
    recv = vbt_recv_init();
    err = vbt_hwb(120, 0, 0, 1, &recv);
    ASSERT_RECV_U8(err, recv, 0, 255, 0, 255);
  }

  CASE("Blue") {
    recv = vbt_recv_init();
    err = vbt_hwb(240, 0, 0, 1, &recv);
    ASSERT_RECV_U8(err, recv, 0, 0, 255, 255);
  }

  CASE("Gray (50% - sum whiteness and blackness >= 1.0)") {
    recv = vbt_recv_init();
    err = vbt_hwb(0, 50, 50, 1, &recv);
    ASSERT_RECV_U8(err, recv, 128, 128, 128, 255);
  }

  CASE("Orange") {
    recv = vbt_recv_init();
    err = vbt_hwb(30, 0, 0, 1, &recv);
    ASSERT_RECV_U8(err, recv, 255, 128, 0, 255);
  }

  CASE("Transparent Orange") {
    recv = vbt_recv_init();
    err = vbt_hwb(30, 0, 0, (vbt_number_t)0.5, &recv);
    ASSERT_RECV_U8(err, recv, 255, 128, 0, 128);
  }

  CASE("Desaturated Red (h=0, w=20, b=20)") {
    // r = 100 * (100 - 20 - 20) + 20 = 80 => 204
    // g = 0   * (100 - 20 - 20) + 20 = 20 => 51
    // b = 0   * (100 - 20 - 20) + 20 = 20 => 51
    recv = vbt_recv_init();
    err = vbt_hwb(0, 20, 20, 1, &recv);
    ASSERT_RECV_U8(err, recv, 204, 51, 51, 255);
  }

  CASE("White (200% clamped to 100%, -50% clamped to 0%)") {
    recv = vbt_recv_init();
    err = vbt_hwb(0, 200, -50, 1, &recv);
    ASSERT_RECV_U8(err, recv, 255, 255, 255, 255);
  }

  CASE("White (hue = 720deg)") {
    recv = vbt_recv_init();
    err = vbt_hwb(720, 100, 0, 1, &recv);
    ASSERT_RECV_U8(err, recv, 255, 255, 255, 255);
  }
}

TEST(vbt_hwb_errors) {
  vbt_recv_t recv = vbt_recv_init();

  CASE("recv = NULL") {
    ASSERT_EQ(vbt_hwb(0, 0, 0, 0, NULL), VBT_ERR);
  }

  CASE("arg = NAN") {
    ASSERT_EQ(vbt_hwb(NAN, 0, 0, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_hwb(0, NAN, 0, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_hwb(0, 0, NAN, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_hwb(0, 0, 0, NAN, &recv), VBT_ERR);
  }

  CASE("arg = INF") {
    ASSERT_EQ(vbt_hwb(INF, 0, 0, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_hwb(0, INF, 0, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_hwb(0, 0, INF, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_hwb(0, 0, 0, INF, &recv), VBT_ERR);
  }

  CASE("arg = -INF") {
    ASSERT_EQ(vbt_hwb(-INF, 0, 0, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_hwb(0, -INF, 0, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_hwb(0, 0, -INF, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_hwb(0, 0, 0, -INF, &recv), VBT_ERR);
  }
}

TEST(vbt_lab_conversion) {
  vbt_recv_t recv;
  int err;

  CASE("Black") {
    recv = vbt_recv_init();
    err = vbt_lab(0, 0, 0, 1, &recv);
    ASSERT_RECV_U8(err, recv, 0, 0, 0, 255);
  }

  CASE("White") {
    recv = vbt_recv_init();
    err = vbt_lab(100, 0, 0, 1, &recv);
    ASSERT_RECV_U8(err, recv, 255, 255, 255, 255);
  }

  CASE("Red") {
    recv = vbt_recv_init();
    err = vbt_lab((vbt_number_t)53.23, (vbt_number_t)80.11, (vbt_number_t)67.22,
                  1, &recv);
    ASSERT_RECV_U8(err, recv, 255, 0, 0, 255);
  }

  CASE("Green") {
    recv = vbt_recv_init();
    err = vbt_lab((vbt_number_t)87.73, (vbt_number_t)-86.18,
                  (vbt_number_t)83.18, 1, &recv);
    ASSERT_RECV_U8(err, recv, 0, 255, 0, 255);
  }

  CASE("Blue") {
    recv = vbt_recv_init();
    err = vbt_lab((vbt_number_t)32.3, (vbt_number_t)79.19,
                  (vbt_number_t)-107.86, 1, &recv);
    ASSERT_RECV_U8(err, recv, 0, 0, 255, 255);
  }

  CASE("Gray") {
    recv = vbt_recv_init();
    err = vbt_lab((vbt_number_t)53.59, 0, 0, 1, &recv);
    ASSERT_RECV_U8(err, recv, 128, 128, 128, 255);
  }
}

TEST(vbt_lab_errors) {
  vbt_recv_t recv = vbt_recv_init();

  CASE("recv = NULL") {
    ASSERT_EQ(vbt_lab(0, 0, 0, 0, NULL), VBT_ERR);
  }

  CASE("arg = NAN") {
    ASSERT_EQ(vbt_lab(NAN, 0, 0, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_lab(0, NAN, 0, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_lab(0, 0, NAN, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_lab(0, 0, 0, NAN, &recv), VBT_ERR);
  }

  CASE("arg = INF") {
    ASSERT_EQ(vbt_lab(INF, 0, 0, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_lab(0, INF, 0, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_lab(0, 0, INF, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_lab(0, 0, 0, INF, &recv), VBT_ERR);
  }

  CASE("arg = -INF") {
    ASSERT_EQ(vbt_lab(-INF, 0, 0, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_lab(0, -INF, 0, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_lab(0, 0, -INF, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_lab(0, 0, 0, -INF, &recv), VBT_ERR);
  }
}

TEST(vbt_lch_conversion) {
  vbt_recv_t recv;
  int err;

  CASE("Black") {
    recv = vbt_recv_init();
    err = vbt_lch(0, 0, 0, 1, &recv);
    ASSERT_RECV_U8(err, recv, 0, 0, 0, 255);
  }

  CASE("White") {
    recv = vbt_recv_init();
    err = vbt_lch(100, 0, 0, 1, &recv);
    ASSERT_RECV_U8(err, recv, 255, 255, 255, 255);
  }

  CASE("Red") {
    recv = vbt_recv_init();
    err = vbt_lch((vbt_number_t)53.23, (vbt_number_t)104.55, 40, 1, &recv);
    ASSERT_RECV_U8(err, recv, 255, 0, 0, 255);
  }

  CASE("Green") {
    recv = vbt_recv_init();
    err = vbt_lch((vbt_number_t)87.73, (vbt_number_t)119.78,
                  (vbt_number_t)136.02, 1, &recv);
    ASSERT_RECV_U8(err, recv, 0, 255, 0, 255);
  }

  CASE("Blue") {
    recv = vbt_recv_init();
    err = vbt_lch((vbt_number_t)32.3, (vbt_number_t)133.81,
                  (vbt_number_t)306.28, 1, &recv);
    ASSERT_RECV_U8(err, recv, 0, 0, 255, 255);
  }

  CASE("Gray") {
    recv = vbt_recv_init();
    err = vbt_lch((vbt_number_t)53.59, 0, 0, 1, &recv);
    ASSERT_RECV_U8(err, recv, 128, 128, 128, 255);
  }
}

TEST(vbt_lch_errors) {
  vbt_recv_t recv = vbt_recv_init();

  CASE("recv = NULL") {
    ASSERT_EQ(vbt_lch(0, 0, 0, 0, NULL), VBT_ERR);
  }

  CASE("arg = NAN") {
    ASSERT_EQ(vbt_lch(NAN, 0, 0, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_lch(0, NAN, 0, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_lch(0, 0, NAN, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_lch(0, 0, 0, NAN, &recv), VBT_ERR);
  }

  CASE("arg = INF") {
    ASSERT_EQ(vbt_lch(INF, 0, 0, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_lch(0, INF, 0, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_lch(0, 0, INF, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_lch(0, 0, 0, INF, &recv), VBT_ERR);
  }

  CASE("arg = -INF") {
    ASSERT_EQ(vbt_lch(-INF, 0, 0, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_lch(0, -INF, 0, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_lch(0, 0, -INF, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_lch(0, 0, 0, -INF, &recv), VBT_ERR);
  }
}

TEST(vbt_oklab_conversion) {
  vbt_recv_t recv;
  int err;

  CASE("Black") {
    recv = vbt_recv_init();
    err = vbt_oklab(0, 0, 0, 1, &recv);
    ASSERT_RECV_U8(err, recv, 0, 0, 0, 255);
  }

  CASE("White") {
    recv = vbt_recv_init();
    err = vbt_oklab(1, 0, 0, 1, &recv);
    ASSERT_RECV_U8(err, recv, 255, 255, 255, 255);
  }

  CASE("Red") {
    recv = vbt_recv_init();
    err = vbt_oklab((vbt_number_t)0.627955, (vbt_number_t)0.224863,
                    (vbt_number_t)0.125846, 1, &recv);
    ASSERT_RECV_U8(err, recv, 255, 0, 0, 255);
  }

  CASE("Green") {
    recv = vbt_recv_init();
    err = vbt_oklab((vbt_number_t)0.866440, (vbt_number_t)-0.233887,
                    (vbt_number_t)0.179498, 1, &recv);
    ASSERT_RECV_U8(err, recv, 0, 255, 0, 255);
  }

  CASE("Blue") {
    recv = vbt_recv_init();
    err = vbt_oklab((vbt_number_t)0.452014, (vbt_number_t)-0.032457,
                    (vbt_number_t)-0.311528, 1, &recv);
    ASSERT_RECV_U8(err, recv, 0, 0, 255, 255);
  }

  CASE("Gray") {
    recv = vbt_recv_init();
    err = vbt_oklab((vbt_number_t)0.5978, 0, 0, 1, &recv);
    ASSERT_RECV_U8(err, recv, 127, 127, 127, 255);
  }
}

TEST(vbt_oklab_errors) {
  vbt_recv_t recv = vbt_recv_init();

  CASE("recv = NULL") {
    ASSERT_EQ(vbt_oklab(0, 0, 0, 0, NULL), VBT_ERR);
  }

  CASE("arg = NAN") {
    ASSERT_EQ(vbt_oklab(NAN, 0, 0, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_oklab(0, NAN, 0, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_oklab(0, 0, NAN, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_oklab(0, 0, 0, NAN, &recv), VBT_ERR);
  }

  CASE("arg = INF") {
    ASSERT_EQ(vbt_oklab(INF, 0, 0, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_oklab(0, INF, 0, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_oklab(0, 0, INF, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_oklab(0, 0, 0, INF, &recv), VBT_ERR);
  }

  CASE("arg = -INF") {
    ASSERT_EQ(vbt_oklab(-INF, 0, 0, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_oklab(0, -INF, 0, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_oklab(0, 0, -INF, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_oklab(0, 0, 0, -INF, &recv), VBT_ERR);
  }
}

TEST(vbt_oklch_conversion) {
  vbt_recv_t recv;
  int err;

  CASE("Black") {
    recv = vbt_recv_init();
    err = vbt_oklch(0, 0, 0, 1, &recv);
    ASSERT_RECV_U8(err, recv, 0, 0, 0, 255);
  }

  CASE("White") {
    recv = vbt_recv_init();
    err = vbt_oklch(1, 0, 0, 1, &recv);
    ASSERT_RECV_U8(err, recv, 255, 255, 255, 255);
  }

  CASE("Red") {
    recv = vbt_recv_init();
    err = vbt_oklch((vbt_number_t)0.627955, (vbt_number_t)0.25766,
                    (vbt_number_t)29.233, 1, &recv);
    ASSERT_RECV_U8(err, recv, 255, 0, 0, 255);
  }

  CASE("Green") {
    recv = vbt_recv_init();
    err = vbt_oklch((vbt_number_t)0.866440, (vbt_number_t)0.2948, 142.5, 1,
                    &recv);
    ASSERT_RECV_U8(err, recv, 0, 255, 0, 255);
  }

  CASE("Blue") {
    recv = vbt_recv_init();
    err = vbt_oklch((vbt_number_t)0.452014, (vbt_number_t)0.3132,
                    (vbt_number_t)264.05, 1, &recv);
    ASSERT_RECV_U8(err, recv, 0, 0, 255, 255);
  }

  CASE("Gray") {
    recv = vbt_recv_init();
    err = vbt_oklch((vbt_number_t)0.5978, 0, 0, 1, &recv);
    ASSERT_RECV_U8(err, recv, 127, 127, 127, 255);
  }
}

TEST(vbt_oklch_errors) {
  vbt_recv_t recv = vbt_recv_init();

  CASE("recv = NULL") {
    ASSERT_EQ(vbt_oklch(0, 0, 0, 0, NULL), VBT_ERR);
  }

  CASE("arg = NAN") {
    ASSERT_EQ(vbt_oklch(NAN, 0, 0, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_oklch(0, NAN, 0, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_oklch(0, 0, NAN, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_oklch(0, 0, 0, NAN, &recv), VBT_ERR);
  }

  CASE("arg = INF") {
    ASSERT_EQ(vbt_oklch(INF, 0, 0, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_oklch(0, INF, 0, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_oklch(0, 0, INF, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_oklch(0, 0, 0, INF, &recv), VBT_ERR);
  }

  CASE("arg = -INF") {
    ASSERT_EQ(vbt_oklch(-INF, 0, 0, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_oklch(0, -INF, 0, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_oklch(0, 0, -INF, 0, &recv), VBT_ERR);
    ASSERT_EQ(vbt_oklch(0, 0, 0, -INF, &recv), VBT_ERR);
  }
}
