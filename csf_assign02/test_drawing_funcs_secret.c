// Secret autograder unit tests for A2 MS1

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image.h"
#include "drawing_funcs.h"
#include "tctest.h"
// TODO: add prototypes for your helper functions

// an expected color identified by a (non-zero) character code
typedef struct {
  char c;
  uint32_t color;
} ExpectedColor;

// struct representing a "picture" of an expected image
typedef struct {
  ExpectedColor colors[80];
  const char *pic;
} Picture;

typedef struct {
  struct Image small;
  struct Image large;
  struct Image tilemap;
  struct Image spritemap;
} TestObjs;

// dimensions and pixel index computation for "small" test image (objs->small)
#define SMALL_W        8
#define SMALL_H        6
#define SMALL_IDX(x,y) ((y)*SMALL_W + (x))

// dimensions of the "large" test image
#define LARGE_W        24
#define LARGE_H        20

// create test fixture data
TestObjs *setup(void) {
  TestObjs *objs = (TestObjs *) malloc(sizeof(TestObjs));
  init_image(&objs->small, SMALL_W, SMALL_H);
  init_image(&objs->large, LARGE_W, LARGE_H);
  objs->tilemap.data = NULL;
  objs->spritemap.data = NULL;
  return objs;
}

// clean up test fixture data
void cleanup(TestObjs *objs) {
  free(objs->small.data);
  free(objs->large.data);
  free(objs->tilemap.data);
  free(objs->spritemap.data);

  free(objs);
}

uint32_t lookup_color(char c, const ExpectedColor *colors) {
  for (unsigned i = 0; ; i++) {
    assert(colors[i].c != 0);
    if (colors[i].c == c) {
      return colors[i].color;
    }
  }
}

void check_picture(struct Image *img, Picture *p) {
  unsigned num_pixels = img->width * img->height;
  assert(strlen(p->pic) == num_pixels);

  for (unsigned i = 0; i < num_pixels; i++) {
    char c = p->pic[i];
    uint32_t expected_color = lookup_color(c, p->colors);
    uint32_t actual_color = img->data[i];
    ASSERT(actual_color == expected_color);
  }
}

// prototypes of test functions
void test_draw_pixel(TestObjs *objs);
#if 0
void test_draw_rect(TestObjs *objs);
void test_draw_circle(TestObjs *objs);
void test_draw_circle_clip(TestObjs *objs);
void test_draw_tile(TestObjs *objs);
void test_draw_sprite(TestObjs *objs);
#endif
void test_draw_pixel_secret_1a(TestObjs *objs);
void test_draw_pixel_secret_1b(TestObjs *objs);
void test_draw_pixel_secret_1c(TestObjs *objs);
void test_draw_pixel_secret_1d(TestObjs *objs);
void test_draw_pixel_stress1(TestObjs *objs);
void test_draw_pixel_stress2(TestObjs *objs);
#if 0
void test_draw_pixel_secret_1(TestObjs *objs);
void test_draw_pixel_secret_2(TestObjs *objs);
void test_draw_rect_secret_1(TestObjs *objs);
void test_draw_rect_secret_2(TestObjs *objs);
void test_draw_circle_secret_1(TestObjs *objs);
void test_draw_circle_secret_2(TestObjs *objs);
void test_draw_tile_secret_1(TestObjs *objs);
void test_draw_tile_secret_2(TestObjs *objs);
void test_draw_sprite_secret_1(TestObjs *objs);
void test_draw_sprite_secret_2(TestObjs *objs);
#endif

int main(int argc, char **argv) {
  if (argc > 1) {
    // user specified a specific test function to run
    tctest_testname_to_execute = argv[1];
  }

  TEST_INIT();

  // TODO: add TEST() directives for your helper functions
  TEST(test_draw_pixel);
#if 0
  TEST(test_draw_rect);
  TEST(test_draw_circle);
  TEST(test_draw_circle_clip);
  TEST(test_draw_tile);
  TEST(test_draw_sprite);
#endif
  TEST(test_draw_pixel_secret_1a);
  TEST(test_draw_pixel_secret_1b);
  TEST(test_draw_pixel_secret_1c);
  TEST(test_draw_pixel_secret_1d);
  TEST(test_draw_pixel_stress1);
  TEST(test_draw_pixel_stress2);
#if 0
  TEST(test_draw_pixel_secret_1);
  TEST(test_draw_pixel_secret_2);
  TEST(test_draw_rect_secret_1);
  TEST(test_draw_rect_secret_2);
  TEST(test_draw_circle_secret_1);
  TEST(test_draw_circle_secret_2);
  TEST(test_draw_tile_secret_1);
  TEST(test_draw_tile_secret_2);
  TEST(test_draw_sprite_secret_1);
  TEST(test_draw_sprite_secret_2);
#endif

  TEST_FINI();
}

void test_draw_pixel(TestObjs *objs) {
  // initially objs->small pixels are opaque black
  ASSERT(objs->small.data[SMALL_IDX(3, 2)] == 0x000000FFU);
  ASSERT(objs->small.data[SMALL_IDX(5, 4)] == 0x000000FFU);

  // test drawing completely opaque pixels
  draw_pixel(&objs->small, 3, 2, 0xFF0000FF); // opaque red
  ASSERT(objs->small.data[SMALL_IDX(3, 2)] == 0xFF0000FF);
  draw_pixel(&objs->small, 5, 4, 0x800080FF); // opaque magenta (half-intensity)
  ASSERT(objs->small.data[SMALL_IDX(5, 4)] == 0x800080FF);

  // test color blending
  draw_pixel(&objs->small, 3, 2, 0x00FF0080); // half-opaque full-intensity green
  ASSERT(objs->small.data[SMALL_IDX(3, 2)] == 0x7F8000FF);
  draw_pixel(&objs->small, 4, 2, 0x0000FF40); // 1/4-opaque full-intensity blue
  ASSERT(objs->small.data[SMALL_IDX(4, 2)] == 0x000040FF);
}

#if 0
void test_draw_rect(TestObjs *objs) {
  struct Rect red_rect = { .x = 2, .y = 2, .width=3, .height=3 };
  struct Rect blue_rect = { .x = 3, .y = 3, .width=3, .height=3 };
  draw_rect(&objs->small, &red_rect, 0xFF0000FF); // red is full-intensity, full opacity
  draw_rect(&objs->small, &blue_rect, 0x0000FF80); // blue is full-intensity, half opacity

  const uint32_t red   = 0xFF0000FF; // expected full red color
  const uint32_t blue  = 0x000080FF; // expected full blue color
  const uint32_t blend = 0x7F0080FF; // expected red/blue blend color
  const uint32_t black = 0x000000FF; // expected black (background) color

  Picture expected = {
    { {'r', red}, {'b', blue}, {'n', blend}, {' ', black} },
    "        "
    "        "
    "  rrr   "
    "  rnnb  "
    "  rnnb  "
    "   bbb  "
  };

  check_picture(&objs->small, &expected);
}

