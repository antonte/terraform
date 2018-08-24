#include "stone_class.hpp"
#include <shade/library.hpp>

StoneClass::StoneClass(Library &lib)
{
  level.push_back(lib.getObj("stone_lvl_0"));
}

StoneClass::~StoneClass() {}
