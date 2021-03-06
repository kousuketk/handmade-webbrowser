#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <linux/omapfb.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#define FB_PATH "/dev/fb0"
#define FONT_PATH "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf"
#define WIDTH 2048
#define HEIGHT 2048

unsigned char image[HEIGHT][WIDTH];
unsigned int bgimage[HEIGHT][WIDTH];

// Draw bitmap of text
void draw_bitmap(FT_Bitmap *bitmap, FT_Int x, FT_Int y)
{
  FT_Int i, j, p, q;
  FT_Int x_max = x + bitmap->width;
  FT_Int y_max = y + bitmap->rows;

  for (i = x, p = 0; i < x_max; i++, p++)
  {
    for (j = y, q = 0; j < y_max; j++, q++)
    {
      if (i < 0 || j < 0 || i >= WIDTH || j >= HEIGHT)
        continue;
      image[j][i] |= bitmap->buffer[q * bitmap->width + p];
    }
  }
}

void draw_text(char *text, FT_Vector pen, int target_height,
               FT_Face face, FT_Matrix matrix, FT_GlyphSlot slot)
{
  int n;
  int num_chars = strlen(text);
  for (n = 0; n < num_chars; n++)
  {
    // set transformation and
    // load glyph image into the slot(erase previous one)
    FT_Set_Transform(face, &matrix, &pen);
    FT_Load_Char(face, text[n], FT_LOAD_RENDER);

    // now, draw to our target surface(convert position)
    draw_bitmap(&slot->bitmap, slot->bitmap_left, target_height - slot->bitmap_top);

    // increament pen position
    pen.x += slot->advance.x;
    pen.y += slot->advance.y;
  }
}

// Draw box on backgroud
void draw_box(int x, int y, int width, int height, unsigned int color)
{
  for (int iy = y; iy < height; iy++)
  {
    for (int ix = x; ix < width; ix++)
    {
      bgimage[iy][ix] = color;
    }
  }
}

// Show backgournd and text
void show_image(unsigned int fcolor)
{
  int fd;
  fd = open(FB_PATH, O_RDWR);
  uint32_t *frameBuffer = (uint32_t *)mmap(NULL, WIDTH * HEIGHT * 4, PROT_WRITE, MAP_SHARED, fd, 0);
  for (int iy = 0; iy < HEIGHT; iy++)
  {
    for (int ix = 0; ix < WIDTH; ix++)
    {
      image[iy][ix] == 0 ? bgimage[iy][ix] : image[iy][ix] < 128 ? fcolor + 0x00888888
                                                                 : fcolor;
    }
  }
  msync(frameBuffer, WIDTH * HEIGHT * 4, 0);
  munmap(frameBuffer, WIDTH * HEIGHT * 4);
  close(fd);
}

int main(void)
{
  FT_Library library;
  FT_Face face;
  FT_GlyphSlot slot;
  FT_Matrix matrix;
  FT_Vector pen;
  char *idest;
  char *font;
  char *text;
  double angle;
  int target_height, n, num_chars;

  // initialize library
  font = FONT_PATH;
  FT_Init_FreeType(&library);
  FT_New_Face(library, font, 0, &face);
  FT_Set_Char_Size(face, 12 * 64, 0, 100, 0);
  slot = face->glyph;

  // set up matrix
  angle = 0;
  matrix.xx = (FT_Fixed)(cos(angle) * 0x10000L);
  matrix.xy = (FT_Fixed)(-sin(angle) * 0x10000L);
  matrix.yx = (FT_Fixed)(sin(angle) * 0x10000L);
  matrix.yy = (FT_Fixed)(cos(angle) * 0x10000L);

  target_height = HEIGHT;

  // title
  text = "Test";
  pen.x = 10 * 64;
  pen.y = (target_height - 20) * 64;
  draw_text(text, pen, target_height, face, matrix, slot);

  // URL
  text = "example.com";
  pen.x = 100 * 64;
  pen.y = (target_height - 45) * 64;
  draw_text(text, pen, target_height, face, matrix, slot);

  // Body
  text = "Hello, test GUI!";
  pen.x = 10 * 64;
  pen.y = (target_height - 80) * 64;
  draw_text(text, pen, target_height, face, matrix, slot);

  draw_box(0, 0, 100, 25, 0xFFDDDDDD);     // title tab
  draw_box(100, 0, 2048, 25, 0xFFAAAAAA);  // empty tab space
  draw_box(0, 25, 2048, 50, 0xFFDDDDDD);   // URL bar(back)
  draw_box(100, 30, 700, 48, 0xFFFFFFFF);  // URL bar(textbox)
  draw_box(0, 50, 2048, 2048, 0xFFFFFFFF); // body background

  show_image(0xFF000000);

  FT_Done_Face(face);
  FT_Done_FreeType(library);

  getchar();
  return 0;
}