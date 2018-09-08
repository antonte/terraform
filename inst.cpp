#include "inst.hpp"

#include "rend.hpp"
#include "world.hpp"

Inst::Inst(Library &lib) : world(std::make_unique<World>(lib)), rend(std::make_unique<Rend>(lib)) {}

Inst::~Inst() {}
