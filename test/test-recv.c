#include "test-common.h"

TEST(vbt_recv_receive_values) {
  vbt_recv_t recv;
  int err;

  CASE("u8") {
    recv = vbt_recv_init_tag(VBT_RECV_VAL_U8);
    err = vbt_rgb(50, 100, 200, 1, &recv);

    ASSERT_EQ(err, VBT_SUCCESS);
    ASSERT_EQ(recv.tag, VBT_RECV_VAL_U8);
    ASSERT_EQ(recv.u.val.u8.r, 50);
    ASSERT_EQ(recv.u.val.u8.g, 100);
    ASSERT_EQ(recv.u.val.u8.b, 200);
    ASSERT_EQ(recv.u.val.u8.a, 255);
  }

  CASE("f32") {
    recv = vbt_recv_init_tag(VBT_RECV_VAL_F32);
    err = vbt_rgb(50, 100, 200, 1, &recv);

    ASSERT_EQ(err, VBT_SUCCESS);
    ASSERT_EQ(recv.tag, VBT_RECV_VAL_F32);
    ASSERT_FLOAT_EQ(recv.u.val.f32.r, 50.0f / 255.0f);
    ASSERT_FLOAT_EQ(recv.u.val.f32.g, 100.0f / 255.0f);
    ASSERT_FLOAT_EQ(recv.u.val.f32.b, 200.0f / 255.0f);
    ASSERT_FLOAT_EQ(recv.u.val.f32.a, 255.0f / 255.0f);
  }

  CASE("f64") {
    recv = vbt_recv_init_tag(VBT_RECV_VAL_F64);
    err = vbt_rgb(50, 100, 200, 1, &recv);

    ASSERT_EQ(err, VBT_SUCCESS);
    ASSERT_EQ(recv.tag, VBT_RECV_VAL_F64);
    ASSERT_DOUBLE_EQ(recv.u.val.f64.r, 50.0 / 255.0);
    ASSERT_DOUBLE_EQ(recv.u.val.f64.g, 100.0 / 255.0);
    ASSERT_DOUBLE_EQ(recv.u.val.f64.b, 200.0 / 255.0);
    ASSERT_DOUBLE_EQ(recv.u.val.f64.a, 255.0 / 255.0);
  }
}

TEST(vbt_recv_receive_refs) {
  vbt_recv_t recv;
  int err;

  CASE("u8") {
    vbt_u8_t r, g, b, a;
    recv = vbt_recv_init_ref_u8(&r, &g, &b, &a);
    err = vbt_rgb(50, 100, 200, 1, &recv);

    ASSERT_EQ(err, VBT_SUCCESS);
    ASSERT_EQ(recv.tag, VBT_RECV_REF_U8);
    ASSERT_EQ(r, 50);
    ASSERT_EQ(g, 100);
    ASSERT_EQ(b, 200);
    ASSERT_EQ(a, 255);
  }

  CASE("u8 - null to exclude component") {
    recv = vbt_recv_init_ref_u8(NULL, NULL, NULL, NULL);
    err = vbt_rgb(50, 100, 200, 1, &recv);
    ASSERT_EQ(err, VBT_SUCCESS);
  }

  CASE("f32") {
    float rf, gf, bf, af;
    recv = vbt_recv_init_ref_f32(&rf, &gf, &bf, &af);
    err = vbt_rgb(50, 100, 200, 1, &recv);

    ASSERT_EQ(err, VBT_SUCCESS);
    ASSERT_EQ(recv.tag, VBT_RECV_REF_F32);
    ASSERT_FLOAT_EQ(rf, 50.0f / 255.0f);
    ASSERT_FLOAT_EQ(gf, 100.0f / 255.0f);
    ASSERT_FLOAT_EQ(bf, 200.0f / 255.0f);
    ASSERT_FLOAT_EQ(af, 255.0f / 255.0f);
  }

  CASE("f32 - null to exclude component") {
    recv = vbt_recv_init_ref_f32(NULL, NULL, NULL, NULL);
    err = vbt_rgb(50, 100, 200, 1, &recv);
    ASSERT_EQ(err, VBT_SUCCESS);
  }

  CASE("f64") {
    double rff, gff, bff, aff;
    recv = vbt_recv_init_ref_f64(&rff, &gff, &bff, &aff);
    err = vbt_rgb(50, 100, 200, 1, &recv);

    ASSERT_EQ(err, VBT_SUCCESS);
    ASSERT_EQ(recv.tag, VBT_RECV_REF_F64);
    ASSERT_DOUBLE_EQ(rff, 50.0 / 255.0);
    ASSERT_DOUBLE_EQ(gff, 100.0 / 255.0);
    ASSERT_DOUBLE_EQ(bff, 200.0 / 255.0);
    ASSERT_DOUBLE_EQ(aff, 255.0 / 255.0);
  }

  CASE("f64 - null to exclude component") {
    // allow nulls (client can select components they want)
    recv = vbt_recv_init_ref_f64(NULL, NULL, NULL, NULL);
    err = vbt_rgb(50, 100, 200, 1, &recv);
    ASSERT_EQ(err, VBT_SUCCESS);
  }
}
