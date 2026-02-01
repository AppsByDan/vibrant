// v1.0.0 - vibrant
//
// Color conversion API with a parser for CSS-like color strings.
//
// vibrant is implemented as single-header library.
//
// LICENSE
//
// This project is dual-licensed under the MIT license and the
// Apache License (Version 2.0). You may choose either license at
// your option.
//
// License text at the end of this file.
//
// USAGE
//
// #include <stdio.h>
//
// #define VIBRANT_IMPLEMENTATION
// #include <vibrant.h>
//
// int main() {
//   vbt_recv_t recv = vbt_recv_init();
//   int res = vbt_parse_z("hsl(180.0, 50%, 50%)", &recv);
//
//   if (res == VBT_SUCCESS) {
//     printf("%u %u %u %u",
//       recv.u.val.u8.r, recv.u.val.u8.g, recv.u.val.u8.b, recv.u.val.u8.a);
//   }
//
//   return 0;
// }
//
// DEPENDENCIES
//
// vibrant depends on math.h and therefore requires linking to the math
// library (-lm) on Linux platforms.
//
// COMPILER FLAGS
//
// * VIBRANT_NO_PARSE
//   If defined, remove vbt_parse* functions, and therefore parse
//   functionality, from vibrant. Otherwise (default), parse functionality
//   is available.
//
// * VIBRANT_STATIC
//   If defined, vibrant api using static linkage. Otherwise (default),
//   extern linkage is used.
//
// * VIBRANT_DOUBLE_PRECISION
//   If defined, use double precision floating point (double) for color
//   conversion operations. Otherwise (default), single precision floating
//   point (float) is used. For most use cases, the default is preferred.
//

#ifndef VIBRANT_H
#define VIBRANT_H

#ifdef __cplusplus
#include <cinttypes>  // uint8_t
#include <cstddef>    // size_t
#else
#include <inttypes.h>  // uint8_t
#include <stddef.h>    // size_t
#endif

#define VBT_ERR (-1)
#define VBT_SUCCESS (0)

#ifndef VBTDEF
#ifdef VIBRANT_STATIC
#define VBTDEF static
#else
#define VBTDEF extern
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t vbt_u8_t;
typedef size_t vbt_size_t;

#if defined(VIBRANT_DOUBLE_PRECISION)
typedef double vbt_number_t;
#else
typedef float vbt_number_t;
#endif

// Indicates how a vbt_recv_t object will receive color values.
typedef enum vbt_recv_tag_t {
  // get rgb components [0-255] by value
  VBT_RECV_VAL_U8,
  // get rgb components floats, in [0-1] range, by value
  VBT_RECV_VAL_F32,
  // get rgb components doubles, in [0-1] range, by value
  VBT_RECV_VAL_F64,
  // user provided refs (pointers) to u8 values that will receive
  // rgb [0-255] components
  VBT_RECV_REF_U8,
  // user provided refs (pointers) to float values that will receive
  // rgb [0-1] components
  VBT_RECV_REF_F32,
  // user provided refs (pointers) to double values that will receive
  // rgb [0-1] components
  VBT_RECV_REF_F64,
} vbt_recv_tag_t;

// Receiver object used by the vibrant color API.
//
// The object is an in/out parameter. The user specifies how the receiver
// will receive colors using the tag. The tags have two categories "by value"
// and "by reference". When a by value tag is specified, the receiver object
// is populated with the values in the requested format. When a by reference
// tag is specified, the color will be set via the provided pointers in the
// receiver object.
//
// Many graphics apis represent colors in different structs and formats. The
// goal of the receiver object is to make vibrant conversion as easy as
// possible.
typedef struct vbt_recv_t {
  vbt_recv_tag_t tag;
  union {
    union {
      struct {
        vbt_u8_t r, g, b, a;
      } u8;
      struct {
        float r, g, b, a;
      } f32;
      struct {
        double r, g, b, a;
      } f64;
    } val;
    union {
      struct {
        vbt_u8_t *r, *g, *b, *a;
      } u8;
      struct {
        float *r, *g, *b, *a;
      } f32;
      struct {
        double *r, *g, *b, *a;
      } f64;
    } ref;
  } u;

#ifdef __cplusplus
  // C++ glue to make recv initializer function work across C and C++ compiles.
  vbt_recv_t() noexcept : tag(VBT_RECV_VAL_U8) {
    this->u.ref.f64 = {0, 0, 0, 0};
  }
  explicit vbt_recv_t(vbt_recv_tag_t t) noexcept : tag(t) {
    this->u.ref.f64 = {0, 0, 0, 0};
  }
  vbt_recv_t(vbt_u8_t* r, vbt_u8_t* g, vbt_u8_t* b, vbt_u8_t* a) noexcept
      : tag(VBT_RECV_REF_U8) {
    this->u.ref.u8 = {r, g, b, a};
  }
  vbt_recv_t(float* r, float* g, float* b, float* a) noexcept
      : tag(VBT_RECV_REF_F32) {
    this->u.ref.f32 = {r, g, b, a};
  }
  vbt_recv_t(double* r, double* g, double* b, double* a) noexcept
      : tag(VBT_RECV_REF_F64) {
    this->u.ref.f64 = {r, g, b, a};
  }
#endif
} vbt_recv_t;

#ifndef VIBRANT_NO_PARSE

// Parse a CSS-like color string into the sRGB colorspace.
//
// hex colors - #fff and #ffffff with opaque alpha (255) or #ffff and
//              #ffffffff with specified alpha
//
// css color names - case insensitive
// https://developer.mozilla.org/en-US/docs/Web/CSS/Reference/Values/named-color
//
// functions - example: hwb(180, 50%, 50%) or hwb(180 50% 50% / 0.5)
//   rgb(r g b) - color from rgb color components
//   hsl(h s l) - color from hue, saturation and lightness
//   hwb(h w b) - color from hue, whiteness and blackness
//   lch(l c h) - color from LCH colorspace
//   lab(l a b) - color from LAB colorspace
//   oklch(l c h) - color from Oklch colorspace
//   oklab(l a b) - color from Oklab colorspace
//
//   note: alpha can be specified in these function using a "/",
//         rgb(255 255 255 / 50%), or using the function name suffixed with
//         "a", rgba(255, 255, 255, 50%). alpha can be expressed as 0-1 or
//         0%-100%.
// @param recv
// @returns VBT_SUCCESS: color successfully parsed and set in recv
//          VBT_ERR: error parsing string or invalid arguments
VBTDEF int vbt_parse(const char* value, vbt_size_t len, vbt_recv_t* recv);

VBTDEF int vbt_parse_z(const char* value, vbt_recv_t* recv);

#endif  // VIBRANT_NO_PARSE

// Builds an sRGB color from components.
//
// @param r rgb color between 0-255
// @param g rgb color between 0-255
// @param b rgb color between 0-255
// @param alpha an alpha representing the alpha channel value of the output
//        color, where the number 0 corresponds to 0% (fully transparent) and
//        1 corresponds to 100% (fully opaque).
// @param recv
// @returns VBT_SUCCESS: color successfully parsed and set in recv
//          VBT_ERR: invalid arguments
VBTDEF int vbt_rgb(vbt_u8_t r,
                   vbt_u8_t g,
                   vbt_u8_t b,
                   vbt_number_t a,
                   vbt_recv_t* recv);

// Expresses a color in the sRGB color space according to its hue,
// saturation, and lightness components.
//
// @param hue an angle between 0 and 360 degrees representing the color's
//        hue angle. an angle value outside the [0-360] range will be
//        normalized.
// @param saturation
// @param lightness
// @param alpha an alpha representing the alpha channel value of the output
//        color, where the number 0 corresponds to 0% (fully transparent) and
//        1 corresponds to 100% (fully opaque).
// @param recv
// @returns VBT_SUCCESS: color successfully parsed and set in recv
//          VBT_ERR: invalid arguments
VBTDEF int vbt_hsl(vbt_number_t hue,
                   vbt_number_t saturation,
                   vbt_number_t lightness,
                   vbt_number_t alpha,
                   vbt_recv_t* recv);

// Expresses a color in the sRGB color space according to its hue, whiteness,
// and blackness.
//
// @param hue an angle between 0 and 360 degrees representing the color's
//        hue angle. an angle value outside the [0-360] range will be
//        normalized.
// @param whiteness a number representing the color's whiteness to mix in.
//        0 represents no whiteness. 100 represents full whiteness if
//        blackness is 0, otherwise both the whiteness and blackness
//        values are normalized.
// @param blackness
// @param alpha an alpha representing the alpha channel value of the output
//        color, where the number 0 corresponds to 0% (fully transparent) and
//        1 corresponds to 100% (fully opaque).
// @param recv
// @returns VBT_SUCCESS: color successfully parsed and set in recv
//          VBT_ERR: invalid arguments
VBTDEF int vbt_hwb(vbt_number_t hue,
                   vbt_number_t whiteness,
                   vbt_number_t blackness,
                   vbt_number_t alpha,
                   vbt_recv_t* recv);

// Converts the given color in the LCH colorspace to sRGB
//
// @param lightness a number between 0 and 100. This value specifies the
//        color's lightness. Here the number 0 corresponds to 0% (black)
//        and the number 100 corresponds to 100% (white).
// @param chroma a number. This value is a measure of the color's chroma
//        (roughly representing the "amount of color"). Its minimum useful
//        value is 0, or 0%, while its maximum is theoretically unbounded
//        (but in practice does not exceed 230), with 100% being
//        equivalent to 150.
// @param hue an angle between 0 and 360 degrees representing the color's
//        hue angle. an angle value outside the [0-360] range will be
//        normalized.
// @param alpha an alpha representing the alpha channel value of the output
//        color, where the number 0 corresponds to 0% (fully transparent) and
//        1 corresponds to 100% (fully opaque).
// @param recv
// @returns VBT_SUCCESS: color successfully parsed and set in recv
//          VBT_ERR: invalid arguments
VBTDEF int vbt_lch(vbt_number_t lightness,
                   vbt_number_t chroma,
                   vbt_number_t hue,
                   vbt_number_t alpha,
                   vbt_recv_t* recv);

