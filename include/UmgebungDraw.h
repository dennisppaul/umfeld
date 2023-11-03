/*
 * Umgebung
 *
 * This file is part of the *Umgebung* library (https://github.com/dennisppaul/umgebung).
 * Copyright (c) 2023 Dennis P Paul.
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <string>
#include "Umgebung.h"

void stroke(float r, float g, float b, float a = 1.0);

void stroke(float a);

void noStroke();

void fill(float r, float g, float b, float a = 1.0);

void fill(float a);

void noFill();

void background(float a, float b, float c, float d = 1.0);

void background(float a);

void rect(float x, float y, float width, float height);

void line(float x1, float y1, float x2, float y2);

PFont *loadFont(const char *file, float size); // @development maybe use smart pointers here

void textFont(PFont *font);

void text(const std::string &text, float x, float y);

PImage *loadImage(const std::string &filename);

void image(PImage *img, float x, float y, float w, float h);

void image(PImage *img, float x, float y);
