#pragma once

class Window
{
protected:
  // shared window data

public:
  virtual ~Window() = default;
  virtual void Initialize() {}
  virtual void Render() = 0;
};
