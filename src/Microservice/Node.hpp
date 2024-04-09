#pragma once

struct Node // TODO!:placeholder for imgui blueprint node editor/json config
{
  std::string kind;
  std::string name;
  std::vector<Node*> inputs;
  std::vector<Node*> outputs;
};
