#include "sched.hpp"

void Sched::tick()
{
  ++now;
  while (!invokes.empty() && invokes.begin()->first.first <= now)
  {
    auto &&it = invokes.begin();
    auto &&key = it->first;
    it->second();
    invokes.erase(key);
  }
}

UnSched Sched::schedWithUnSched(std::function<void()> &&func, int time)
{
  auto &&key = std::make_pair(now + time, rand());
  while (invokes.find(key) != std::end(invokes))
    key = std::make_pair(now + time, rand());
  invokes.emplace(key, std::move(func));
  return UnSched([this, key]() { invokes.erase(key); });
}

void Sched::operator()(std::function<void()> &&func, int time)
{
  auto &&key = std::make_pair(now + time, rand());
  while (invokes.find(key) != std::end(invokes))
    key = std::make_pair(now + time, rand());
  invokes.emplace(key, std::move(func));
}
