#include "button.hpp"

Button::Button(int x, int y, int w, int h) : Widget(x, y, w, h) {}

bool Button::onMouseDown(int mouseX, int mouseY)
{
  if (mouseX < x || mouseX >= x + w || mouseY < y || mouseY >= y + h)
    return false;
  pressed = true;
  return true;
}

bool Button::onMouseUp(int mouseX, int mouseY)
{
  if (!pressed)
    return false;
  pressed = false;
  if (mouseX < x || mouseX >= x + w || mouseY < y || mouseY >= y + h)
    return false;
  if (!onClick)
    return false;
  onClick();
  return true;
}

void Button::draw()
{
  if (!onDraw)
    return;
  onDraw(pressed);
}
