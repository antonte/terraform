#include "ui.hpp"

#include "widget.hpp"
#include <algorithm>

bool Ui::onMouseDown(int x, int y)
{
  for (auto &&w : widgets)
    if (w->onMouseDown(x, y))
      return true;
  return false;
}

bool Ui::onMouseUp(int x, int y)
{
  for (auto &&w : widgets)
    if (w->onMouseUp(x, y))
      return true;
  return false;
}

void Ui::draw()
{
  for (auto &&w : widgets)
    w->draw();
}

void Ui::add(Widget &w)
{
  if (std::find(std::begin(widgets), std::end(widgets), &w) == std::end(widgets))
    widgets.push_back(&w);
}

void Ui::remove(Widget &w)
{
  auto it = std::find(std::begin(widgets), std::end(widgets), &w);
  if (it == std::end(widgets))
    return;
  widgets.erase(it);
}
