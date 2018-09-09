#include "text_button.hpp"
#include "rend.hpp"
#include <shade/text.hpp>
#include <shade/shader_program.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

TextButton::TextButton(Rend &aRend,
                       Library &aLib,
                       int x,
                       int y,
                       int w,
                       int h)
  : Widget(x, y, w, h), rend(&aRend), lib(&aLib)
{
}

void TextButton::setText(const std::string &value)
{
  text.clear();
  std::istringstream strm(value);
  std::string line;
  while (getline(strm, line, '\n'))
  {
    text.push_back(std::make_unique<Text>(*lib, "font"));
    text.back()->setText(line);
  }
}

bool TextButton::onMouseDown(int mouseX, int mouseY)
{
  if (mouseX < x || mouseX >= x + w || mouseY < y || mouseY >= y + h)
    return false;
  pressed = true;
  return true;
}

bool TextButton::onMouseUp(int mouseX, int mouseY)
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

void TextButton::draw()
{
  rend->textShad->use();
  auto k = pressed ? 1.03f : 1.0f;
  int yy = 0.0f;
  for (auto &&t: text)
  {
    auto textWidth = t->getWidth() * FontSize;
    auto textHeight = t->getHeight() * FontSize;
    rend->mvp =
      glm::translate(glm::vec3(
        x + (w - textWidth * k) / 2.0f, y + yy * FontSize + (h - textHeight * k) / 2.0f, 0.0f)) *
      glm::scale(glm::vec3(FontSize, FontSize, FontSize) * k);
    rend->mvp.update();
    t->draw();
    ++yy;
  }
}
