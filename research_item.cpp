#include "research_item.hpp"
#include <sstream>
#include <iomanip>

ResearchItem::ResearchItem(int &aValue,
                           const std::string &aDescription,
                           float aK,
                           const std::string &aUnit,
                           float aImprovement)
  : value(&aValue), description(aDescription), k(aK), unit(aUnit), improvement(aImprovement)
{
}

void ResearchItem::research()
{
  *value *= 1.0f + improvement / 100.0f;
  cost *= 1.0f + std::abs(improvement) / 100.0f;
  improvement *= 0.98f;
}

std::string ResearchItem::getText()
{
  std::ostringstream strm;
  strm.imbue(std::locale(""));
  strm << description << "\n"
       << "(" << std::fixed << std::setprecision(2) << *value * k << unit << " + " << std::fixed
       << std::setprecision(2) << improvement << "%) " << std::fixed << "$" << cost;
  return strm.str();
}