// Converts the given color in the CIE L*a*b* colorspace to sRGB.
//
// @param lightness a number between 0 and 100. This value specifies the
//        color's lightness. Here the number 0 corresponds to 0% (black)
//        and the number 100 corresponds to 100% (white).
// @param a a number between -125 and 125. This value specifies the color's
//        distance along the a axis, which defines how green (moving
//        towards -125) or red (moving towards +125) the color is. Note that
//        these values are signed (allowing both positive and negative values)
//        and theoretically unbounded, meaning that you can set values
//        outside the +-125 (+-100%) limits. In practice, values cannot
//        exceed +-160.
// @param b a number between -125 and 125. This value specifies the color's
//        distance along the a axis, which defines how green (moving
//        towards -125) or red (moving towards +125) the color is. Note that
//        these values are signed (allowing both positive and negative values)
//        and theoretically unbounded, meaning that you can set values
//        outside the +-125 (+-100%) limits. In practice, values cannot
//        exceed +-160.
// @param alpha an alpha representing the alpha channel value of the output
//        color, where the number 0 corresponds to 0% (fully transparent) and
//        1 corresponds to 100% (fully opaque).
// @param recv
// @returns VBT_SUCCESS: color successfully parsed and set in recv
//          VBT_ERR: invalid arguments
VBTDEF int vbt_lab(vbt_number_t lightness,
                   vbt_number_t a,
                   vbt_number_t b,
                   vbt_number_t alpha,
                   vbt_recv_t* recv);

// Expresses the given color in the Oklab color space, using the same L axis
// as vbt_oklab(), but with polar Chroma (chroma) and Hue (hue) coordinates.
//
// @param lightness a number between 0 and 100. This value specifies the
//        color's lightness. Here the number 0 corresponds to 0% (black)
//        and the number 100 corresponds to 100% (white).
// @param chroma a number. This value is a measure of the color's chroma
//        (roughly representing the "amount of color"). Its minimum useful
//        value is 0, while the maximum is theoretically unbounded (but in
//        practice does not exceed 0.5). In this case, 0% is 0 and 100% is
//        the number 0.4.
// @param hue an angle between 0 and 360 degrees representing the color's
//        hue angle. an angle value outside the [0-360] range will be
//        normalized.
// @param alpha an alpha representing the alpha channel value of the output
//        color, where the number 0 corresponds to 0% (fully transparent) and
//        1 corresponds to 100% (fully opaque).
// @param recv
// @returns VBT_SUCCESS: color successfully parsed and set in recv
//          VBT_ERR: invalid arguments
VBTDEF int vbt_oklch(vbt_number_t lightness,
                     vbt_number_t chroma,
                     vbt_number_t hue,
                     vbt_number_t alpha,
                     vbt_recv_t* recv);

// Expresses the given color using the Cartesian coordinate system on the
// Oklab colorspace - a- and b-axes.
//
// @param lightness a number between 0 and 100. This value specifies the
//        color's lightness. Here the number 0 corresponds to 0% (black)
//        and the number 100 corresponds to 100% (white).
// @param a a number between -0.4 and 0.4. This value specifies the color's
//        distance along the a axis in the Oklab color space, which defines
//        how green (moving towards -0.4) or red (moving towards +0.4) the
//        color is. Note that these values are signed (allowing both
//        positive and negative values) and theoretically unbounded, meaning
//        that you can set values outside the +-0.4 (+-100%) limits. In
//        practice, values cannot exceed +-0.5.
// @param b a number between -0.4 and 0.4. This value specifies the color's
//        distance along the a axis in the Oklab color space, which defines
//        how blue (moving towards -0.4) or yellow (moving towards +0.4) the
//        color is. Note that these values are signed (allowing both
//        positive and negative values) and theoretically unbounded, meaning
//        that you can set values outside the +-0.4 (+-100%) limits. In
//        practice, values cannot exceed +-0.5.
// @param alpha an alpha representing the alpha channel value of the output
//        color, where the number 0 corresponds to 0% (fully transparent) and
//        1 corresponds to 100% (fully opaque).
// @param recv
// @returns VBT_SUCCESS: color successfully parsed and set in recv
//          VBT_ERR: invalid arguments
VBTDEF int vbt_oklab(vbt_number_t lightness,
                     vbt_number_t a,
                     vbt_number_t b,
                     vbt_number_t alpha,
                     vbt_recv_t* recv);

#ifdef __cplusplus
}
#endif

// Utility functions/macros for initing vbt_recv_t objects across
// C and C++ builds.
#ifdef __cplusplus
inline vbt_recv_t vbt_recv_init() noexcept {
  return {};
}
inline vbt_recv_t vbt_recv_init_tag(vbt_recv_tag_t tag) noexcept {
  return vbt_recv_t{tag};
}
inline vbt_recv_t vbt_recv_init_ref_u8(vbt_u8_t* r,
                                       vbt_u8_t* g,
                                       vbt_u8_t* b,
                                       vbt_u8_t* a) noexcept {
  return {r, g, b, a};
}
inline vbt_recv_t vbt_recv_init_ref_f32(float* r,
                                        float* g,
                                        float* b,
                                        float* a) noexcept {
  return {r, g, b, a};
}
inline vbt_recv_t vbt_recv_init_ref_f64(double* r,
                                        double* g,
                                        double* b,
                                        double* a) noexcept {
  return {r, g, b, a};
}
#else
#define vbt_recv_init() vbt_recv_init_tag(VBT_RECV_VAL_U8)
#define vbt_recv_init_tag(TAG) \
  ((vbt_recv_t){.tag = TAG, .u.ref.f64 = {0, 0, 0, 0}})
#define vbt_recv_init_ref_u8(R, G, B, A) \
  ((vbt_recv_t){.tag = VBT_RECV_REF_U8, .u.ref.u8 = {R, G, B, A}})
#define vbt_recv_init_ref_f32(R, G, B, A) \
  ((vbt_recv_t){.tag = VBT_RECV_REF_F32, .u.ref.f32 = {R, G, B, A}})
#define vbt_recv_init_ref_f64(R, G, B, A) \
  ((vbt_recv_t){.tag = VBT_RECV_REF_F64, .u.ref.f64 = {R, G, B, A}})
#endif

#endif  // VIBRANT_H

#ifdef VIBRANT_IMPLEMENTATION

#ifdef __cplusplus
#include <cmath>  // fmod, isfinite, pow, cos, sin
#else
#include <math.h>  // fmod, isfinite, pow, cos, sin
#endif

#if defined(VIBRANT_DOUBLE_PRECISION)
#define vbt__fmod fmod
#define vbt__pow pow
#define vbt__cos cos
#define vbt__sin sin
#define vbt__cbrt cbrt
#else
#define vbt__fmod fmodf
#define vbt__pow powf
#define vbt__cos cosf
#define vbt__sin sinf
#define vbt__cbrt cbrtf
#endif
#ifdef __cplusplus
#define vbt__isfinite std::isfinite
#else
#define vbt__isfinite isfinite
#endif

#define VBT__PI ((vbt_number_t)3.14159265358979323846)

// sRGB D65 reference white
#define VBT__D65_X ((vbt_number_t)0.95047)
#define VBT__D65_Y ((vbt_number_t)1.0)
#define VBT__D65_Z ((vbt_number_t)1.08883)

// CIE Standard Illuminant D50
#define VBT__CIE_E ((vbt_number_t)(216.0 / 24389.0))
#define VBT__CIE_K ((vbt_number_t)(24389.0 / 27.0))

#define VBT__MAX_STR_LEN (128)
#define VBT__NUMBER_MAX ((vbt_number_t)(16777216))
#define VBT__NUMBER_DECIMAL_LIMIT (9)
#define VBT__DEG_MIN ((vbt_number_t)(0))
#define VBT__DEG_MAX ((vbt_number_t)(360))
#define VBT__PERCENT_MIN ((vbt_number_t)(0))
#define VBT__PERCENT_MAX ((vbt_number_t)(100))
#define VBT__NOT_A_FUNCTION (-1000)

#define VBT__ARR_LEN(a) (sizeof(a) / sizeof(a[0]))
#define VBT__MIN(a, b) ((a) < (b) ? (a) : (b))
#define VBT__MAX(a, b) ((a) > (b) ? (a) : (b))
#define VBT__CLAMP(val, min, max) \
  ((val) < (min) ? (min) : ((val) > (max) ? (max) : (val)))
#define VBT__CLAMP_01(val) VBT__CLAMP(val, (vbt_number_t)(0), (vbt_number_t)(1))
#define VBT__CLAMP_0100(val) VBT__CLAMP(val, VBT__PERCENT_MIN, VBT__PERCENT_MAX)
#define VBT__CLAMP_0360(val) VBT__CLAMP(val, VBT__DEG_MIN, VBT__DEG_MAX)

// [0-1] to [0-255]
// assume clamped, round might be more accurate?
#define VBT__01_TO_255(comp) \
  (vbt_u8_t)(comp * (vbt_number_t)(255) + (vbt_number_t)(0.5))

typedef int vbt_bool_t;
#define VBT__TRUE (1)
#define VBT__FALSE (0)

// clang-format off
static int vbt__write_u8(vbt_recv_t* recv, vbt_u8_t r, vbt_u8_t g, vbt_u8_t b, vbt_u8_t a);
static int vbt__write_01(vbt_recv_t* recv, vbt_number_t r, vbt_number_t g, vbt_number_t b, vbt_number_t a);
static vbt_number_t vbt__normalize_angle(vbt_number_t hue);
static void vbt__hsl_to_rgb(vbt_number_t hue, vbt_number_t saturation, vbt_number_t lightness, vbt_number_t* r, vbt_number_t* g, vbt_number_t* b);
static vbt_number_t vbt__hsl_to_rgb_fn(vbt_number_t h, vbt_number_t s, vbt_number_t l, vbt_number_t n);
// clang-format on

VBTDEF int vbt_rgb(vbt_u8_t red,
                   vbt_u8_t green,
                   vbt_u8_t blue,
                   vbt_number_t alpha,
                   vbt_recv_t* recv) {
  if (!recv || !vbt__isfinite(alpha)) {
    return VBT_ERR;
  }

  return vbt__write_u8(recv, red, green, blue,
                       VBT__01_TO_255(VBT__CLAMP_01(alpha)));
}

VBTDEF int vbt_hsl(vbt_number_t hue,
                   vbt_number_t saturation,
                   vbt_number_t lightness,
                   vbt_number_t alpha,
                   vbt_recv_t* recv) {
  if (!recv || !vbt__isfinite(hue) || !vbt__isfinite(saturation) ||
      !vbt__isfinite(lightness) || !vbt__isfinite(alpha)) {
    return VBT_ERR;
  }

  vbt_number_t r;
  vbt_number_t g;
  vbt_number_t b;

  vbt__hsl_to_rgb(vbt__normalize_angle(hue), VBT__CLAMP_0100(saturation),
                  VBT__CLAMP_0100(lightness), &r, &g, &b);

  return vbt__write_01(recv, r, g, b, VBT__CLAMP_01(alpha));
}

