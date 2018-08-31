#pragma once
#include <functional>
#include <map>

class UnSched
{
public:
  UnSched(std::function<void()> &&aFunc) : func(std::move(aFunc)) {}
  ~UnSched() { func(); }

private:
  std::function<void()> func;
};

class Sched
{
public:
  void tick();
  UnSched schedWithUnSched(std::function<void()> &&, int time = 0);
  void operator()(std::function<void()> &&, int time = 0);

private:
  int now = 0;
  std::map<std::pair<int, int>, std::function<void()>> invokes;
};
