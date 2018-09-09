#pragma once
#include <string>

class TextButton;
class Ui;

class ResearchItem
{
public:
  ResearchItem(int &value,
               const std::string &description,
               float k,
               const std::string &unit,
               float improvement);
  std::string getText();
  void research();
  int *value;
  std::string description;
  float k;
  std::string unit;
  int cost = 30000000;
  float improvement; // %
};