void test_draw_circle(TestObjs *objs) {
  Picture expected = {
    { {' ', 0x000000FF}, {'x', 0x00FF00FF} },
    "   x    "
    "  xxx   "
    " xxxxx  "
    "  xxx   "
    "   x    "
    "        "
  };

  draw_circle(&objs->small, 3, 2, 2, 0x00FF00FF);

  check_picture(&objs->small, &expected);
}

void test_draw_circle_clip(TestObjs *objs) {
  Picture expected = {
    { {' ', 0x000000FF}, {'x', 0x00FF00FF} },
    " xxxxxxx"
    " xxxxxxx"
    "xxxxxxxx"
    " xxxxxxx"
    " xxxxxxx"
    "  xxxxx "
  };

  draw_circle(&objs->small, 4, 2, 4, 0x00FF00FF);

  check_picture(&objs->small, &expected);
}

void test_draw_tile(TestObjs *objs) {
  ASSERT(read_image("img/PrtMimi.png", &objs->tilemap) == IMG_SUCCESS);

  struct Rect r = { .x = 4, .y = 2, .width = 16, .height = 18 };
  draw_rect(&objs->large, &r, 0x1020D0FF);

  struct Rect grass = { .x = 0, .y = 16, .width = 16, .height = 16 };
  draw_tile(&objs->large, 3, 2, &objs->tilemap, &grass);

  Picture pic = {
    {
      { ' ', 0x000000ff },
      { 'a', 0x52ad52ff },
      { 'b', 0x1020d0ff },
      { 'c', 0x257b4aff },
      { 'd', 0x0c523aff },
    },
    "                        "
    "                        "
    "             a     b    "
    "            a      b    "
    "            a     ab    "
    "           ac      b    "
    "           ac a    b    "
    "      a a  ad  a   b    "
    "     a  a aad  aa ab    "
    "     a  a acd aaacab    "
    "    aa  cdacdaddaadb    "
    "     aa cdaddaaddadb    "
    "     da ccaddcaddadb    "
    "    adcaacdaddddcadb    "
    "   aaccacadcaddccaab    "
    "   aacdacddcaadcaaab    "
    "   aaaddddaddaccaacb    "
    "   aaacddcaadacaaadb    "
    "    bbbbbbbbbbbbbbbb    "
    "    bbbbbbbbbbbbbbbb    "
  };

  check_picture(&objs->large, &pic);
}

void test_draw_sprite(TestObjs *objs) {
  ASSERT(read_image("img/NpcGuest.png", &objs->spritemap) == IMG_SUCCESS);

  struct Rect r = { .x = 1, .y = 1, .width = 14, .height = 14 };
  draw_rect(&objs->large, &r, 0x800080FF);

  struct Rect sue = { .x = 128, .y = 136, .width = 16, .height = 15 };
  draw_sprite(&objs->large, 4, 2, &objs->spritemap, &sue);

  Picture pic = {
    {
      { ' ', 0x000000ff },
      { 'a', 0x800080ff },
      { 'b', 0x9cadc1ff },
      { 'c', 0xefeae2ff },
      { 'd', 0x100000ff },
      { 'e', 0x264c80ff },
      { 'f', 0x314e90ff },
    },
    "                        "
    " aaaaaaaaaaaaaa         "
    " aaaaaaaaaaaaaa         "
    " aaaaaaaaaaaaaa         "
    " aaaaaaabccccccbc       "
    " aaaaacccccccccccc      "
    " aaaacbddcccddcbccc     "
    " aaacbbbeccccedbccc     "
    " aaacbbceccccebbbccc    "
    " aaabbbccccccccbbccc    "
    " aaaabbbcccccccb ccb    "
    " aaaabaaaaabbaa  cb     "
    " aaaaaaaaafffea         "
    " aaaaaaaaaffeea         "
    " aaaaaaacffffcc         "
    "        cffffccb        "
    "         bbbbbbb        "
    "                        "
    "                        "
    "                        "
  };

  check_picture(&objs->large, &pic);
}
#endif

void test_draw_pixel_secret_1a(TestObjs *objs) {
  // test that pixels can be drawn (including correct color blending)
  // in all four corners

  draw_pixel(&objs->small, 0, 0, 0xFC0000FF);

  Picture pic1 = {
    { {' ', 0x000000FF}, {'x', 0xFC0000FF} },
    "x       "
    "        "
    "        "
    "        "
    "        "
    "        "
  };

  check_picture(&objs->small, &pic1);

  draw_pixel(&objs->small, 0, 0, 0x40400080);

  Picture pic2 = {
    { {' ', 0x000000FF}, {'x', 0x9d2000FF} },
    "x       "
    "        "
    "        "
    "        "
    "        "
    "        "
  };

  check_picture(&objs->small, &pic2);
}

void test_draw_pixel_secret_1b(TestObjs *objs) {
  // test that pixels can be drawn (including correct color blending)
  // in all four corners

  draw_pixel(&objs->small, 7, 0, 0xFC0000FF);

  Picture pic1 = {
    { {' ', 0x000000FF}, {'x', 0xFC0000FF} },
    "       x"
    "        "
    "        "
    "        "
    "        "
    "        "
  };

  check_picture(&objs->small, &pic1);

  draw_pixel(&objs->small, 7, 0, 0x40400080);

  Picture pic2 = {
    { {' ', 0x000000FF}, {'x', 0x9d2000FF} },
    "       x"
    "        "
    "        "
    "        "
    "        "
    "        "
  };

  check_picture(&objs->small, &pic2);
}

void test_draw_pixel_secret_1c(TestObjs *objs) {
  // test that pixels can be drawn (including correct color blending)
  // in all four corners

  draw_pixel(&objs->small, 0, 5, 0xFC0000FF);

  Picture pic1 = {
    { {' ', 0x000000FF}, {'x', 0xFC0000FF} },
    "        "
    "        "
    "        "
    "        "
    "        "
    "x       "
  };

  check_picture(&objs->small, &pic1);

  draw_pixel(&objs->small, 0, 5, 0x40400080);

  Picture pic2 = {
    { {' ', 0x000000FF}, {'x', 0x9d2000FF} },
    "        "
    "        "
    "        "
    "        "
    "        "
    "x       "
  };

  check_picture(&objs->small, &pic2);
}