// https://www.w3.org/TR/css-color-4/#hwb-to-rgb
/*
function hwbToRgb(hue, white, black) {
    white /= 100;
    black /= 100;
    if (white + black >= 1) {
        let gray = white / (white + black);
        return [gray, gray, gray];
    }
    let rgb = hslToRgb(hue, 100, 50);
    for (let i = 0; i < 3; i++) {
        rgb[i] *= (1 - white - black);
        rgb[i] += white;
    }
    return rgb;
}
*/
VBTDEF int vbt_hwb(vbt_number_t hue,
                   vbt_number_t whiteness,
                   vbt_number_t blackness,
                   vbt_number_t alpha,
                   vbt_recv_t* recv) {
  if (!recv || !vbt__isfinite(hue) || !vbt__isfinite(whiteness) ||
      !vbt__isfinite(blackness) || !vbt__isfinite(alpha)) {
    return VBT_ERR;
  }

  const vbt_number_t h = vbt__normalize_angle(hue);
  const vbt_number_t w = VBT__CLAMP_0100(whiteness) / VBT__PERCENT_MAX;
  const vbt_number_t b = VBT__CLAMP_0100(blackness) / VBT__PERCENT_MAX;
  const vbt_number_t wb = w + b;
  const vbt_number_t clamped_alpha = VBT__CLAMP_01(alpha);

  if (wb >= (vbt_number_t)1) {
    vbt_number_t gray = w / wb;
    return vbt__write_01(recv, gray, gray, gray, clamped_alpha);
  }

  vbt_number_t rgb[3];

  vbt__hsl_to_rgb(h, 100, 50, &rgb[0], &rgb[1], &rgb[2]);

  for (size_t i = 0; i < VBT__ARR_LEN(rgb); i++) {
    rgb[i] *= ((vbt_number_t)1.0 - w - b);
    rgb[i] += w;
  }

  return vbt__write_01(recv, rgb[0], rgb[1], rgb[2], clamped_alpha);
}

VBTDEF int vbt_lch(vbt_number_t lightness,
                   vbt_number_t chroma,
                   vbt_number_t hue,
                   vbt_number_t alpha,
                   vbt_recv_t* recv) {
  if (!recv || !vbt__isfinite(lightness) || !vbt__isfinite(chroma) ||
      !vbt__isfinite(hue) || !vbt__isfinite(alpha)) {
    return VBT_ERR;
  }

  const vbt_number_t h_rad = hue * VBT__PI / (vbt_number_t)180.0;
  const vbt_number_t a = chroma * vbt__cos(h_rad);
  const vbt_number_t b = chroma * vbt__sin(h_rad);

  return vbt_lab(lightness, a, b, alpha, recv);
}

VBTDEF int vbt_lab(vbt_number_t lightness,
                   vbt_number_t a,
                   vbt_number_t b,
                   vbt_number_t alpha,
                   vbt_recv_t* recv) {
  if (!recv || !vbt__isfinite(lightness) || !vbt__isfinite(a) ||
      !vbt__isfinite(b) || !vbt__isfinite(alpha)) {
    return VBT_ERR;
  }

  const vbt_number_t lightness_clamped = VBT__CLAMP_0100(lightness);
  const vbt_number_t fy =
      (lightness_clamped + (vbt_number_t)16.0) / (vbt_number_t)116.0;
  const vbt_number_t fx = a / (vbt_number_t)500.0 + fy;
  const vbt_number_t fz = fy - b / (vbt_number_t)200.0;

  const vbt_number_t fx3 = fx * fx * fx;
  const vbt_number_t fz3 = fz * fz * fz;

  const vbt_number_t xr =
      (fx3 > VBT__CIE_E)
          ? fx3
          : ((vbt_number_t)116.0 * fx - (vbt_number_t)16.0) / VBT__CIE_K;
  const vbt_number_t yr = (lightness_clamped > VBT__CIE_K * VBT__CIE_E)
                              ? vbt__pow(fy, (vbt_number_t)3.0)
                              : lightness_clamped / VBT__CIE_K;
  const vbt_number_t zr =
      (fz3 > VBT__CIE_E)
          ? fz3
          : ((vbt_number_t)116.0 * fz - (vbt_number_t)16.0) / VBT__CIE_K;

  const vbt_number_t x = xr * VBT__D65_X;
  const vbt_number_t y = yr * VBT__D65_Y;
  const vbt_number_t z = zr * VBT__D65_Z;

  const vbt_number_t r_lin = (vbt_number_t)3.2404542 * x -
                             (vbt_number_t)1.5371385 * y -
                             (vbt_number_t)0.4985314 * z;
  const vbt_number_t g_lin = (vbt_number_t)-0.9692660 * x +
                             (vbt_number_t)1.8760108 * y +
                             (vbt_number_t)0.0415560 * z;
  const vbt_number_t b_lin = (vbt_number_t)0.0556434 * x -
                             (vbt_number_t)0.2040259 * y +
                             (vbt_number_t)1.0572252 * z;

  vbt_number_t r =
      (r_lin > (vbt_number_t)0.0031308)
          ? (vbt_number_t)1.055 *
                    vbt__pow(r_lin, (vbt_number_t)1.0 / (vbt_number_t)2.4) -
                (vbt_number_t)0.055
          : (vbt_number_t)12.92 * r_lin;
  vbt_number_t g =
      (g_lin > (vbt_number_t)0.0031308)
          ? (vbt_number_t)1.055 *
                    vbt__pow(g_lin, (vbt_number_t)1.0 / (vbt_number_t)2.4) -
                (vbt_number_t)0.055
          : (vbt_number_t)12.92 * g_lin;
  vbt_number_t b_val =
      (b_lin > (vbt_number_t)0.0031308)
          ? (vbt_number_t)1.055 *
                    vbt__pow(b_lin, (vbt_number_t)1.0 / (vbt_number_t)2.4) -
                (vbt_number_t)0.055
          : (vbt_number_t)12.92 * b_lin;

  r = VBT__CLAMP_01(r);
  g = VBT__CLAMP_01(g);
  b_val = VBT__CLAMP_01(b_val);

  return vbt__write_01(recv, r, g, b_val, VBT__CLAMP_01(alpha));
}

VBTDEF int vbt_oklch(vbt_number_t lightness,
                     vbt_number_t chroma,
                     vbt_number_t hue,
                     vbt_number_t alpha,
                     vbt_recv_t* recv) {
  if (!recv || !vbt__isfinite(lightness) || !vbt__isfinite(chroma) ||
      !vbt__isfinite(hue) || !vbt__isfinite(alpha)) {
    return VBT_ERR;
  }

  const vbt_number_t h_rad = hue * VBT__PI / (vbt_number_t)180.0;
  const vbt_number_t a = chroma * vbt__cos(h_rad);
  const vbt_number_t b = chroma * vbt__sin(h_rad);

  return vbt_oklab(lightness, a, b, alpha, recv);
}

VBTDEF int vbt_oklab(vbt_number_t lightness,
                     vbt_number_t a,
                     vbt_number_t b,
                     vbt_number_t alpha,
                     vbt_recv_t* recv) {
  if (!recv || !vbt__isfinite(lightness) || !vbt__isfinite(a) ||
      !vbt__isfinite(b) || !vbt__isfinite(alpha)) {
    return VBT_ERR;
  }

  const vbt_number_t lightness_clamped = VBT__CLAMP_0100(lightness);
  const vbt_number_t l_ = lightness_clamped + (vbt_number_t)0.3963377774 * a +
                          (vbt_number_t)0.2158037573 * b;
  const vbt_number_t m_ = lightness_clamped - (vbt_number_t)0.1055613423 * a -
                          (vbt_number_t)0.0638541728 * b;
  const vbt_number_t s_ = lightness_clamped - (vbt_number_t)0.0894841775 * a -
                          (vbt_number_t)1.2914855480 * b;

  const vbt_number_t l = l_ * l_ * l_;
  const vbt_number_t m = m_ * m_ * m_;
  const vbt_number_t s = s_ * s_ * s_;

  const vbt_number_t r_lin = (vbt_number_t)4.0767416621 * l -
                             (vbt_number_t)3.3077115913 * m +
                             (vbt_number_t)0.2309699292 * s;
  const vbt_number_t g_lin = (vbt_number_t)-1.2684380046 * l +
                             (vbt_number_t)2.6097574011 * m -
                             (vbt_number_t)0.3413193965 * s;
  const vbt_number_t b_lin = (vbt_number_t)-0.0041960863 * l -
                             (vbt_number_t)0.7034186147 * m +
                             (vbt_number_t)1.7076147009 * s;

  vbt_number_t r =
      (r_lin > (vbt_number_t)0.0031308)
          ? (vbt_number_t)1.055 *
                    vbt__pow(r_lin, (vbt_number_t)1.0 / (vbt_number_t)2.4) -
                (vbt_number_t)0.055
          : (vbt_number_t)12.92 * r_lin;
  vbt_number_t g =
      (g_lin > (vbt_number_t)0.0031308)
          ? (vbt_number_t)1.055 *
                    vbt__pow(g_lin, (vbt_number_t)1.0 / (vbt_number_t)2.4) -
                (vbt_number_t)0.055
          : (vbt_number_t)12.92 * g_lin;
  vbt_number_t b_val =
      (b_lin > (vbt_number_t)0.0031308)
          ? (vbt_number_t)1.055 *
                    vbt__pow(b_lin, (vbt_number_t)1.0 / (vbt_number_t)2.4) -
                (vbt_number_t)0.055
          : (vbt_number_t)12.92 * b_lin;

  r = VBT__CLAMP_01(r);
  g = VBT__CLAMP_01(g);
  b_val = VBT__CLAMP_01(b_val);

  return vbt__write_01(recv, r, g, b_val, VBT__CLAMP_01(alpha));
}

