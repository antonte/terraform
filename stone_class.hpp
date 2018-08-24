#pragma once
#include <vector>

class Obj;
class Library;

class StoneClass
{
public:
  StoneClass(Library&);
  ~StoneClass();
  std::vector<Obj *> level;
};