void test_draw_pixel_secret_1d(TestObjs *objs) {
  // test that pixels can be drawn (including correct color blending)
  // in all four corners

  draw_pixel(&objs->small, 7, 5, 0xFC0000FF);

  Picture pic1 = {
    { {' ', 0x000000FF}, {'x', 0xFC0000FF} },
    "        "
    "        "
    "        "
    "        "
    "        "
    "       x"
  };

  check_picture(&objs->small, &pic1);

  draw_pixel(&objs->small, 7, 5, 0x40400080);

  Picture pic2 = {
    { {' ', 0x000000FF}, {'x', 0x9d2000FF} },
    "        "
    "        "
    "        "
    "        "
    "        "
    "       x"
  };

  check_picture(&objs->small, &pic2);
}

void test_draw_pixel_stress1(TestObjs *objs) {
  draw_pixel(&objs->small, 5, 5, 0x019e892b);
  draw_pixel(&objs->small, 6, 4, 0xafcf9e89);
  draw_pixel(&objs->small, 1, 1, 0xbbc76e8b);
  draw_pixel(&objs->small, 4, 3, 0x11480a30);
  draw_pixel(&objs->small, 1, 5, 0xbc602653);
  draw_pixel(&objs->small, 4, 5, 0x75f4f6a4);
  draw_pixel(&objs->small, 0, 1, 0x047a09cc);
  draw_pixel(&objs->small, 7, 3, 0xf8475d92);
  draw_pixel(&objs->small, 2, 0, 0x94a6bb16);
  draw_pixel(&objs->small, 7, 4, 0x4f99a730);
  draw_pixel(&objs->small, 6, 0, 0xcf78514c);
  draw_pixel(&objs->small, 2, 5, 0xa99de830);
  draw_pixel(&objs->small, 1, 3, 0xfe215b1c);
  draw_pixel(&objs->small, 1, 4, 0x0c56332b);
  draw_pixel(&objs->small, 5, 4, 0xc8e9326c);
  draw_pixel(&objs->small, 0, 2, 0x0b9dc442);
  draw_pixel(&objs->small, 6, 3, 0xbd984242);
  draw_pixel(&objs->small, 0, 2, 0x9240184e);
  draw_pixel(&objs->small, 4, 2, 0x45af4570);
  draw_pixel(&objs->small, 1, 5, 0xb93b9a9e);
  draw_pixel(&objs->small, 6, 5, 0x66f7eaab);
  draw_pixel(&objs->small, 6, 3, 0x11fefee2);
  draw_pixel(&objs->small, 3, 1, 0x56ea8db1);
  draw_pixel(&objs->small, 0, 5, 0x69fef5df);
  draw_pixel(&objs->small, 6, 2, 0x41ef50dc);
  draw_pixel(&objs->small, 4, 2, 0xef2664ef);
  draw_pixel(&objs->small, 0, 1, 0x310a7c66);
  draw_pixel(&objs->small, 5, 4, 0x311004f6);
  draw_pixel(&objs->small, 5, 2, 0x68eb20b0);
  draw_pixel(&objs->small, 5, 4, 0x63e45e6c);
  draw_pixel(&objs->small, 7, 0, 0x711ed3c5);
  draw_pixel(&objs->small, 4, 3, 0x4226f588);
  draw_pixel(&objs->small, 1, 1, 0x7ee11864);
  draw_pixel(&objs->small, 2, 2, 0xf4073ce2);
  draw_pixel(&objs->small, 0, 1, 0x7e4a44da);
  draw_pixel(&objs->small, 4, 5, 0xd80f9102);
  draw_pixel(&objs->small, 2, 2, 0xc041b780);
  draw_pixel(&objs->small, 4, 1, 0xb0d70b83);
  draw_pixel(&objs->small, 1, 3, 0x8df01fd6);
  draw_pixel(&objs->small, 0, 1, 0x266f21b0);
  draw_pixel(&objs->small, 4, 1, 0xf2350409);
  draw_pixel(&objs->small, 5, 3, 0x55fa6e64);
  draw_pixel(&objs->small, 0, 3, 0x6e3be6fc);
  draw_pixel(&objs->small, 0, 2, 0x78c0ca19);
  draw_pixel(&objs->small, 4, 5, 0xbafcc346);
  draw_pixel(&objs->small, 4, 4, 0xa511270c);
  draw_pixel(&objs->small, 1, 3, 0x64badc89);
  draw_pixel(&objs->small, 7, 4, 0x509e17df);
  draw_pixel(&objs->small, 5, 0, 0x273ede32);
  draw_pixel(&objs->small, 3, 4, 0x534d45fd);
  draw_pixel(&objs->small, 6, 4, 0x1143f0cc);
  draw_pixel(&objs->small, 1, 4, 0x69c52d9f);
  draw_pixel(&objs->small, 4, 2, 0x5b49d119);
  draw_pixel(&objs->small, 2, 3, 0xbbe11b3e);
  draw_pixel(&objs->small, 6, 3, 0x1daea4ba);
  draw_pixel(&objs->small, 1, 5, 0x3709bd24);
  draw_pixel(&objs->small, 5, 4, 0x2293188c);
  draw_pixel(&objs->small, 7, 0, 0x2b8ebf2b);
  draw_pixel(&objs->small, 6, 0, 0x10f69f8c);
  draw_pixel(&objs->small, 0, 4, 0x1747d94e);
  draw_pixel(&objs->small, 2, 2, 0x9abb859b);
  draw_pixel(&objs->small, 3, 1, 0x2a50eda7);
  draw_pixel(&objs->small, 7, 4, 0x43947c1f);
  draw_pixel(&objs->small, 2, 5, 0xa91a4044);
  draw_pixel(&objs->small, 3, 2, 0xa90601d1);
  draw_pixel(&objs->small, 4, 4, 0x9badfeef);
  draw_pixel(&objs->small, 5, 5, 0xba382811);
  draw_pixel(&objs->small, 3, 2, 0xaf70e5ec);
  draw_pixel(&objs->small, 2, 0, 0x39f51c15);
  draw_pixel(&objs->small, 4, 3, 0xf3bc2de0);
  draw_pixel(&objs->small, 3, 4, 0x423b2bd3);
  draw_pixel(&objs->small, 1, 5, 0x0a16da20);
  draw_pixel(&objs->small, 5, 0, 0xed4197e9);
  draw_pixel(&objs->small, 2, 0, 0xe87a78fe);
  draw_pixel(&objs->small, 6, 0, 0xb6d50667);
  draw_pixel(&objs->small, 5, 2, 0x15d1059d);
  draw_pixel(&objs->small, 0, 4, 0x72461127);
  draw_pixel(&objs->small, 0, 3, 0xcf8de731);
  draw_pixel(&objs->small, 0, 3, 0x39ee3cf2);
  draw_pixel(&objs->small, 3, 4, 0xe60d4fa1);
  draw_pixel(&objs->small, 1, 4, 0x2dc5ccb2);
  draw_pixel(&objs->small, 3, 3, 0x3292f181);
  draw_pixel(&objs->small, 4, 3, 0x98736e44);
  draw_pixel(&objs->small, 0, 5, 0x67608521);
  draw_pixel(&objs->small, 4, 3, 0xc0804213);
  draw_pixel(&objs->small, 5, 5, 0x92efd674);
  draw_pixel(&objs->small, 1, 4, 0x34cf1e9b);
  draw_pixel(&objs->small, 4, 4, 0x01ecd772);
  draw_pixel(&objs->small, 5, 5, 0x001d225a);
  draw_pixel(&objs->small, 6, 4, 0xac6f327a);
  draw_pixel(&objs->small, 6, 1, 0xc8a715d4);
  draw_pixel(&objs->small, 4, 0, 0x0dd51d67);
  draw_pixel(&objs->small, 3, 3, 0xd3acaf89);
  draw_pixel(&objs->small, 2, 5, 0x7942d813);
  draw_pixel(&objs->small, 2, 2, 0x94ea68e1);
  draw_pixel(&objs->small, 0, 5, 0x7a3e96b6);
  draw_pixel(&objs->small, 4, 3, 0xc29de5c0);
  draw_pixel(&objs->small, 2, 3, 0x4523f61f);
  draw_pixel(&objs->small, 0, 1, 0x7dae8db0);
  draw_pixel(&objs->small, 6, 2, 0x13ddba0c);

  //write_image("out/stress1.png", &objs->small);

  Picture pic = {
    {
      { ' ', 0x000000ff },
      { 'a', 0xe77977ff },
      { 'b', 0x05560bff },
      { 'c', 0xd93c8dff },
      { 'd', 0x5eaf3cff },
      { 'e', 0x4f2ba7ff },
      { 'f', 0x68966eff },
      { 'g', 0x6e992dff },
      { 'h', 0x2f6cbcff },
      { 'i', 0x5f6b04ff },
      { 'j', 0xa68a11ff },
      { 'k', 0x353d39ff },
      { 'l', 0x96dd6aff },
      { 'm', 0xac67d3ff },
      { 'n', 0xd32b6aff },
      { 'o', 0x28be0bff },
      { 'p', 0x36ce4aff },
      { 'q', 0x3ce544ff },
      { 'r', 0x6ec082ff },
      { 's', 0x2f3323ff },
      { 't', 0x7c7e96ff },
      { 'u', 0xc29bbdff },
      { 'v', 0x21622bff },
      { 'w', 0x1abcb5ff },
      { 'x', 0x8d2835ff },
      { 'y', 0x171c3aff },
      { 'z', 0x33c24dff },
      { 'A', 0xaa1e43ff },
      { 'B', 0x50c3e3ff },
      { 'C', 0x328020ff },
      { 'D', 0x625c84ff },
      { 'E', 0x468d24ff },
      { 'F', 0x7166a5ff },
      { 'G', 0x6e277cff },
      { 'H', 0x471e3cff },
      { 'I', 0x6ab4a7ff },
      { 'J', 0x2e5953ff },
      { 'K', 0x44a59cff },
    },
    "  a bcde"
    "fg hi j "
    "k lmnop "
    "qrstuvwx"
    "yz ABCDE"
    "FGH IJK "
  };

  check_picture(&objs->small, &pic);
}

