#pragma once

class Widget
{
  friend class Ui;

public:
  Widget(int x, int y, int w, int h);
  virtual ~Widget() = default;

protected:
  int x;
  int y;
  int w;
  int h;

private:
  virtual bool onMouseDown(int x, int y) = 0;
  virtual bool onMouseUp(int x, int y) = 0;
  virtual void draw() = 0;
};
