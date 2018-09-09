#include "widget.hpp"

Widget::Widget(int aX, int aY, int aW, int aH) : x(aX), y(aY), w(aW), h(aH) {}

bool Widget::onMouseDown(int, int)
{
  return false;
}

bool Widget::onMouseUp(int, int)
{
  return false;
}

void Widget::draw() {}