void test_draw_pixel_stress2(TestObjs *objs) {
  draw_pixel(&objs->small, 0, 6, 0x7d094ea4);
  draw_pixel(&objs->small, 3, -1, 0x7d7197d1);
  draw_pixel(&objs->small, 1, 4, 0xbac52f14);
  draw_pixel(&objs->small, 2, 2, 0x28998f39);
  draw_pixel(&objs->small, 5, 3, 0x09e26e21);
  draw_pixel(&objs->small, 9, 1, 0x4c5b82ca);
  draw_pixel(&objs->small, 3, -2, 0x52a581a1);
  draw_pixel(&objs->small, 2, 3, 0xb223e76b);
  draw_pixel(&objs->small, 5, -1, 0x3403a3f1);
  draw_pixel(&objs->small, 0, -2, 0xd2d73888);
  draw_pixel(&objs->small, 5, 3, 0x6e6b4b9f);
  draw_pixel(&objs->small, 3, 3, 0xb2f0d657);
  draw_pixel(&objs->small, 8, -1, 0x2cc66456);
  draw_pixel(&objs->small, 5, 3, 0x371676b8);
  draw_pixel(&objs->small, 7, 4, 0x0f4cf98e);
  draw_pixel(&objs->small, -1, -2, 0xe2d4b8f8);
  draw_pixel(&objs->small, -2, -1, 0x8126cb1e);
  draw_pixel(&objs->small, 9, 0, 0x03c8d1b9);
  draw_pixel(&objs->small, -1, -2, 0x8587439e);
  draw_pixel(&objs->small, 6, 1, 0x56deb36a);
  draw_pixel(&objs->small, 6, 4, 0xac54e3b0);
  draw_pixel(&objs->small, -2, -1, 0xde1dd880);
  draw_pixel(&objs->small, 7, 4, 0x1e8abd89);
  draw_pixel(&objs->small, 9, 3, 0x50bf5258);
  draw_pixel(&objs->small, 0, 4, 0x6955a493);
  draw_pixel(&objs->small, 5, 3, 0x2f863299);
  draw_pixel(&objs->small, -1, -2, 0x0dcfe374);
  draw_pixel(&objs->small, 8, 0, 0xed1afef7);
  draw_pixel(&objs->small, 0, 5, 0x83dfd927);
  draw_pixel(&objs->small, 9, -1, 0xc419fe6b);
  draw_pixel(&objs->small, 5, 7, 0x090228b9);
  draw_pixel(&objs->small, 7, 0, 0x75bb1504);
  draw_pixel(&objs->small, 7, 4, 0xef71c5d0);
  draw_pixel(&objs->small, -1, 6, 0x52b0ba76);
  draw_pixel(&objs->small, 2, 0, 0xbee9dc51);
  draw_pixel(&objs->small, 6, -2, 0xd48acf3b);
  draw_pixel(&objs->small, 7, 2, 0x0923c7e2);
  draw_pixel(&objs->small, -2, -1, 0xe4d55022);
  draw_pixel(&objs->small, 4, 7, 0xcd89476a);
  draw_pixel(&objs->small, 0, 1, 0x89f8ca6b);
  draw_pixel(&objs->small, 3, 2, 0xdab1d556);
  draw_pixel(&objs->small, 9, 6, 0x2722ef4e);
  draw_pixel(&objs->small, -1, 6, 0xeff94c7f);
  draw_pixel(&objs->small, 8, 1, 0x61ee0e1a);
  draw_pixel(&objs->small, 2, 3, 0xfc84eac0);
  draw_pixel(&objs->small, 2, -1, 0x7eb17f38);
  draw_pixel(&objs->small, 3, 5, 0xb8109f21);
  draw_pixel(&objs->small, 0, 0, 0xb1abf344);
  draw_pixel(&objs->small, 3, 6, 0x55a4abf0);
  draw_pixel(&objs->small, 9, 4, 0x1ea225bf);
  draw_pixel(&objs->small, 5, 0, 0x40f67ef6);
  draw_pixel(&objs->small, -1, 4, 0x586322cb);
  draw_pixel(&objs->small, 5, 2, 0xbaf77b33);
  draw_pixel(&objs->small, -1, 0, 0x9dd56401);
  draw_pixel(&objs->small, 3, 1, 0xfb9023e5);
  draw_pixel(&objs->small, -1, 4, 0xb7b2f1f7);
  draw_pixel(&objs->small, 6, 7, 0xb0dedb8e);
  draw_pixel(&objs->small, 6, 4, 0x54fec6b3);
  draw_pixel(&objs->small, 1, 7, 0x452b8923);
  draw_pixel(&objs->small, 3, 2, 0xe627e7eb);
  draw_pixel(&objs->small, 2, -1, 0x3cb0d1e5);
  draw_pixel(&objs->small, 0, 4, 0xcd580b8f);
  draw_pixel(&objs->small, 1, 4, 0x5eaf0a35);
  draw_pixel(&objs->small, 6, -2, 0xc4996bfd);
  draw_pixel(&objs->small, 4, 1, 0xc9df765e);
  draw_pixel(&objs->small, 1, 5, 0x03ab82cc);
  draw_pixel(&objs->small, 3, 5, 0x05eca72d);
  draw_pixel(&objs->small, 3, 6, 0xdac21e58);
  draw_pixel(&objs->small, 5, 0, 0xf6c98103);
  draw_pixel(&objs->small, 4, 2, 0x83de4311);
  draw_pixel(&objs->small, 2, 1, 0x2dcfa9ba);
  draw_pixel(&objs->small, 2, -1, 0xa21022f6);
  draw_pixel(&objs->small, 3, -2, 0x1a9fcf80);
  draw_pixel(&objs->small, 9, 4, 0x48e45536);
  draw_pixel(&objs->small, 3, 0, 0x8a35666e);
  draw_pixel(&objs->small, 0, 1, 0x93d7cbab);
  draw_pixel(&objs->small, 7, 0, 0x8d38d158);
  draw_pixel(&objs->small, 3, 1, 0x240acaa3);
  draw_pixel(&objs->small, -2, 3, 0xf1c111b1);
  draw_pixel(&objs->small, -1, 7, 0xf3d215a1);
  draw_pixel(&objs->small, 5, -1, 0x5d1b00a9);
  draw_pixel(&objs->small, 1, 6, 0x291f9bba);
  draw_pixel(&objs->small, 4, 4, 0xa9ff2d31);
  draw_pixel(&objs->small, 9, -1, 0xb47e6732);
  draw_pixel(&objs->small, 4, 2, 0x1a6a6885);
  draw_pixel(&objs->small, 5, 0, 0x4bffb6d2);
  draw_pixel(&objs->small, 7, 0, 0xe3d12461);
  draw_pixel(&objs->small, 2, 1, 0xc3c9a343);
  draw_pixel(&objs->small, 9, 0, 0xbc543651);
  draw_pixel(&objs->small, 3, -1, 0x79ee7a49);
  draw_pixel(&objs->small, 6, 7, 0xaaf24875);
  draw_pixel(&objs->small, 0, 0, 0x9b581341);
  draw_pixel(&objs->small, 0, 6, 0xba537132);
  draw_pixel(&objs->small, 8, 2, 0x4529829a);
  draw_pixel(&objs->small, 3, 2, 0x37843251);
  draw_pixel(&objs->small, -1, 7, 0x2dfdf9ed);
  draw_pixel(&objs->small, -1, 5, 0x2ab51056);
  draw_pixel(&objs->small, 3, 0, 0x238bc4ff);
  draw_pixel(&objs->small, 6, 6, 0xda500f58);
  draw_pixel(&objs->small, -1, -1, 0x3e547240);
  draw_pixel(&objs->small, -2, 4, 0x3291cfad);
  draw_pixel(&objs->small, -1, -1, 0xf111d3e3);
  draw_pixel(&objs->small, 1, 7, 0x0699a6f2);
  draw_pixel(&objs->small, 8, 3, 0x6c03cdfb);
  draw_pixel(&objs->small, 0, -2, 0x0edaef72);
  draw_pixel(&objs->small, 7, 5, 0xdc0018a8);
  draw_pixel(&objs->small, 7, 3, 0x590fd476);
  draw_pixel(&objs->small, 6, 7, 0x6e7e06e2);
  draw_pixel(&objs->small, 6, 1, 0x697efffb);
  draw_pixel(&objs->small, 7, 6, 0x9dde93b5);
  draw_pixel(&objs->small, 3, 6, 0x8f4ff348);
  draw_pixel(&objs->small, 2, 0, 0x2ac861cb);
  draw_pixel(&objs->small, -1, 6, 0xe700e6e6);
  draw_pixel(&objs->small, -1, 7, 0x04dd960c);
  draw_pixel(&objs->small, 1, 7, 0xc5d3549a);
  draw_pixel(&objs->small, 3, -1, 0xfa452dc2);
  draw_pixel(&objs->small, 2, 6, 0xa900dc84);
  draw_pixel(&objs->small, -1, -2, 0xb3f757e8);
  draw_pixel(&objs->small, 7, 2, 0xae60f81d);
  draw_pixel(&objs->small, 3, 6, 0x0b8feb7d);
  draw_pixel(&objs->small, 1, 6, 0x09e1f5fa);
  draw_pixel(&objs->small, -2, 1, 0xee6a4e32);
  draw_pixel(&objs->small, 6, 6, 0x02aa87cd);
  draw_pixel(&objs->small, 2, 2, 0x0f4ebd9c);
  draw_pixel(&objs->small, 4, 7, 0x2ab5af48);
  draw_pixel(&objs->small, -2, 7, 0x157b9eef);
  draw_pixel(&objs->small, 4, -2, 0xb4def33f);
  draw_pixel(&objs->small, 3, 5, 0x2f575591);
  draw_pixel(&objs->small, 9, 5, 0x51c5c74b);
  draw_pixel(&objs->small, 7, 7, 0x6863b27c);
  draw_pixel(&objs->small, 7, 1, 0xdb8311fe);
  draw_pixel(&objs->small, 3, 6, 0xfc942258);
  draw_pixel(&objs->small, 6, 6, 0x3a4d4101);
  draw_pixel(&objs->small, 0, 6, 0x2b49b3bb);
  draw_pixel(&objs->small, 0, 0, 0xb262c16a);
  draw_pixel(&objs->small, 9, 6, 0x1706737d);
  draw_pixel(&objs->small, 9, 7, 0x09293e94);
  draw_pixel(&objs->small, 0, 4, 0xccef7ea0);
  draw_pixel(&objs->small, 9, 7, 0x35beb0f0);
  draw_pixel(&objs->small, 3, 4, 0xe1c5a238);
  draw_pixel(&objs->small, 5, 1, 0x18f27d7b);
  draw_pixel(&objs->small, 6, 5, 0xdb769cc4);
  draw_pixel(&objs->small, 7, 4, 0x8d92702c);
  draw_pixel(&objs->small, -1, 4, 0x5129c49b);
  draw_pixel(&objs->small, 6, 6, 0xce5e8b7e);
  draw_pixel(&objs->small, 9, 7, 0x5d4ef53d);
  draw_pixel(&objs->small, 0, 2, 0xa13c2fef);
  draw_pixel(&objs->small, 8, 0, 0xb8b28db4);
  draw_pixel(&objs->small, -1, -2, 0x77dfa0a7);
  draw_pixel(&objs->small, -2, 4, 0x695c31bb);
  draw_pixel(&objs->small, 4, 1, 0x02769285);
  draw_pixel(&objs->small, 2, 6, 0x8c6fc7e1);
  draw_pixel(&objs->small, 4, 1, 0x1f013035);
  draw_pixel(&objs->small, -2, 7, 0xc510e65a);
  draw_pixel(&objs->small, 0, 0, 0xe32e670b);
  draw_pixel(&objs->small, 5, 7, 0xb37b0f16);
  draw_pixel(&objs->small, 9, 7, 0x577cfd76);
  draw_pixel(&objs->small, 1, 3, 0x835d022f);
  draw_pixel(&objs->small, 6, 4, 0xea6a6cf1);
  draw_pixel(&objs->small, 1, -1, 0xe9e256c5);
  draw_pixel(&objs->small, 4, 5, 0xdb710947);
  draw_pixel(&objs->small, 0, 6, 0x7e41f54d);
  draw_pixel(&objs->small, 0, 2, 0x903c22af);
  draw_pixel(&objs->small, 1, 5, 0x51b1beb9);
  draw_pixel(&objs->small, -1, 6, 0xad6ecada);
  draw_pixel(&objs->small, 1, 6, 0x8544d363);
  draw_pixel(&objs->small, 6, -2, 0x5c4d8ea5);
  draw_pixel(&objs->small, 7, -1, 0x7787c22f);
  draw_pixel(&objs->small, 9, 6, 0xd8b91e2a);
  draw_pixel(&objs->small, 7, 4, 0x4199f2eb);
  draw_pixel(&objs->small, 2, -2, 0x3fdcb686);
  draw_pixel(&objs->small, 5, 2, 0x8a6ad1b8);
  draw_pixel(&objs->small, 8, 7, 0x7054d039);
  draw_pixel(&objs->small, -2, 0, 0xabc17419);
  draw_pixel(&objs->small, 9, 6, 0x9562a7cd);
  draw_pixel(&objs->small, 9, 7, 0xd59b7e67);
  draw_pixel(&objs->small, 3, 0, 0xbb911579);
  draw_pixel(&objs->small, 4, 3, 0x3a742d33);
  draw_pixel(&objs->small, 1, 7, 0x3b0edba6);
  draw_pixel(&objs->small, 7, -2, 0x74b6ca3c);
  draw_pixel(&objs->small, -2, 3, 0x64aacaaf);
  draw_pixel(&objs->small, 1, 4, 0xbf69f346);
  draw_pixel(&objs->small, 5, 2, 0xd02b7cf6);
  draw_pixel(&objs->small, 5, 2, 0xc97ed9ea);
  draw_pixel(&objs->small, 8, 4, 0xc21bf0d0);
  draw_pixel(&objs->small, 1, -1, 0x12baca0b);
  draw_pixel(&objs->small, 6, -1, 0x17048d6d);
  draw_pixel(&objs->small, 4, 0, 0x7f73349c);
  draw_pixel(&objs->small, 4, -2, 0x9d92caaa);
  draw_pixel(&objs->small, 0, 5, 0x3bfbc784);
  draw_pixel(&objs->small, 8, 7, 0xdc6aff88);
  draw_pixel(&objs->small, 8, 4, 0xe2675e40);
  draw_pixel(&objs->small, 3, -1, 0x6e43aaf8);
  draw_pixel(&objs->small, 7, 1, 0x4e2ffc3a);
  draw_pixel(&objs->small, 8, 5, 0xc0b19f4d);
  draw_pixel(&objs->small, 1, 7, 0xe65ac8f5);
  draw_pixel(&objs->small, 9, 5, 0xe62c1186);
  draw_pixel(&objs->small, 6, 1, 0x593f2fcf);
  draw_pixel(&objs->small, 7, 7, 0xb0a79fe4);
  draw_pixel(&objs->small, 2, 7, 0x5fdbae80);

  //write_image("out/stress2.png", &objs->small);

  Picture pic = {
    {
      { ' ', 0x000000ff },
      { '!', 0x79466dff },
      { '"', 0x2dae5bff },
      { '#', 0x6b8d70ff },
      { '$', 0x4d461fff },
      { '%', 0x48fbabff },
      { '&', 0x745b3aff },
      { '\'', 0x75b2a3ff },
      { '(', 0x4aa385ff },
      { ')', 0x68348cff },
      { '*', 0x224f56ff },
      { '+', 0x0b743cff },
      { ',', 0x5b4a55ff },
      { '-', 0xba6f45ff },
      { '.', 0x913a25ff },
      { '/', 0x0c3c7fff },
      { '0', 0xa545a4ff },
      { '1', 0x113d38ff },
      { '2', 0xc977d1ff },
      { '3', 0x1926b8ff },
      { '4', 0x181100ff },
      { '5', 0xd066c7ff },
      { '6', 0x3c5149ff },
      { '7', 0x0b1709ff },
      { '8', 0x335f45ff },
      { '9', 0x290662ff },
      { ':', 0xb4b060ff },
      { ';', 0x4a3f45ff },
      { '<', 0x312b23ff },
      { '=', 0x203108ff },
      { '>', 0xe26e70ff },
      { '?', 0x4a96ecff },
      { '@', 0x289276ff },
      { 'A', 0x3ba5a6ff },
      { 'B', 0x224443ff },
      { 'C', 0x3c1f02ff },
      { 'D', 0xa85a77ff },
      { 'E', 0x90000fff },
    },
    "! \"#$% &"
    "' ()*+,-"
    ". /012 3"
    " 45678 9"
    ":; <= >?"
    "@A BC DE"
  };

  check_picture(&objs->small, &pic);
}

