#pragma once

struct Node
{
  std::string name;
  std::string kind;
  std::vector<Node*> inputs;
  std::vector<Node*> outputs;
};
