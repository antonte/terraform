#pragma once
#include <functional>
#include <map>

class UnSched
{
public:
  UnSched() = default;
  UnSched(std::function<void()> &&aFunc) : func(std::move(aFunc)) {}
  UnSched(const UnSched &unSched) = delete;
  UnSched &operator=(const UnSched &unSched) = delete;
  UnSched &operator=(UnSched &&unSched)
  {
    if (func)
      func();
    func = std::move(unSched.func);
    unSched.func = nullptr;
    return *this;
  }
  UnSched(UnSched &&unSched) : func(std::move(unSched.func)) { unSched.func = nullptr; }
  ~UnSched()
  {
    if (func)
      func();
  }

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