#if 0
void test_draw_pixel_secret_1(TestObjs *objs) {
  // test that pixels can be drawn (including correct color blending)
  // in all four corners

  draw_pixel(&objs->small, 0, 0, 0xFC0000FF);
  draw_pixel(&objs->small, 7, 0, 0xFC0000FF);
  draw_pixel(&objs->small, 0, 5, 0xFC0000FF);
  draw_pixel(&objs->small, 7, 5, 0xFC0000FF);

  Picture pic1 = {
    { {' ', 0x000000FF}, {'x', 0xFC0000FF} },
    "x      x"
    "        "
    "        "
    "        "
    "        "
    "x      x"
  };

  check_picture(&objs->small, &pic1);

  draw_pixel(&objs->small, 0, 0, 0x40400080);
  draw_pixel(&objs->small, 7, 0, 0x40400080);
  draw_pixel(&objs->small, 0, 5, 0x40400080);
  draw_pixel(&objs->small, 7, 5, 0x40400080);

  Picture pic2 = {
    { {' ', 0x000000FF}, {'x', 0x9d2000FF} },
    "x      x"
    "        "
    "        "
    "        "
    "        "
    "x      x"
  };

  check_picture(&objs->small, &pic2);
}

void test_draw_pixel_secret_2(TestObjs *objs) {
  // test that out of bounds pixels are not drawn

  draw_pixel(&objs->small, -1, 0, 0x9930D6FF);
  draw_pixel(&objs->small, 0, -1, 0x9930D6FF);
  draw_pixel(&objs->small, 8, 0, 0x9930D6FF);
  draw_pixel(&objs->small, 7, -1, 0x9930D6FF);
  draw_pixel(&objs->small, -1, 5, 0x9930D6FF);
  draw_pixel(&objs->small, 0, 6, 0x9930D6FF);
  draw_pixel(&objs->small, 8, 5, 0x9930D6FF);
  draw_pixel(&objs->small, 7, 6, 0x9930D6FF);

  Picture pic = {
    {{' ', 0x000000FF}},
    "        "
    "        "
    "        "
    "        "
    "        "
    "        "
  };

  check_picture(&objs->small, &pic);
}

