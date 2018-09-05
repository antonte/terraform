#pragma once
#include "widget.hpp"
#include <functional>

class Button : public Widget
{
public:
  Button(int x, int y, int w, int h);
  std::function<void()> onClick;
  std::function<void(bool pressed)> onDraw;

private:
  bool onMouseDown(int x, int y) override;
  bool onMouseUp(int x, int y) override;
  void draw() override;
  bool pressed = false;
};
