// various graph optimization functions and features
// updated frequently @ https://github.com/zdenyhraz
// PhD work of Zdenek Hrazdira
// made during 2019

#include "GraphOptimization.hpp"

std::ostream& operator<<(std::ostream& out, std::tuple<u32, f64, u32> tuple)
{
  out << "<" << std::get<0>(tuple) << "," << std::get<1>(tuple) << "," << std::get<2>(tuple) << ">";
  return out;
}

u32 getTupleIndexWithIndex(std::vector<std::tuple<u32, f64, u32>> vec, u32 index)
{
  for (u32 i = 0; i < vec.size(); i++)
  {
    if (std::get<0>(vec[i]) == index)
    {
      return i;
    }
  }
  return -1;
}

// cppcheck-suppress unusedFunction
void findOptimalGraphPathDebug()
{
  Graph graph(9);
  graph.addEdge(0, 1, 0.7);
  graph.addEdge(0, 2, 0.6);
  graph.addEdge(0, 3, 0.5);
  graph.addEdge(3, 4, 1.3);
  graph.addEdge(3, 5, 0.9);
  graph.addEdge(2, 5, 0.5);
  graph.addEdge(5, 6, 2);
  graph.addEdge(5, 7, 3);
  graph.addEdge(1, 8, 0.3);
  graph.addEdge(8, 7, 2);
  graph.addEdge(4, 7, 0.1);
  graph.nodeNames = std::vector<std::string>{"A", "B", "C", "D", "E", "F", "G", "H", "I"};
  graph.findOptimalPath_Dijkstra(0, 7);
}
