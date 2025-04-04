#pragma once

class Window
{
protected:
public:
  virtual ~Window() = default;
  virtual void Initialize() {}
  virtual void Render() = 0;
};