void test_draw_rect_secret_1(TestObjs *objs) {
  // draw two rectangles which are both partially outside the
  // image bounds, and also overlap with each other
  // (so color blending occurs)

  struct Rect r1 = { .x=-2, .y=-2, .width=8, .height=7 };
  struct Rect r2 = { .x=2, .y=2, .width=8, .height=7 };

  draw_rect(&objs->small, &r1, 0x000060FF);
  draw_rect(&objs->small, &r2, 0xFF000070);

  const uint32_t red   = 0x700000FF; // expected full red color
  const uint32_t blue  = 0x000060FF; // expected full blue color
  const uint32_t blend = 0x700035FF; // expected red/blue blend color
  const uint32_t black = 0x000000FF; // expected black (background) color

  Picture expected = {
    { {'r', red}, {'b', blue}, {'n', blend}, {' ', black} },
    "bbbbbb  "
    "bbbbbb  "
    "bbnnnnrr"
    "bbnnnnrr"
    "bbnnnnrr"
    "  rrrrrr"
  };

  check_picture(&objs->small, &expected);
}

void test_draw_rect_secret_2(TestObjs *objs) {
  // similar to test_draw_rect_secret_1, but different overlap

  struct Rect r1 = { .x=4, .y=-3, .width=9, .height=7 };
  struct Rect r2 = { .x=-10, .y=3, .width=15, .height=88 };

  draw_rect(&objs->small, &r1, 0x000060FF);
  draw_rect(&objs->small, &r2, 0xFF000070);

  const uint32_t red   = 0x700000FF; // expected full red color
  const uint32_t blue  = 0x000060FF; // expected full blue color
  const uint32_t blend = 0x700035FF; // expected red/blue blend color
  const uint32_t black = 0x000000FF; // expected black (background) color

  Picture expected = {
    { {'r', red}, {'b', blue}, {'n', blend}, {' ', black} },
    "    bbbb"
    "    bbbb"
    "    bbbb"
    "rrrrnbbb"
    "rrrrr   "
    "rrrrr   "
  };

  check_picture(&objs->small, &expected);
}