static int vbt__write_u8(vbt_recv_t* recv,
                         vbt_u8_t r,
                         vbt_u8_t g,
                         vbt_u8_t b,
                         vbt_u8_t a) {
  switch (recv->tag) {
    case VBT_RECV_VAL_U8:
      recv->u.val.u8.r = r;
      recv->u.val.u8.g = g;
      recv->u.val.u8.b = b;
      recv->u.val.u8.a = a;
      break;
    case VBT_RECV_VAL_F32:
      recv->u.val.f32.r = (float)r / 255.0f;
      recv->u.val.f32.g = (float)g / 255.0f;
      recv->u.val.f32.b = (float)b / 255.0f;
      recv->u.val.f32.a = (float)a / 255.0f;
      break;
    case VBT_RECV_VAL_F64:
      recv->u.val.f64.r = (double)r / 255.0;
      recv->u.val.f64.g = (double)g / 255.0;
      recv->u.val.f64.b = (double)b / 255.0;
      recv->u.val.f64.a = (double)a / 255.0;
      break;
    case VBT_RECV_REF_U8:
      if (recv->u.ref.u8.r)
        *recv->u.ref.u8.r = r;
      if (recv->u.ref.u8.g)
        *recv->u.ref.u8.g = g;
      if (recv->u.ref.u8.b)
        *recv->u.ref.u8.b = b;
      if (recv->u.ref.u8.a)
        *recv->u.ref.u8.a = a;
      break;
    case VBT_RECV_REF_F32:
      if (recv->u.ref.f32.r)
        *recv->u.ref.f32.r = (float)r / 255.0f;
      if (recv->u.ref.f32.g)
        *recv->u.ref.f32.g = (float)g / 255.0f;
      if (recv->u.ref.f32.b)
        *recv->u.ref.f32.b = (float)b / 255.0f;
      if (recv->u.ref.f32.a)
        *recv->u.ref.f32.a = (float)a / 255.0f;
      break;
    case VBT_RECV_REF_F64:
      if (recv->u.ref.f64.r)
        *recv->u.ref.f64.r = (double)r / 255.0;
      if (recv->u.ref.f64.g)
        *recv->u.ref.f64.g = (double)g / 255.0;
      if (recv->u.ref.f64.b)
        *recv->u.ref.f64.b = (double)b / 255.0;
      if (recv->u.ref.f64.a)
        *recv->u.ref.f64.a = (double)a / 255.0;
      break;
    default:
      break;
  }

  return VBT_SUCCESS;
}

static int vbt__write_01(vbt_recv_t* recv,
                         vbt_number_t r,
                         vbt_number_t g,
                         vbt_number_t b,
                         vbt_number_t a) {
  switch (recv->tag) {
    case VBT_RECV_VAL_U8:
      recv->u.val.u8.r = VBT__01_TO_255(r);
      recv->u.val.u8.g = VBT__01_TO_255(g);
      recv->u.val.u8.b = VBT__01_TO_255(b);
      recv->u.val.u8.a = VBT__01_TO_255(a);
      break;
    case VBT_RECV_VAL_F32:
      recv->u.val.f32.r = (float)r;
      recv->u.val.f32.g = (float)g;
      recv->u.val.f32.b = (float)b;
      recv->u.val.f32.a = (float)a;
      break;
    case VBT_RECV_VAL_F64:
      recv->u.val.f64.r = (double)r;
      recv->u.val.f64.g = (double)g;
      recv->u.val.f64.b = (double)b;
      recv->u.val.f64.a = (double)a;
      break;
    case VBT_RECV_REF_U8:
      if (recv->u.ref.u8.r)
        *recv->u.ref.u8.r = VBT__01_TO_255(r);
      if (recv->u.ref.u8.g)
        *recv->u.ref.u8.g = VBT__01_TO_255(g);
      if (recv->u.ref.u8.b)
        *recv->u.ref.u8.b = VBT__01_TO_255(b);
      if (recv->u.ref.u8.a)
        *recv->u.ref.u8.a = VBT__01_TO_255(a);
      break;
    case VBT_RECV_REF_F32:
      if (recv->u.ref.f32.r)
        *recv->u.ref.f32.r = (float)r;
      if (recv->u.ref.f32.g)
        *recv->u.ref.f32.g = (float)g;
      if (recv->u.ref.f32.b)
        *recv->u.ref.f32.b = (float)b;
      if (recv->u.ref.f32.a)
        *recv->u.ref.f32.a = (float)a;
      break;
    case VBT_RECV_REF_F64:
      if (recv->u.ref.f64.r)
        *recv->u.ref.f64.r = (double)r;
      if (recv->u.ref.f64.g)
        *recv->u.ref.f64.g = (double)g;
      if (recv->u.ref.f64.b)
        *recv->u.ref.f64.b = (double)b;
      if (recv->u.ref.f64.a)
        *recv->u.ref.f64.a = (double)a;
      break;
    default:
      break;
  }

  return VBT_SUCCESS;
}

// https://www.w3.org/TR/css-color-4/#hsl-to-rgb
/*
function hslToRgb(hue, sat, light) {

    sat /= 100;
    light /= 100;

    function f(n) {
        let k = (n + hue/30) % 12;
        let a = sat * Math.min(light, 1 - light);
        return light - a * Math.max(-1, Math.min(k - 3, 9 - k, 1));
    }

    return [f(0), f(8), f(4)];
}
*/

static vbt_number_t vbt__hsl_to_rgb_fn(vbt_number_t h,
                                       vbt_number_t s,
                                       vbt_number_t l,
                                       vbt_number_t n) {
  vbt_number_t k = vbt__fmod((n + h / (vbt_number_t)30.0), (vbt_number_t)12.0);
  vbt_number_t a = s * VBT__MIN(l, (vbt_number_t)1.0 - l);
  return l - a * VBT__MAX((vbt_number_t)-1.0,
                          VBT__MIN(k - (vbt_number_t)3.0,
                                   VBT__MIN((vbt_number_t)9.0 - k,
                                            (vbt_number_t)1.0)));
}

static void vbt__hsl_to_rgb(vbt_number_t hue,
                            vbt_number_t saturation,
                            vbt_number_t lightness,
                            vbt_number_t* r,
                            vbt_number_t* g,
                            vbt_number_t* b) {
  const vbt_number_t s = saturation / (vbt_number_t)100.0;
  const vbt_number_t l = lightness / (vbt_number_t)100.0;

  *r = vbt__hsl_to_rgb_fn(hue, s, l, 0);
  *g = vbt__hsl_to_rgb_fn(hue, s, l, 8);
  *b = vbt__hsl_to_rgb_fn(hue, s, l, 4);
}

static vbt_number_t vbt__normalize_angle(vbt_number_t angle) {
  const vbt_number_t a = vbt__fmod(angle, VBT__DEG_MAX);

  return (a < 0 ? a + VBT__DEG_MAX : a);
}

#ifndef VIBRANT_NO_PARSE

typedef enum vbt__function_t {
  VBT__FUNCTION_RGB,
  VBT__FUNCTION_HSL,
  VBT__FUNCTION_HWB,
  VBT__FUNCTION_LCH,
  VBT__FUNCTION_LAB,
  VBT__FUNCTION_OKLCH,
  VBT__FUNCTION_OKLAB
} vbt__function_t;

typedef enum vbt__css_unit_t {
  VBT__CSS_UNIT_UNSET = 0,
  VBT__CSS_UNIT_PERCENT,
  VBT__CSS_UNIT_NUMBER,
} vbt__css_unit_t;

typedef struct vbt__css_value_t {
  vbt_number_t value;
  vbt__css_unit_t unit;
} vbt__css_value_t;

typedef struct vbt__css_color_t {
  const char* name;
  vbt_u8_t color[4];
} vbt__css_color_t;

typedef struct vbt__parser_t {
  const char* sp;
  const char* end;
} vbt__parser_t;

// clang-format off
static vbt_bool_t vbt__hex_char_to_int(int c, int* out);
static int vbt__parse_hex(const char* value, vbt_size_t len, vbt_recv_t* recv);
static int vbt__parse_css_function(const char* value, vbt_size_t len, vbt_recv_t* recv);
static int vbt__parse_css_color_name(const char* value, vbt_size_t len, vbt_recv_t* recv);
static int vbt__consume_css_value(vbt__parser_t* p, vbt__css_value_t* css_value);
static vbt_number_t vbt__css_value_to_01(const vbt__css_value_t* css_value);
static vbt_number_t vbt__css_value_to_lch_chroma(const vbt__css_value_t* css_value);
static vbt_number_t vbt__css_value_to_lab_ab(const vbt__css_value_t* css_value);
static vbt_number_t vbt__css_value_to_ok_lightness(const vbt__css_value_t* css_value);
static vbt_number_t vbt__css_value_to_oklab_ab(const vbt__css_value_t* css_value);
static vbt_number_t vbt__css_value_to_percent(const vbt__css_value_t* css_value);
static vbt_u8_t vbt__css_value_to_u8(const vbt__css_value_t* css_value);
static int vbt__casecmp(const char* s1, const char* s2, vbt_size_t n);
static int vbt__consume_whitespace(vbt__parser_t* p);
static int vbt__consume_if(vbt__parser_t* p, const char* str, size_t len);
static int vbt__consume_delimiter(vbt__parser_t* p, int* is_comma_mode, int is_comma_mode_set);
static int vbt__parse_number(vbt__parser_t* p, vbt_number_t* out);
static int vbt__tolower(int c);
static vbt_size_t vbt__strlen_safe(const char* str, vbt_size_t limit);
static vbt_size_t vbt__hash(const char* str, size_t len);
static const vbt__css_color_t* vbt__find_css_color(const char* value,  vbt_size_t len);
// clang-format on

VBTDEF int vbt_parse(const char* value, vbt_size_t len, vbt_recv_t* recv) {
  if (len == 0 || len > VBT__MAX_STR_LEN || !value || !recv) {
    return VBT_ERR;
  }

  if (*value == '#') {
    return vbt__parse_hex(value, len, recv);
  }

  int result = vbt__parse_css_function(value, len, recv);

  if (result == VBT__NOT_A_FUNCTION) {
    return vbt__parse_css_color_name(value, len, recv);
  }

  return result;
}

VBTDEF int vbt_parse_z(const char* value, vbt_recv_t* recv) {
  vbt_size_t len = vbt__strlen_safe(value, VBT__MAX_STR_LEN);
  return vbt_parse(value, len, recv);
}

static vbt_bool_t vbt__hex_char_to_int(int c, int* out) {
  if (c >= '0' && c <= '9') {
    *out = c - '0';
  } else if (c >= 'a' && c <= 'f') {
    *out = c - 'a' + 10;
  } else if (c >= 'A' && c <= 'F') {
    *out = c - 'A' + 10;
  } else {
    return VBT__FALSE;
  }

  return VBT__TRUE;
}

