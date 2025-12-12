#include <conio.h>
#include <graphics.h>
#include <math.h>
#include <stdio.h>

void lineDDA(int x0, int y0, int x1, int y1) {
  int dx = x1 - x0;
  int dy = y1 - y0;

  int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

  float xInc = dx / (float)steps;
  float yInc = dy / (float)steps;

  float x = x0;
  float y = y0;

  for (int i = 0; i <= steps; i++) {
    putpixel(round(x), round(y), WHITE);
    x += xInc;
    y += yInc;
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

  lineDDA(x0, y0, x1, y1);

  getch();
  closegraph();
  return 0;
}
