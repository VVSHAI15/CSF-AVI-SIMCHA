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
  ExpectedColor colors[20];
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
void test_draw_rect(TestObjs *objs);
void test_draw_circle(TestObjs *objs);
void test_draw_circle_clip(TestObjs *objs);
void test_draw_tile(TestObjs *objs);
void test_draw_sprite(TestObjs *objs);
void test_in_bounds();
void test_compute_index();
void test_color_components();
void test_blend_components();
void test_blend_colors();
void test_set_pixel();
void test_square();
void test_square_dist();

int main(int argc, char **argv) {
  if (argc > 1) {
    // user specified a specific test function to run
    tctest_testname_to_execute = argv[1];
  }

  TEST_INIT();

  // TODO: add TEST() directives for your helper functions
  TEST(test_draw_pixel);
  TEST(test_draw_rect);
  TEST(test_draw_circle);
  TEST(test_draw_circle_clip);
  TEST(test_draw_tile);
  TEST(test_draw_sprite);

// Tests for helper functions. 
  TEST(test_in_bounds);
  TEST(test_compute_index);
  TEST(test_color_components);
  TEST(test_blend_components);
  TEST(test_blend_colors);
  TEST(test_set_pixel);
  TEST(test_square);
  TEST(test_square_dist);

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

void test_in_bounds() {
  struct Image img = {100, 100, NULL}; // Test image of size 100x100
  
  // Test corners
  ASSERT(in_bounds(&img, 0, 0)); 
  ASSERT(in_bounds(&img, 99, 99)); 
  ASSERT(!in_bounds(&img, 100, 0)); 
  ASSERT(!in_bounds(&img, 0, 100)); 
  
  // Test beyond corners
  ASSERT(!in_bounds(&img, -1, 0)); 
  ASSERT(!in_bounds(&img, 0, -1)); 
  ASSERT(!in_bounds(&img, 100, 100)); 
  
  // Test middle
  ASSERT(in_bounds(&img, 50, 50)); 
}

void test_compute_index() {
  struct Image img = {100, 100, NULL}; // Test image of size 100x100
  
  // Test first row
  ASSERT(compute_index(&img, 1, 0) == 1); // Second pixel, first row
  
  // Test second row
  ASSERT(compute_index(&img, 0, 1) == 100); // First pixel, second row
  
  // Test last row
  ASSERT(compute_index(&img, 0, 99) == 9900); // First pixel, last row
  
  // Test random middle
  ASSERT(compute_index(&img, 50, 50) == 5050); // Middle pixel 

}

void test_color_components() {
  // Test with a fully opaque white
  uint32_t white = 0xFFFFFFFF; // White color
  ASSERT(get_r(white) == 0xFF); // Red
  ASSERT(get_g(white) == 0xFF); // Green
  ASSERT(get_b(white) == 0xFF); // Blue
  ASSERT(get_a(white) == 0xFF); // Alpha
  

  uint32_t yellow = 0xFFFF00FF; // Solid yellow (R=255, G=255, B=0, A=255)
  ASSERT(get_r(yellow) == 0xFF);
  ASSERT(get_g(yellow) == 0xFF);
  ASSERT(get_b(yellow) == 0x00);
  ASSERT(get_a(yellow) == 0xFF);

  uint32_t cyan = 0x00FFFF80; // Semi-transparent cyan (R=0, G=255, B=255, A=128)
  ASSERT(get_r(cyan) == 0x00);
  ASSERT(get_g(cyan) == 0xFF);
  ASSERT(get_b(cyan) == 0xFF);
  ASSERT(get_a(cyan) == 0x80);


  // Test alpha with a fully transparent color
  uint32_t transparent = 0xFFFFFF00; // Transparent color
  ASSERT(get_a(transparent) == 0x00); // Alpha transparency
}

void test_blend_components() {
  // Test fully opaque foreground
  ASSERT(blend_components(255, 100, 255) == 255); // Opaque foreground
  
  // Test fully transparent foreground
  ASSERT(blend_components(0, 100, 0) == 100); // Transparent foreground
  
  // 50% opacity
  ASSERT(blend_components(255, 0, 128) == 128);
  ASSERT(blend_components(0, 255, 128) == 127);

  // Different opacity levels
  ASSERT(blend_components(255, 0, 25) == 25);
  ASSERT(blend_components(255, 0, 230) == 230);
  ASSERT(blend_components(0, 255, 51) == 204);
}

void test_blend_colors() {
  // Fully opaque foreground over any background should result in the foreground color.
  ASSERT(blend_colors(0xFF0000FF, 0x00FF00FF) == 0xFF0000FF);
  
  // Fully transparent foreground should result in the background color.  
  ASSERT(blend_colors(0x00FF0000, 0xFF0000FF) == 0xFF0000FF);

  // 50% transparent white over black should result in mid-gray with full opacity
  uint32_t result = blend_colors(0xFFFFFF80, 0x000000FF); 
  ASSERT(result == 0x808080FF);

  // We blend semi Opaque blue into fully opaque red. This should result in a purplish tint.
  result = blend_colors(0x0000FF40, 0xFF0000FF);
  
  // Check if the blue component is blended correctly; the result should have more than 0 blue.
  ASSERT((result & 0x0000FF00) > 0);

  // Since we're blending a semi-transparent color over an opaque one, check if the result is indeed opaque.
  ASSERT((result & 0x000000FF) == 0x000000FF);
}

void test_set_pixel() {
  // Small test image
  struct Image img;
  img.width = 2;
  img.height = 2;
  uint32_t data[4] = {0xFFFFFFFF, 0x000000FF, 0xFF0000FF, 0x0000FFFF}; // White, Black, Red, Blue
  img.data = data;

  // Set a semi-transparent green over the first pixel (white), should result in lighter green
  set_pixel(&img, 0, 0x80FF0080);
  ASSERT(img.data[0] != 0xFFFFFFFF);
  
  // Overwrite the second pixel (black) with fully opaque red, should be red
  set_pixel(&img, 1, 0xFF0000FF);
  ASSERT(img.data[1] == 0xFF0000FF);
}


void test_square() {
  // Test with zero
  ASSERT(square(0) == 0);

  // Test with positive numbers
  ASSERT(square(1) == 1);
  ASSERT(square(10) == 100);
  ASSERT(square(123) == 15129);

  // Test with negative numbers
  ASSERT(square(-1) == 1);
  ASSERT(square(-10) == 100);
  ASSERT(square(-123) == 15129);

  // Test with large values
  ASSERT(square(100000) == 10000000000LL);
  ASSERT(square(-100000) == 10000000000LL);
}

void test_square_dist() {
  // Test with points at the same location
  ASSERT(square_dist(0, 0, 0, 0) == 0);

  // Test with points on the x-axis
  ASSERT(square_dist(-1, 0, 1, 0) == 4);

  // Test with points on the y-axis
  ASSERT(square_dist(0, -1, 0, 1) == 4);

  // Test with points diagonally placed
  ASSERT(square_dist(-1, -1, 1, 1) == 8);

  // Test with large values
  ASSERT(square_dist(100000, 100000, -100000, -100000) == 80000000000LL);

  // Test with mixed positive and negative values
  ASSERT(square_dist(-3, 4, 0, 0) == 25);
}