#pragma once
#include <vector>

class Widget;

class Ui
{
public:
  bool onMouseDown(int x, int y);
  bool onMouseUp(int x, int y);
  void draw();
  void add(Widget &);

private:
  std::vector<Widget *> widgets;
};
