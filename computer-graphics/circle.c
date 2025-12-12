#include <conio.h>
#include <graphics.h>
#include <stdio.h>

void drawCirclePoints(int xc, int yc, int x, int y) {
  putpixel(xc + x, yc + y, WHITE);
  putpixel(xc - x, yc + y, WHITE);
  putpixel(xc + x, yc - y, WHITE);
  putpixel(xc - x, yc - y, WHITE);
  putpixel(xc + y, yc + x, WHITE);
  putpixel(xc - y, yc + x, WHITE);
  putpixel(xc + y, yc - x, WHITE);
  putpixel(xc - y, yc - x, WHITE);
}

void midpointCircle(int xc, int yc, int r) {
  int x = 0;
  int y = r;

  // Initial decision parameter
  int p = 1 - r;

  drawCirclePoints(xc, yc, x, y);

  while (x < y) {
    x++;

    if (p < 0) {
      p = p + 2 * x + 1;  // Midpoint inside
    } else {
      y--;
      p = p + 2 * (x - y) + 1;  // Midpoint outside
    }

    drawCirclePoints(xc, yc, x, y);
  }
}

int main() {
  int gd = DETECT, gm;
  int xc, yc, r;

  printf("Enter center (xc yc): ");
  scanf("%d %d", &xc, &yc);

  printf("Enter radius: ");
  scanf("%d", &r);

  initgraph(&gd, &gm, "C:\\BGI");

  midpointCircle(xc, yc, r);

  getch();
  closegraph();
  return 0;
}
