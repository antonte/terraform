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
  virtual bool OnMouseDown(int x, int y) = 0;
  virtual bool OnMouseUp(int x, int y) = 0;
  virtual void draw() = 0;
};
