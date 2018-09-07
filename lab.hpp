#pragma once
#include <memory>
#include <vector>

class Button;
class Library;
class Obj;
class Rend;
class Ui;

class Lab
{
public:
  Lab(Rend&, Library&, Ui&);
  ~Lab();
  bool closed = false;
private:
  Obj *crossIcon;
  std::unique_ptr<Button> closeBtn;
  std::unique_ptr<Button> researchBtn;
  Ui *ui;

};
