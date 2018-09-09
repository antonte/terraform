#pragma once
#include <memory>
#include <vector>

class Button;
class Inst;
class Library;
class Obj;
class ResearchItem;
class TextButton;
class Ui;
class Widget;
struct BotSpecs;

class Lab
{
public:
  Lab(Library &, Inst &, Ui &, BotSpecs &);
  void show();
  void hide();
  bool isVisible() const;
  ~Lab();

private:
  Obj *crossIcon;
  std::unique_ptr<Button> closeBtn;
  std::unique_ptr<Widget> bgPlane;
  std::vector<std::pair<std::unique_ptr<ResearchItem>, std::unique_ptr<TextButton>>> researchItems;
  Ui *ui;
  bool visible = false;
  void show(Button&);
  void hide(Button&);
};