static int vbt__parse_hex(const char* value, vbt_size_t len, vbt_recv_t* recv) {
  int components[4];
  int component_index = 0;

  if (len == 4 || len == 5) {  // #rgb or #rgba
    for (size_t i = 1; i < len; i++) {
      int c;

      if (!vbt__hex_char_to_int((unsigned char)value[i], &c)) {
        return VBT_ERR;
      }

      components[component_index++] = (c << 4) | c;
    }
  } else if (len == 7 || len == 9) {  // #rrggbb or #rrggbbaa
    for (size_t i = 1; i < len; i += 2) {
      int a;
      int b;

      if (!vbt__hex_char_to_int((unsigned char)value[i], &a)) {
        return VBT_ERR;
      }

      if (!vbt__hex_char_to_int((unsigned char)value[i + 1], &b)) {
        return VBT_ERR;
      }

      components[component_index++] = (a << 4) | b;
    }
  } else {
    return VBT_ERR;
  }

  return vbt__write_u8(recv, (vbt_u8_t)(components[0]),
                       (vbt_u8_t)(components[1]), (vbt_u8_t)(components[2]),
                       component_index == 4 ? (vbt_u8_t)(components[3]) : 255);
}

static vbt_size_t vbt__strlen_safe(const char* str, vbt_size_t limit) {
  vbt_size_t len = 0;

  if (!str) {
    return len;
  }

  while (len < limit && *str) {
    str++;
    len++;
  }

  if (len == limit && *str) {
    return 0;  // Overflow
  }

  return len;
}

static int vbt__tolower(int c) {
  return (c >= 'A' && c <= 'Z') ? (c | 0x20) : c;
}

static int vbt__casecmp(const char* s1, const char* s2, vbt_size_t n) {
  int result = 0;
  const unsigned char* p1 = (const unsigned char*)s1;
  const unsigned char* p2 = (const unsigned char*)s2;

  for (vbt_size_t i = 0; i < n; ++i) {
    result = vbt__tolower(p1[i]) - vbt__tolower(p2[i]);

    if (result != 0) {
      break;
    }
  }

  return result;
}

static int vbt__consume_whitespace(vbt__parser_t* p) {
  int space_count = 0;

  // only skip non-breaking space chars
  while (p->sp < p->end && (*p->sp == ' ' || *p->sp == '\t')) {
    p->sp++;
    space_count++;
  }

  return space_count;
}

// string -> float
// specialized to handle parsing needs
static int vbt__parse_number(vbt__parser_t* p, vbt_number_t* out) {
  const char* sp = p->sp;
  const char* end = p->end;
  vbt_number_t res = 0;
  vbt_number_t sign = 1;

  if (sp < end) {
    if (*sp == '-') {
      sign = -1;
      sp++;
    } else if (*sp == '+') {
      sp++;
    }
  }

  while (sp < end && *sp >= '0' && *sp <= '9') {
    const vbt_number_t d = (vbt_number_t)(*sp - '0');
    if (res > (VBT__NUMBER_MAX - d) / (vbt_number_t)(10.0)) {
      return -1;
    }
    res = res * (vbt_number_t)(10.0) + d;
    sp++;
  }

  if (sp < end && *sp == '.') {
    sp++;
    vbt_number_t f = (vbt_number_t)(0.1);
    vbt_size_t n = 0;

    while (sp < end && *sp >= '0' && *sp <= '9') {
      if (n++ >= VBT__NUMBER_DECIMAL_LIMIT) {
        return -1;
      }

      const vbt_number_t d = (vbt_number_t)(*sp - '0');
      if (res < VBT__NUMBER_MAX && f > ((vbt_number_t)0)) {
        const vbt_number_t val = d * f;
        if (res > VBT__NUMBER_MAX - val) {
          return -1;
        } else {
          res += val;
        }
        f *= (vbt_number_t)(0.1);
      }

      sp++;
    }
  }

  if (sp == p->sp) {
    return -1;
  }

  p->sp = sp;
  *out = res * sign;

  return 0;
}

static int vbt__consume_css_value(vbt__parser_t* p,
                                  vbt__css_value_t* css_value) {
  if (vbt__parse_number(p, &css_value->value) != 0) {
    return 0;
  }

  if (vbt__consume_if(p, "%", 1)) {
    css_value->unit = VBT__CSS_UNIT_PERCENT;
  } else {
    css_value->unit = VBT__CSS_UNIT_NUMBER;
  }

  return 1;
}

static vbt_number_t vbt__css_value_to_01(const vbt__css_value_t* css_value) {
  if (css_value->unit == VBT__CSS_UNIT_PERCENT) {
    return VBT__CLAMP(css_value->value, VBT__PERCENT_MIN, VBT__PERCENT_MAX) /
           VBT__PERCENT_MAX;
  }

  return VBT__CLAMP_01(css_value->value);
}

static vbt_number_t vbt__css_value_to_percent(
    const vbt__css_value_t* css_value) {
  return VBT__CLAMP(css_value->value, VBT__PERCENT_MIN, VBT__PERCENT_MAX);
}

static vbt_u8_t vbt__css_value_to_u8(const vbt__css_value_t* css_value) {
  if (css_value->unit == VBT__CSS_UNIT_PERCENT) {
    vbt_number_t percent =
        VBT__CLAMP(css_value->value, VBT__PERCENT_MIN, VBT__PERCENT_MAX) /
        VBT__PERCENT_MAX;

    return VBT__01_TO_255(percent);
  }

  const vbt_number_t lo = 0;
  const vbt_number_t hi = 255;

  return (vbt_u8_t)VBT__CLAMP(css_value->value + (vbt_number_t)(0.5), lo, hi);
}

static int vbt__consume_if(vbt__parser_t* p, const char* str, size_t len) {
  for (size_t i = 0; i < len; i++) {
    if (p->sp + i >= p->end || p->sp[i] != str[i]) {
      return 0;
    }
  }

  p->sp += len;
  return 1;
}

static int vbt__consume_delimiter(vbt__parser_t* p,
                                  int* is_comma_mode,
                                  int is_comma_mode_set) {
  int space_count = vbt__consume_whitespace(p);

  if (!is_comma_mode_set) {
    if (vbt__consume_if(p, ",", 1)) {
      *is_comma_mode = 1;
      return 1;
    }

    *is_comma_mode = 0;
    return space_count > 0 ? 1 : 0;
  }

  if (*is_comma_mode) {
    return vbt__consume_if(p, ",", 1) ? 1 : 0;
  }

  return space_count > 0 ? 1 : 0;
}

static vbt_number_t vbt__css_value_to_lch_chroma(
    const vbt__css_value_t* css_value) {
  if (css_value->unit == VBT__CSS_UNIT_PERCENT) {
    return VBT__CLAMP(css_value->value, VBT__PERCENT_MIN, VBT__PERCENT_MAX) *
           (vbt_number_t)(1.5);
  }

  return css_value->value;
}

static vbt_number_t vbt__css_value_to_lab_ab(
    const vbt__css_value_t* css_value) {
  if (css_value->unit == VBT__CSS_UNIT_PERCENT) {
    return VBT__CLAMP(css_value->value, -VBT__PERCENT_MAX, VBT__PERCENT_MAX) *
           (vbt_number_t)(1.25);
  }

  return css_value->value;
}

static vbt_number_t vbt__css_value_to_ok_lightness(
    const vbt__css_value_t* css_value) {
  if (css_value->unit == VBT__CSS_UNIT_PERCENT) {
    return VBT__CLAMP(css_value->value, VBT__PERCENT_MIN, VBT__PERCENT_MAX) /
           VBT__PERCENT_MAX;
  }

  return VBT__CLAMP_01(css_value->value);
}

static vbt_number_t vbt__css_value_to_oklab_ab(
    const vbt__css_value_t* css_value) {
  if (css_value->unit == VBT__CSS_UNIT_PERCENT) {
    return VBT__CLAMP(css_value->value, -VBT__PERCENT_MAX, VBT__PERCENT_MAX) *
           (vbt_number_t)(0.004);
  }

  return css_value->value;
}

