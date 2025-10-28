#include "Umfeld.h"

using namespace umfeld;

void settings() {
    size(1024, 768);
}

void setup() {
    colorMode(RGB, 1.0, 1.0, 1.0, 1.0);
    noFill();
    stroke(1.0f, 0.25f, 0.35f);
}

void draw() {
    background(0.85f);
    const float size = 50.0f;
    const float x    = width / 2.0f;
    const float y    = height / 2.0f;
    line(x - size, y - size, x + size, y + size);
    line(x - size, y + size, x + size, y - size);
}
