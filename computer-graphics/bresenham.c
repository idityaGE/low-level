#include <conio.h>
#include <graphics.h>
#include <stdio.h>
#include <stdlib.h>

void bresenhamLine(int x0, int y0, int x1, int y1) {
  int dx = abs(x1 - x0);
  int dy = abs(y1 - y0);

  int sx = (x0 < x1) ? 1 : -1;  // step in x direction
  int sy = (y0 < y1) ? 1 : -1;  // step in y direction

  int err = dx - dy;

  while (1) {
    putpixel(x0, y0, WHITE);

    if (x0 == x1 && y0 == y1) break;

    int e2 = 2 * err;

    if (e2 > -dy) {
      err -= dy;
      x0 += sx;
    }
    if (e2 < dx) {
      err += dx;
      y0 += sy;
    }
  }
}

int main() {
  int gd = DETECT, gm;
  int x0, y0, x1, y1;

  printf("Enter x0 y0: ");
  scanf("%d %d", &x0, &y0);
  printf("Enter x1 y1: ");
  scanf("%d %d", &x1, &y1);

  initgraph(&gd, &gm, "C:\\BGI");

  bresenhamLine(x0, y0, x1, y1);

  getch();
  closegraph();
  return 0;
}