static int vbt__parse_css_function(const char* value,
                                   vbt_size_t len,
                                   vbt_recv_t* recv) {
  // you must be this tall to enter
  if (len < VBT__ARR_LEN("xxx(0,0,0)") - 1) {
    return VBT__NOT_A_FUNCTION;
  }

  vbt__parser_t parser;

  parser.sp = value;
  parser.end = value + len;

  vbt__function_t fn;

  if (vbt__consume_if(&parser, "rgb", 3)) {
    fn = VBT__FUNCTION_RGB;
  } else if (vbt__consume_if(&parser, "hsl", 3)) {
    fn = VBT__FUNCTION_HSL;
  } else if (vbt__consume_if(&parser, "hwb", 3)) {
    fn = VBT__FUNCTION_HWB;
  } else if (vbt__consume_if(&parser, "lch", 3)) {
    fn = VBT__FUNCTION_LCH;
  } else if (vbt__consume_if(&parser, "lab", 3)) {
    fn = VBT__FUNCTION_LAB;
  } else if (vbt__consume_if(&parser, "oklch", 5)) {
    fn = VBT__FUNCTION_OKLCH;
  } else if (vbt__consume_if(&parser, "oklab", 5)) {
    fn = VBT__FUNCTION_OKLAB;
  } else {
    return VBT__NOT_A_FUNCTION;
  }

  int is_alpha_version = 0;

  if (vbt__consume_if(&parser, "a", 1)) {
    is_alpha_version = 1;
  }

  vbt__css_value_t arg[4];
  int is_comma_mode;
  int is_comma_mode_set = 0;

  vbt__consume_whitespace(&parser);

  if (!vbt__consume_if(&parser, "(", 1)) {
    return VBT_ERR;
  }

  // all function have at least 3 arguments
  for (size_t i = 0; i < VBT__ARR_LEN(arg) - 1; i++) {
    vbt__consume_whitespace(&parser);

    if (!vbt__consume_css_value(&parser, &arg[i])) {
      return VBT_ERR;
    }

    // break after read value #3, next char can be ' ', ',', ')' or '/'
    if (i == VBT__ARR_LEN(arg) - 2) {
      break;
    }

    if (!vbt__consume_delimiter(&parser, &is_comma_mode, is_comma_mode_set)) {
      return VBT_ERR;
    }

    is_comma_mode_set = 1;
  }

  if (is_alpha_version) {
    // 'a' version of functions always have 4 parameters
    if (!vbt__consume_delimiter(&parser, &is_comma_mode, is_comma_mode_set)) {
      return VBT_ERR;
    }

    vbt__consume_whitespace(&parser);

    if (!vbt__consume_css_value(&parser, &arg[3])) {
      return VBT_ERR;
    }
  } else {
    // non 'a' functions can add alpha using '/' instead of ','
    vbt__consume_whitespace(&parser);

    if (vbt__consume_if(&parser, "/", 1)) {
      vbt__consume_whitespace(&parser);

      if (!vbt__consume_css_value(&parser, &arg[3])) {
        return VBT_ERR;
      }
    } else {
      arg[3].value = 1;
      arg[3].unit = VBT__CSS_UNIT_NUMBER;
    }
  }

  vbt__consume_whitespace(&parser);

  if (!vbt__consume_if(&parser, ")", 1)) {
    return VBT_ERR;
  }

  vbt__consume_whitespace(&parser);

  if (parser.sp != parser.end) {
    return VBT_ERR;
  }

  // TODO: this should be an assert.
  for (size_t i = 0; i < VBT__ARR_LEN(arg); i++) {
    if (arg[i].unit == VBT__CSS_UNIT_UNSET) {
      return VBT_ERR;
    }
  }

  // translate parsed args to each css function's requirements
  // https://developer.mozilla.org/en-US/docs/Web/CSS/Reference/Values/color_value
  switch (fn) {
    case VBT__FUNCTION_HSL: {
      const vbt_number_t hue = arg[0].value;
      const vbt_number_t saturation = vbt__css_value_to_percent(&arg[1]);
      const vbt_number_t lightness = vbt__css_value_to_percent(&arg[2]);
      const vbt_number_t alpha = vbt__css_value_to_01(&arg[3]);

      return vbt_hsl(hue, saturation, lightness, alpha, recv);
    }
    case VBT__FUNCTION_HWB: {
      const vbt_number_t hue = arg[0].value;
      const vbt_number_t whiteness = vbt__css_value_to_percent(&arg[1]);
      const vbt_number_t blackness = vbt__css_value_to_percent(&arg[2]);
      const vbt_number_t alpha = vbt__css_value_to_01(&arg[3]);

      return vbt_hwb(hue, whiteness, blackness, alpha, recv);
    }
    case VBT__FUNCTION_RGB: {
      const vbt_u8_t red = vbt__css_value_to_u8(&arg[0]);
      const vbt_u8_t green = vbt__css_value_to_u8(&arg[1]);
      const vbt_u8_t blue = vbt__css_value_to_u8(&arg[2]);
      const vbt_number_t alpha = vbt__css_value_to_01(&arg[3]);

      return vbt__write_u8(recv, red, green, blue, VBT__01_TO_255(alpha));
    }
    case VBT__FUNCTION_LCH: {
      const vbt_number_t lightness = vbt__css_value_to_percent(&arg[0]);
      const vbt_number_t chroma = vbt__css_value_to_lch_chroma(&arg[1]);
      const vbt_number_t hue = arg[2].value;
      const vbt_number_t alpha = vbt__css_value_to_01(&arg[3]);

      return vbt_lch(lightness, chroma, hue, alpha, recv);
    }
    case VBT__FUNCTION_LAB: {
      const vbt_number_t lightness = vbt__css_value_to_percent(&arg[0]);
      const vbt_number_t a = vbt__css_value_to_lab_ab(&arg[1]);
      const vbt_number_t b = vbt__css_value_to_lab_ab(&arg[2]);
      const vbt_number_t alpha = vbt__css_value_to_01(&arg[3]);

      return vbt_lab(lightness, a, b, alpha, recv);
    }
    case VBT__FUNCTION_OKLCH: {
      // chroma has same constraints as oklab ab
      const vbt_number_t lightness = vbt__css_value_to_ok_lightness(&arg[0]);
      const vbt_number_t chroma = vbt__css_value_to_oklab_ab(&arg[1]);
      const vbt_number_t hue = arg[2].value;
      const vbt_number_t alpha = vbt__css_value_to_01(&arg[3]);

      return vbt_oklch(lightness, chroma, hue, alpha, recv);
    }
    case VBT__FUNCTION_OKLAB: {
      const vbt_number_t lightness = vbt__css_value_to_ok_lightness(&arg[0]);
      const vbt_number_t a = vbt__css_value_to_oklab_ab(&arg[1]);
      const vbt_number_t b = vbt__css_value_to_oklab_ab(&arg[2]);
      const vbt_number_t alpha = vbt__css_value_to_01(&arg[3]);

      return vbt_oklab(lightness, a, b, alpha, recv);
    }
    default: {
      // unreachable
      return VBT_ERR;
    }
  }
}

static int vbt__parse_css_color_name(const char* value,
                                     vbt_size_t len,
                                     vbt_recv_t* recv) {
  const vbt__css_color_t* css_color = vbt__find_css_color(value, len);

  if (*css_color->name != '\0') {
    return vbt__write_u8(recv, css_color->color[0], css_color->color[1],
                         css_color->color[2], css_color->color[3]);
  }

  return VBT_ERR;
}

// clang-format off

/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: gperf --constants-prefix=VBT__ --null-strings --hash-function-name=vbt__hash --lookup-function-name=vbt__in_word_set --ignore-case -l -t -T css_color_names.gperf  */
/* Computed positions: -k'1,3,6-8,12-13' */
/* maximum key range = 557, duplicates = 0 */

static vbt_size_t vbt__hash (const char *str, size_t len) {
  static const unsigned short asso_values[] = {
    561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561,
    561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561,
    561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561,
    561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561,
    561, 561, 561, 561, 561, 5,   160, 0,   35,  0,   50,  10,  40,  0,   561,
    170, 10,  95,  85,  60,  80,  135, 0,   20,  45,  10,  50,  205, 35,  95,
    561, 35,  561, 561, 561, 561, 561, 5,   160, 0,   35,  0,   50,  10,  40,
    0,   561, 170, 10,  95,  85,  60,  80,  135, 0,   20,  45,  10,  50,  205,
    35,  95,  561, 35,  561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561,
    561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561,
    561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561,
    561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561,
    561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561,
    561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561,
    561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561,
    561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561,
    561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561, 561,
    561, 561, 561,
  };
  vbt_size_t hval = len;

  switch (hval) {
    default:
      hval += asso_values[(unsigned char)str[12]];
    /*FALLTHROUGH*/
    case 12:
      hval += asso_values[(unsigned char)str[11]];
    /*FALLTHROUGH*/
    case 11:
    case 10:
    case 9:
    case 8:
      hval += asso_values[(unsigned char)str[7]];
    /*FALLTHROUGH*/
    case 7:
      hval += asso_values[(unsigned char)str[6]];
    /*FALLTHROUGH*/
    case 6:
      hval += asso_values[(unsigned char)str[5]];
    /*FALLTHROUGH*/
    case 5:
    case 4:
    case 3:
      hval += asso_values[(unsigned char)str[2]+2];
    /*FALLTHROUGH*/
    case 2:
    case 1:
      hval += asso_values[(unsigned char)str[0]];
      break;
  }

  return hval;
}

