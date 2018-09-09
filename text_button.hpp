#pragma once
#include "widget.hpp"
#include <functional>
#include <memory>
#include <string>
#include <vector>

class Library;
class Rend;
class Text;

class TextButton: public Widget
{
public:
  TextButton(Rend &, Library &, int x, int y, int w, int h);
  void setText(const std::string& text);
  std::function<void()> onClick;
  static const int FontSize = 23;

private:
  bool onMouseDown(int x, int y) override;
  bool onMouseUp(int x, int y) override;
  void draw() override;
  bool pressed = false;
  Rend *rend;
  Library *lib;
  std::vector<std::unique_ptr<Text>> text;
};