void test_draw_circle_secret_1(TestObjs *objs) {
  draw_circle(&objs->large, 15, 12, 8, 0x000060FF);
  draw_circle(&objs->large, 4, 14, 7, 0xFF000070);

  const uint32_t red   = 0x700000FF; // expected full red color
  const uint32_t blue  = 0x000060FF; // expected full blue color
  const uint32_t blend = 0x700035FF; // expected red/blue blend color
  const uint32_t black = 0x000000FF; // expected black (background) color

  Picture expected = {
    { {'r', red}, {'b', blue}, {'n', blend}, {' ', black} },
    "                        "
    "                        "
    "                        "
    "                        "
    "               b        "
    "            bbbbbbb     "
    "          bbbbbbbbbbb   "
    "    r    bbbbbbbbbbbbb  "
    " rrrrrrr bbbbbbbbbbbbb  "
    "rrrrrrrrnbbbbbbbbbbbbbb "
    "rrrrrrrrnnbbbbbbbbbbbbb "
    "rrrrrrrrnnnbbbbbbbbbbbb "
    "rrrrrrrnnnnbbbbbbbbbbbbb"
    "rrrrrrrrnnnbbbbbbbbbbbb "
    "rrrrrrrrnnnnbbbbbbbbbbb "
    "rrrrrrrrnnnbbbbbbbbbbbb "
    "rrrrrrrrrnnbbbbbbbbbbb  "
    "rrrrrrrrrnnbbbbbbbbbbb  "
    "rrrrrrrrrrbbbbbbbbbbb   "
    "rrrrrrrrr   bbbbbbb     "
  };

  check_picture(&objs->large, &expected);
}