static const vbt__css_color_t* vbt__find_css_color(const char* value,
                                                   vbt_size_t len) {
  static const unsigned char lengthtable[] = {
     0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     4,  0,  0,  0,  0,  0, 10,  0,  0,  0,  4,  5,  0,  0,
     0,  9, 10,  0,  0,  0,  9,  0,  0,  0,  0,  9,  0,  6,
     0,  0,  9,  0,  0,  0,  8,  4,  5,  0,  0,  3,  0,  0,
    11,  0,  0, 14,  0,  0,  7,  0, 14,  0,  6,  0,  0,  9,
     0,  0,  7,  0,  4,  0,  0,  0,  0,  9, 10,  0,  0,  0,
     0,  0,  0,  0,  8,  9,  0,  0,  0,  0,  0,  5, 11,  0,
     0,  4,  0,  6,  0,  0,  9,  0, 11,  0,  8, 14,  0,  6,
     0,  8,  9,  0,  6,  7,  0,  9,  0,  0,  7,  0,  4,  0,
    11,  0,  3,  4, 10,  6,  0, 13,  0,  0, 11,  0,  0,  0,
     5, 11,  0, 13,  9,  0,  0,  0,  0,  0,  0,  0,  7, 13,
    14,  0,  0,  0,  0,  4, 10, 11,  0, 13,  4,  5,  0,  0,
     0,  0, 20,  6,  0,  0,  9,  5,  6,  0,  0,  9, 10,  0,
     0,  8,  9,  0,  0,  0,  8,  9,  0,  0, 12,  0,  9,  0,
     6,  7,  0,  9, 10,  0,  7,  0,  9,  0,  0, 12,  8,  9,
     0,  0,  0,  0,  4,  5,  0,  0,  0,  9,  5,  0,  0,  0,
     9, 10,  0,  0,  0,  9,  0,  6,  7,  8,  0,  5,  6,  0,
     0,  9,  0, 11,  0,  0,  0,  0,  0,  0, 13,  9,  0,  0,
    12, 13,  0,  0,  0,  0, 13,  0,  0,  0,  0,  0,  0, 10,
     0,  0,  0,  0,  0,  0,  0,  8, 14,  0,  0,  0,  0,  9,
     0,  0, 12, 13,  9,  0,  0,  0,  0,  4,  0,  0,  0, 13,
     0,  0,  0,  0,  0,  0,  5,  0,  0,  0,  9,  0,  0, 12,
     8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     7,  0,  0,  0,  0,  0,  0,  0, 10,  0,  0,  0,  0,  5,
     0,  0,  0,  0, 10,  0, 12, 13,  0,  0,  0, 12,  0,  0,
    15,  0,  7,  0, 14, 10,  0,  0,  0,  0,  0,  0,  0,  0,
     9,  0,  0,  0,  8,  4, 15,  0,  0,  0,  0,  0,  0, 17,
    13,  0,  5,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  6,
     0,  0, 14,  0,  0,  0,  0,  0, 10, 16,  0,  0,  0,  0,
    11,  0,  0,  9,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    10,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0, 11,  0,  0,  0,  0,  0,  0,  0,  0, 10,  0,  0,
     0,  0,  0, 11,  0,  0,  0, 15,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0, 11,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  9,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    10
  };

  #define VBT__X {NULL, {0, 0, 0, 0}}
  #define VBT__C(C, R, G, B, A) {C, {R, G, B, A}}

  static const vbt__css_color_t wordlist[] = {
    VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__C("cyan", 0, 255, 255, 255),
    VBT__X, VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__C("gray", 128, 128, 128, 255),
    VBT__X, VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__C("chartreuse", 127, 255, 0, 255),
    VBT__X, VBT__X, VBT__X,
    VBT__C("grey", 128, 128, 128, 255),
    VBT__C("green", 0, 128, 0, 255),
    VBT__X, VBT__X, VBT__X,
    VBT__C("lightgrey", 211, 211, 211, 255),
    VBT__C("lightgreen", 144, 238, 144, 255),
    VBT__X, VBT__X, VBT__X,
    VBT__C("lightgray", 211, 211, 211, 255),
    VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__C("slategrey", 112, 128, 144, 255),
    VBT__X,
    VBT__C("sienna", 160, 82, 45, 255),
    VBT__X, VBT__X,
    VBT__C("slategray", 112, 128, 144, 255),
    VBT__X, VBT__X, VBT__X,
    VBT__C("seashell", 255, 245, 238, 255),
    VBT__C("teal", 0, 128, 128, 255),
    VBT__C("coral", 255, 127, 80, 255),
    VBT__X, VBT__X,
    VBT__C("red", 255, 0, 0, 255),
    VBT__X, VBT__X,
    VBT__C("lightsalmon", 255, 160, 122, 255),
    VBT__X, VBT__X,
    VBT__C("lightslategrey", 119, 136, 153, 255),
    VBT__X, VBT__X,
    VBT__C("fuchsia", 255, 0, 255, 255),
    VBT__X,
    VBT__C("lightslategray", 119, 136, 153, 255),
    VBT__X,
    VBT__C("orange", 255, 165, 0, 255),
    VBT__X, VBT__X,
    VBT__C("orangered", 255, 69, 0, 255),
    VBT__X, VBT__X,
    VBT__C("skyblue", 135, 206, 235, 255),
    VBT__X,
    VBT__C("lime", 0, 255, 0, 255),
    VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__C("limegreen", 50, 205, 50, 255),
    VBT__C("lightcoral", 240, 128, 128, 255),
    VBT__X, VBT__X, VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__X,
    VBT__C("lavender", 230, 230, 250, 255),
    VBT__C("darkgreen", 0, 100, 0, 255),
    VBT__X, VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__C("linen", 250, 240, 230, 255),
    VBT__C("springgreen", 0, 255, 127, 255),
    VBT__X, VBT__X,
    VBT__C("gold", 255, 215, 0, 255),
    VBT__X,
    VBT__C("orchid", 218, 112, 214, 255),
    VBT__X, VBT__X,
    VBT__C("firebrick", 178, 34, 34, 255),
    VBT__X,
    VBT__C("darkmagenta", 139, 0, 139, 255),
    VBT__X,
    VBT__C("darkblue", 0, 0, 139, 255),
    VBT__C("lightsteelblue", 176, 196, 222, 255),
    VBT__X,
    VBT__C("silver", 192, 192, 192, 255),
    VBT__X,
    VBT__C("seagreen", 46, 139, 87, 255),
    VBT__C("lawngreen", 124, 252, 0, 255),
    VBT__X,
    VBT__C("indigo", 75, 0, 130, 255),
    VBT__C("oldlace", 253, 245, 230, 255),
    VBT__X,
    VBT__C("lightcyan", 224, 255, 255, 255),
    VBT__X, VBT__X,
    VBT__C("darkred", 139, 0, 0, 255),
    VBT__X,
    VBT__C("navy", 0, 0, 128, 255),
    VBT__X,
    VBT__C("lightyellow", 255, 255, 224, 255),
    VBT__X,
    VBT__C("tan", 210, 180, 140, 255),
    VBT__C("peru", 205, 133, 63, 255),
    VBT__C("darkorchid", 153, 50, 204, 255),
    VBT__C("purple", 128, 0, 128, 255),
    VBT__X,
    VBT__C("lightseagreen", 32, 178, 170, 255),
    VBT__X, VBT__X,
    VBT__C("greenyellow", 173, 255, 47, 255),
    VBT__X, VBT__X, VBT__X,
    VBT__C("ivory", 255, 255, 240, 255),
    VBT__C("transparent", 0, 0, 0, 0),
    VBT__X,
    VBT__C("rebeccapurple", 102, 51, 153, 255),
    VBT__C("indianred", 205, 92, 92, 255),
    VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__X, VBT__X, VBT__X,
    VBT__C("magenta", 255, 0, 255, 255),
    VBT__C("lavenderblush", 255, 240, 245, 255),
    VBT__C("darkolivegreen", 85, 107, 47, 255),
    VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__C("snow", 255, 250, 250, 255),
    VBT__C("darkviolet", 148, 0, 211, 255),
    VBT__C("forestgreen", 34, 139, 34, 255),
    VBT__X,
    VBT__C("darkslateblue", 72, 61, 139, 255),
    VBT__C("pink", 255, 192, 203, 255),
    VBT__C("black", 0, 0, 0, 255),
    VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__C("lightgoldenrodyellow", 250, 250, 210, 255),
    VBT__C("tomato", 255, 99, 71, 255),
    VBT__X, VBT__X,
    VBT__C("palegreen", 152, 251, 152, 255),
    VBT__C("khaki", 240, 230, 140, 255),
    VBT__C("bisque", 255, 228, 196, 255),
    VBT__X, VBT__X,
    VBT__C("turquoise", 64, 224, 208, 255),
    VBT__C("darkorange", 255, 140, 0, 255),
    VBT__X, VBT__X,
    VBT__C("darkgrey", 169, 169, 169, 255),
    VBT__C("lightpink", 255, 182, 193, 255),
    VBT__X, VBT__X, VBT__X,
    VBT__C("darkgray", 169, 169, 169, 255),
    VBT__C("mintcream", 245, 255, 250, 255),
    VBT__X, VBT__X,
    VBT__C("darkseagreen", 143, 188, 143, 255),
    VBT__X,
    VBT__C("mistyrose", 255, 228, 225, 255),
    VBT__X,
    VBT__C("salmon", 250, 128, 114, 255),
    VBT__C("dimgrey", 105, 105, 105, 255),
    VBT__X,
    VBT__C("lightblue", 173, 216, 230, 255),
    VBT__C("darksalmon", 233, 150, 122, 255),
    VBT__X,
    VBT__C("dimgray", 105, 105, 105, 255),
    VBT__X,
    VBT__C("chocolate", 210, 105, 30, 255),
    VBT__X, VBT__X,
    VBT__C("lemonchiffon", 255, 250, 205, 255),
    VBT__C("moccasin", 255, 228, 181, 255),
    VBT__C("slateblue", 106, 90, 205, 255),
    VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__C("aqua", 0, 255, 255, 255),
    VBT__C("azure", 240, 255, 255, 255),
    VBT__X, VBT__X, VBT__X,
    VBT__C("steelblue", 70, 130, 180, 255),
    VBT__C("wheat", 245, 222, 179, 255),
    VBT__X, VBT__X, VBT__X,
    VBT__C("royalblue", 65, 105, 225, 255),
    VBT__C("aquamarine", 127, 255, 212, 255),
    VBT__X, VBT__X, VBT__X,
    VBT__C("peachpuff", 255, 218, 185, 255),
    VBT__X,
    VBT__C("maroon", 128, 0, 0, 255),
    VBT__C("thistle", 216, 191, 216, 255),
    VBT__C("cornsilk", 255, 248, 220, 255),
    VBT__X,
    VBT__C("olive", 128, 128, 0, 255),
    VBT__C("violet", 238, 130, 238, 255),
    VBT__X, VBT__X,
    VBT__C("cadetblue", 95, 158, 160, 255),
    VBT__X,
    VBT__C("saddlebrown", 139, 69, 19, 255),
    VBT__X, VBT__X, VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__C("darkslategrey", 47, 79, 79, 255),
    VBT__C("goldenrod", 218, 165, 32, 255),
    VBT__X, VBT__X,
    VBT__C("midnightblue", 25, 25, 112, 255),
    VBT__C("darkslategray", 47, 79, 79, 255),
    VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__C("darkturquoise", 0, 206, 209, 255),
    VBT__X, VBT__X, VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__C("dodgerblue", 30, 144, 255, 255),
    VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__X, VBT__X, VBT__X,
    VBT__C("darkcyan", 0, 139, 139, 255),
    VBT__C("mediumseagreen", 60, 179, 113, 255),
    VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__C("olivedrab", 107, 142, 35, 255),
    VBT__X, VBT__X,
    VBT__C("antiquewhite", 250, 235, 215, 255),
    VBT__C("palevioletred", 219, 112, 147, 255),
    VBT__C("rosybrown", 188, 143, 143, 255),
    VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__C("plum", 221, 160, 221, 255),
    VBT__X, VBT__X, VBT__X,
    VBT__C("darkgoldenrod", 184, 134, 11, 255),
    VBT__X, VBT__X, VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__C("brown", 165, 42, 42, 255),
    VBT__X, VBT__X, VBT__X,
    VBT__C("darkkhaki", 189, 183, 107, 255),
    VBT__X, VBT__X,
    VBT__C("lightskyblue", 135, 206, 250, 255),
    VBT__C("deeppink", 255, 20, 147, 255),
    VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__X,
    VBT__C("crimson", 220, 20, 60, 255),
    VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__X, VBT__X, VBT__X,
    VBT__C("sandybrown", 244, 164, 96, 255),
    VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__C("beige", 245, 245, 220, 255),
    VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__C("papayawhip", 255, 239, 213, 255),
    VBT__X,
    VBT__C("mediumpurple", 147, 112, 219, 255),
    VBT__C("paleturquoise", 175, 238, 238, 255),
    VBT__X, VBT__X, VBT__X,
    VBT__C("mediumorchid", 186, 85, 211, 255),
    VBT__X, VBT__X,
    VBT__C("mediumvioletred", 199, 21, 133, 255),
    VBT__X,
    VBT__C("hotpink", 255, 105, 180, 255),
    VBT__X,
    VBT__C("cornflowerblue", 100, 149, 237, 255),
    VBT__C("powderblue", 176, 224, 230, 255),
    VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__C("aliceblue", 240, 248, 255, 255),
    VBT__X, VBT__X, VBT__X,
    VBT__C("honeydew", 240, 255, 240, 255),
    VBT__C("blue", 0, 0, 255, 255),
    VBT__C("mediumturquoise", 72, 209, 204, 255),
    VBT__X, VBT__X, VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__C("mediumspringgreen", 0, 250, 154, 255),
    VBT__C("palegoldenrod", 238, 232, 170, 255),
    VBT__X,
    VBT__C("white", 255, 255, 255, 255),
    VBT__X, VBT__X, VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__C("yellow", 255, 255, 0, 255),
    VBT__X, VBT__X,
    VBT__C("blanchedalmond", 255, 235, 205, 255),
    VBT__X, VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__C("ghostwhite", 248, 248, 255, 255),
    VBT__C("mediumaquamarine", 102, 205, 170, 255),
    VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__C("yellowgreen", 154, 205, 50, 255),
    VBT__X, VBT__X,
    VBT__C("gainsboro", 220, 220, 220, 255),
    VBT__X, VBT__X, VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__C("mediumblue", 0, 0, 205, 255),
    VBT__X, VBT__X, VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__X, VBT__X, VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__X, VBT__X, VBT__X,
    VBT__C("navajowhite", 255, 222, 173, 255),
    VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__C("blueviolet", 138, 43, 226, 255),
    VBT__X, VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__C("floralwhite", 255, 250, 240, 255),
    VBT__X, VBT__X, VBT__X,
    VBT__C("mediumslateblue", 123, 104, 238, 255),
    VBT__X, VBT__X, VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__X, VBT__X, VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__X, VBT__X, VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__X, VBT__X, VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__X,
    VBT__C("deepskyblue", 0, 191, 255, 255),
    VBT__X, VBT__X, VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__X, VBT__X, VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__X, VBT__X, VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__X, VBT__X, VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__X, VBT__X, VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__X, VBT__X, VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__X, VBT__X, VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__X, VBT__X, VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__X, VBT__X, VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__X, VBT__X, VBT__X,
    VBT__C("burlywood", 222, 184, 135, 255),
    VBT__X, VBT__X, VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__X, VBT__X, VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__X, VBT__X, VBT__X, VBT__X, VBT__X, VBT__X,
    VBT__X, VBT__X,
    VBT__C("whitesmoke", 245, 245, 245, 255)
  };
  static const vbt__css_color_t EMPTY = VBT__X;

  #undef VBT__X
  #undef VBT__C

  // from gperf generated macros
  const vbt_size_t VBT__MIN_WORD_LENGTH = 3;
  const vbt_size_t VBT__MAX_WORD_LENGTH = 20;
  const vbt_size_t VBT__MAX_HASH_VALUE = 560;

  if (len <= VBT__MAX_WORD_LENGTH && len >= VBT__MIN_WORD_LENGTH) {
    vbt_size_t key = vbt__hash (value, len);

    if (key <= VBT__MAX_HASH_VALUE && len == lengthtable[key] && vbt__casecmp(value, wordlist[key].name, len) == 0) {
      return &wordlist[key];
    }
  }

  return &EMPTY;
}
// clang-format on

