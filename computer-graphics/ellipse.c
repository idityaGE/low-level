#include <conio.h>
#include <graphics.h>
#include <math.h>
#include <stdio.h>

#define color RED

void plot_pixel(int x, int y, int xc, int yc) {
  putpixel(x + xc, y + yc, color);
  putpixel(-x + xc, y + yc, color);
  putpixel(x + xc, -y + yc, color);
  putpixel(-x + xc, -y + yc, color);
}

void draw_ellipse(int xc, int yc, int rx, int ry) {
  float dx, dy, p1, p2, x, y;

  // intial points
  x = 0;
  y = ry;

  // intial decision parameter for region 1
  p1 = (ry * ry) - (rx * rx * ry) +
       (0.25 * rx * rx);

  dx = 2 * ry * ry * x;
  dy = 2 * rx * rx * y;

  while (dx < dy) {
    plot_pixel(x, y, xc, yc);
    if (p1 < 0)
      p1 = p1 + dx + (ry * ry);
    else {
      y--;
      dy = dy - (2 * rx * rx);
      p1 = p1 + dx - dy + (ry * ry);
    }
    dx = dx + (2 * ry * ry);
    x++;
  }

  // intial decision parameter for region 2
  p2 = ((ry * ry) * ((x + 0.5) * (x + 0.5))) +
       ((rx * rx) * ((y - 1) * (y - 1))) -
       (rx * rx * ry * ry);

  while (y >= 0) {
    plot_pixel(x, y, xc, yc);
    if (p2 > 0)
      p2 = p2 + (rx * rx) - dy;
    else {
      x++;
      dx = dx + (2 * ry * ry);
      p2 = p2 + dx - dy + (rx * rx);
    }
    dy = dy - (2 * rx * rx);
    y--;
  }
}

int main() {
  int gd = DETECT, gm;
  clrscr();
  initgraph(&gd, &gm, "C://BGI");
  setcolor(BLUE);
  draw_ellipse(150, 120, 80, 30);
  getchar();
  closegraph();
  return 0;
}