#pragma once

template <typename T>
class ScopeExit
{
public:
  ScopeExit(T&& fun) : mFun(std::move(fun)) {}
  ~ScopeExit() { mFun(); }

private:
  T mFun;
};