#endif  // VIBRANT_NO_PARSE

#undef VIBRANT_IMPLEMENTATION

#endif  // VIBRANT_IMPLEMENTATION

/*
This project is dual-licensed under the MIT license and the
Apache License (Version 2.0). You may choose either license at your option.

MIT License

Copyright (c) 2026 Daniel Anderson

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

... OR ...

                                 Apache License
                           Version 2.0, January 2004
                        http://www.apache.org/licenses/

   TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION

   1. Definitions.

      "License" shall mean the terms and conditions for use, reproduction,
      and distribution as defined by Sections 1 through 9 of this document.

      "Licensor" shall mean the copyright owner or entity authorized by
      the copyright owner that is granting the License.

      "Legal Entity" shall mean the union of the acting entity and all
      other entities that control, are controlled by, or are under common
      control with that entity. For the purposes of this definition,
      "control" means (i) the power, direct or indirect, to cause the
      direction or management of such entity, whether by contract or
      otherwise, or (ii) ownership of fifty percent (50%) or more of the
      outstanding shares, or (iii) beneficial ownership of such entity.

      "You" (or "Your") shall mean an individual or Legal Entity
      exercising permissions granted by this License.

      "Source" form shall mean the preferred form for making modifications,
      including but not limited to software source code, documentation
      source, and configuration files.

      "Object" form shall mean any form resulting from mechanical
      transformation or translation of a Source form, including but
      not limited to compiled object code, generated documentation,
      and conversions to other media types.

      "Work" shall mean the work of authorship, whether in Source or
      Object form, made available under the License, as indicated by a
      copyright notice that is included in or attached to the work
      (an example is provided in the Appendix below).

      "Derivative Works" shall mean any work, whether in Source or Object
      form, that is based on (or derived from) the Work and for which the
      editorial revisions, annotations, elaborations, or other modifications
      represent, as a whole, an original work of authorship. For the purposes
      of this License, Derivative Works shall not include works that remain
      separable from, or merely link (or bind by name) to the interfaces of,
      the Work and Derivative Works thereof.

      "Contribution" shall mean any work of authorship, including
      the original version of the Work and any modifications or additions
      to that Work or Derivative Works thereof, that is intentionally
      submitted to Licensor for inclusion in the Work by the copyright owner
      or by an individual or Legal Entity authorized to submit on behalf of
      the copyright owner. For the purposes of this definition, "submitted"
      means any form of electronic, verbal, or written communication sent
      to the Licensor or its representatives, including but not limited to
      communication on electronic mailing lists, source code control systems,
      and issue tracking systems that are managed by, or on behalf of, the
      Licensor for the purpose of discussing and improving the Work, but
      excluding communication that is conspicuously marked or otherwise
      designated in writing by the copyright owner as "Not a Contribution."

      "Contributor" shall mean Licensor and any individual or Legal Entity
      on behalf of whom a Contribution has been received by Licensor and
      subsequently incorporated within the Work.

   2. Grant of Copyright License. Subject to the terms and conditions of
      this License, each Contributor hereby grants to You a perpetual,
      worldwide, non-exclusive, no-charge, royalty-free, irrevocable
      copyright license to reproduce, prepare Derivative Works of,
      publicly display, publicly perform, sublicense, and distribute the
      Work and such Derivative Works in Source or Object form.

   3. Grant of Patent License. Subject to the terms and conditions of
      this License, each Contributor hereby grants to You a perpetual,
      worldwide, non-exclusive, no-charge, royalty-free, irrevocable
      (except as stated in this section) patent license to make, have made,
      use, offer to sell, sell, import, and otherwise transfer the Work,
      where such license applies only to those patent claims licensable
      by such Contributor that are necessarily infringed by their
      Contribution(s) alone or by combination of their Contribution(s)
      with the Work to which such Contribution(s) was submitted. If You
      institute patent litigation against any entity (including a
      cross-claim or counterclaim in a lawsuit) alleging that the Work
      or a Contribution incorporated within the Work constitutes direct
      or contributory patent infringement, then any patent licenses
      granted to You under this License for that Work shall terminate
      as of the date such litigation is filed.

   4. Redistribution. You may reproduce and distribute copies of the
      Work or Derivative Works thereof in any medium, with or without
      modifications, and in Source or Object form, provided that You
      meet the following conditions:

      (a) You must give any other recipients of the Work or
          Derivative Works a copy of this License; and

      (b) You must cause any modified files to carry prominent notices
          stating that You changed the files; and

      (c) You must retain, in the Source form of any Derivative Works
          that You distribute, all copyright, patent, trademark, and
          attribution notices from the Source form of the Work,
          excluding those notices that do not pertain to any part of
          the Derivative Works; and

      (d) If the Work includes a "NOTICE" text file as part of its
          distribution, then any Derivative Works that You distribute must
          include a readable copy of the attribution notices contained
          within such NOTICE file, excluding those notices that do not
          pertain to any part of the Derivative Works, in at least one
          of the following places: within a NOTICE text file distributed
          as part of the Derivative Works; within the Source form or
          documentation, if provided along with the Derivative Works; or,
          within a display generated by the Derivative Works, if and
          wherever such third-party notices normally appear. The contents
          of the NOTICE file are for informational purposes only and
          do not modify the License. You may add Your own attribution
          notices within Derivative Works that You distribute, alongside
          or as an addendum to the NOTICE text from the Work, provided
          that such additional attribution notices cannot be construed
          as modifying the License.

      You may add Your own copyright statement to Your modifications and
      may provide additional or different license terms and conditions
      for use, reproduction, or distribution of Your modifications, or
      for any such Derivative Works as a whole, provided Your use,
      reproduction, and distribution of the Work otherwise complies with
      the conditions stated in this License.

   5. Submission of Contributions. Unless You explicitly state otherwise,
      any Contribution intentionally submitted for inclusion in the Work
      by You to the Licensor shall be under the terms and conditions of
      this License, without any additional terms or conditions.
      Notwithstanding the above, nothing herein shall supersede or modify
      the terms of any separate license agreement you may have executed
      with Licensor regarding such Contributions.

   6. Trademarks. This License does not grant permission to use the trade
      names, trademarks, service marks, or product names of the Licensor,
      except as required for reasonable and customary use in describing the
      origin of the Work and reproducing the content of the NOTICE file.

   7. Disclaimer of Warranty. Unless required by applicable law or
      agreed to in writing, Licensor provides the Work (and each
      Contributor provides its Contributions) on an "AS IS" BASIS,
      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
      implied, including, without limitation, any warranties or conditions
      of TITLE, NON-INFRINGEMENT, MERCHANTABILITY, or FITNESS FOR A
      PARTICULAR PURPOSE. You are solely responsible for determining the
      appropriateness of using or redistributing the Work and assume any
      risks associated with Your exercise of permissions under this License.

   8. Limitation of Liability. In no event and under no legal theory,
      whether in tort (including negligence), contract, or otherwise,
      unless required by applicable law (such as deliberate and grossly
      negligent acts) or agreed to in writing, shall any Contributor be
      liable to You for damages, including any direct, indirect, special,
      incidental, or consequential damages of any character arising as a
      result of this License or out of the use or inability to use the
      Work (including but not limited to damages for loss of goodwill,
      work stoppage, computer failure or malfunction, or any and all
      other commercial damages or losses), even if such Contributor
      has been advised of the possibility of such damages.

   9. Accepting Warranty or Additional Liability. While redistributing
      the Work or Derivative Works thereof, You may choose to offer,
      and charge a fee for, acceptance of support, warranty, indemnity,
      or other liability obligations and/or rights consistent with this
      License. However, in accepting such obligations, You may act only
      on Your own behalf and on Your sole responsibility, not on behalf
      of any other Contributor, and only if You agree to indemnify,
      defend, and hold each Contributor harmless for any liability
      incurred by, or claims asserted against, such Contributor by reason
      of your accepting any such warranty or additional liability.

   END OF TERMS AND CONDITIONS
*/