void test_draw_circle_secret_2(TestObjs *objs) {
  draw_circle(&objs->large, 17, 4, 6, 0x000060FF);
  draw_circle(&objs->large, 9, 15, 13, 0xFF000070);

  const uint32_t red   = 0x700000FF; // expected full red color
  const uint32_t blue  = 0x000060FF; // expected full blue color
  const uint32_t blend = 0x700035FF; // expected red/blue blend color
  const uint32_t black = 0x000000FF; // expected black (background) color

  Picture expected = {
    { {'r', red}, {'b', blue}, {'n', blend}, {' ', black} },
    "             bbbbbbbbb  "
    "            bbbbbbbbbbb "
    "         r  bbbbbbbbbbb "
    "    rrrrrrrrnnnbbbbbbbb "
    "   rrrrrrrrnnnnnbbbbbbbb"
    " rrrrrrrrrrrnnnnnnbbbbb "
    "rrrrrrrrrrrrnnnnnnnbbbb "
    "rrrrrrrrrrrrnnnnnnnnbbb "
    "rrrrrrrrrrrrrnnnnnnnbb  "
    "rrrrrrrrrrrrrrnnnnnnn   "
    "rrrrrrrrrrrrrrrrrnrrrr  "
    "rrrrrrrrrrrrrrrrrrrrrr  "
    "rrrrrrrrrrrrrrrrrrrrrr  "
    "rrrrrrrrrrrrrrrrrrrrrr  "
    "rrrrrrrrrrrrrrrrrrrrrr  "
    "rrrrrrrrrrrrrrrrrrrrrrr "
    "rrrrrrrrrrrrrrrrrrrrrr  "
    "rrrrrrrrrrrrrrrrrrrrrr  "
    "rrrrrrrrrrrrrrrrrrrrrr  "
    "rrrrrrrrrrrrrrrrrrrrrr  "
  };

  check_picture(&objs->large, &expected);
}

void test_draw_tile_secret_1(TestObjs *objs) {
  // test that no drawing is done if the tile boundaries are
  // even partially out of bounds in the tilemap

  ASSERT(read_image("img/PrtMimi.png", &objs->tilemap) == IMG_SUCCESS);

  struct Rect oob_left = { .x=249, .y=10, .width=8, .height=8 };
  draw_tile(&objs->large, 5, 5, &objs->tilemap, &oob_left);

  struct Rect oob_bottom = { .x=10, .y=153, .width=8, .height=8 };
  draw_tile(&objs->large, 7, 7, &objs->tilemap, &oob_bottom);

  Picture expected = {
    { {' ', 0x000000FF } },
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
  };

  check_picture(&objs->large, &expected);
}

void test_draw_tile_secret_2(TestObjs *objs) {
  // draw a tile partially out of bounds in the destination image

  ASSERT(read_image("img/PrtMimi.png", &objs->tilemap) == IMG_SUCCESS);

  struct Rect grass = { .x = 0, .y = 16, .width = 16, .height = 16 };
  draw_tile(&objs->large, 11, 9, &objs->tilemap, &grass);

  //write_image("out/tiles2.png", &objs->large);

  Picture expected = {
    {
      { ' ', 0x000000ff },
      { 'a', 0x52ad52ff },
      { 'b', 0x257b4aff },
      { 'c', 0x0c523aff },
    },
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
    "                     a  "
    "                    a   "
    "                    a   "
    "                   ab   "
    "                   ab a "
    "              a a  ac  a"
    "             a  a aac  a"
    "             a  a abc aa"
    "            aa  bcabcacc"
    "             aa bcaccaac"
    "             ca bbaccbac"
  };

  check_picture(&objs->large, &expected);
}

void test_draw_sprite_secret_1(TestObjs *objs) {
  // draw a sprite where the sprite rectangle is partially out of bounds

  ASSERT(read_image("img/NpcGuest.png", &objs->spritemap) == IMG_SUCCESS);

  struct Rect oob = { .x = 3, .y = 169, .width=16, .height = 16 };
  draw_sprite(&objs->large, 3, 2, &objs->spritemap, &oob);

  struct Rect oob2 = { .x = 305, .y = 20, .width=16, .height = 16 };
  draw_sprite(&objs->large, 4, 5, &objs->spritemap, &oob2);

  Picture expected = {
    { {' ', 0x000000FF } },
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
    "                        "
  };

  check_picture(&objs->large, &expected);
}

void test_draw_sprite_secret_2(TestObjs *objs) {
  // draw partially out of bounds sprite

  ASSERT(read_image("img/NpcGuest.png", &objs->spritemap) == IMG_SUCCESS);

  // draw a rectangle so there's a background to layer the sprite on top of
  struct Rect solid = { .x = 2, .y = 1, .width = 22, .height = 19 };
  draw_rect(&objs->large, &solid, 0x000030FF);

  struct Rect frog = { .x = 193, .y = 4, .width = 14, .height = 12 };
  draw_sprite(&objs->large, 8, 7, &objs->spritemap, &frog);

  Picture expected = {
    {
      { ' ', 0x000000ff },
      { 'a', 0x000030ff },
      { 'b', 0x7bda21ff },
      { 'c', 0x198821ff },
      { 'd', 0x100000ff },
      { 'e', 0xefeae2ff },
      { 'f', 0x9cadc1ff },
    },
    "                        "
    "  aaaaaaaaaaaaaaaaaaaaaa"
    "  aaaaaaaaaaaaaaaaaaaaaa"
    "  aaaaaaaaaaaaaaaaaaaaaa"
    "  aaaaaaaaaaaaaaaaaaaaaa"
    "  aaaaaaaaaaaaaaaaaaaaaa"
    "  aaaaaaaaaaaaaaaaaaaaaa"
    "  aaaaaaaabbccaaabbccaaa"
    "  aaaaaaabdbbbbbbdbbccaa"
    "  aaaaaaabdbbbbbbdbbccaa"
    "  aaaaaaabdbbbbbbdbbccaa"
    "  aaaaaaabdbbbbbbdbbccaa"
    "  aaaaaaaebbbbbbbbebccaa"
    "  aaaaaaabddddddddbbffaa"
    "  aaaaaaafbbbbbbbbffbfaa"
    "  aaaaaabeeeefeeeeebbfaa"
    "  aaaaaaaeffedeeffeeffaa"
    "  aaaaaaaeeeefeeeeeeefaa"
    "  aaaaaaabcbcccabcbccaaa"
    "  aaaaaaaaaaaaaaaaaaaaaa"
  };

  check_picture(&objs->large, &expected);
}
#endif
