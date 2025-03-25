#pragma once
#include "Math/Functions.hpp"

uint32_t getTupleIndexWithIndex(std::vector<std::tuple<uint32_t, double, uint32_t>> vec, uint32_t index);

struct Graph
{
  uint32_t N;                                  // # of nodes
  uint32_t E = 0;                              // # of edges
  std::vector<std::vector<double>> edgeMatrix; // connection cost [from][to]
  std::vector<std::string> nodeNames;          // names of individual nodes in order

  explicit Graph(uint32_t N_) : N(N_), edgeMatrix(Zerovect2(N_, N_, -1.)), nodeNames(std::vector<std::string>(N_, "unnamed_node")) {}

  void addEdge(uint32_t from, uint32_t to, double cost) // assuming bi-directional edges
  {
    edgeMatrix[from][to] = cost;
    edgeMatrix[to][from] = cost;
    E++;
  }

  std::vector<uint32_t> findOptimalPath_Dijkstra(uint32_t startIndex, uint32_t endIndex)
  {
    std::vector<std::tuple<uint32_t, double, uint32_t>> nodes(N); //<node index> <path cost> <via node index>
    std::vector<uint32_t> visitedNodes;

    for (uint32_t nodeIndex = 0; nodeIndex < N; nodeIndex++) // initialize the nodes in order
    {
      nodes[nodeIndex] = std::make_tuple(nodeIndex, nodeIndex != startIndex ? std::numeric_limits<double>::max() : 0, startIndex);
    }

    for (uint32_t nodeExplore = 0; nodeExplore < N; nodeExplore++) // expand all N nodes (worst-case)
    {
      std::sort(nodes.begin(), nodes.end(),
          [](std::tuple<uint32_t, double, uint32_t> left, std::tuple<uint32_t, double, uint32_t> right)
          { return std::get<1>(left) < std::get<1>(right); }); // sorts the nodes according to their path costs
      std::tuple<uint32_t, double, uint32_t> currentNode;
      for (uint32_t nodeIndex = 0; nodeIndex < N; nodeIndex++)
      {
        if (std::find(visitedNodes.begin(), visitedNodes.end(), std::get<0>(nodes[nodeIndex])) == visitedNodes.end()) // node not yet expanded
        {
          currentNode = nodes[nodeIndex]; // get the expanded node - first in the sorted vector not considering nodes already expanded
          break;
        }
      }
      uint32_t currentNodeIndex = std::get<0>(currentNode); // get the expanded node index
      double currentNodeCost = std::get<1>(currentNode);    // get the expanded node cost

      for (uint32_t edge = 0; edge < edgeMatrix[currentNodeIndex].size(); edge++) // check all connections to current node
      {
        if ((edgeMatrix[currentNodeIndex][edge] >= 0) and (edge != currentNodeIndex)) // check if connection from current node to target node exists
        {
          uint32_t currentEdgeIndex = getTupleIndexWithIndex(nodes, edge);
          if ((currentNodeCost + edgeMatrix[currentNodeIndex][edge]) < std::get<1>(nodes[currentEdgeIndex])) // if a better path is found, save it
          {
            if constexpr (0)
              std::cout << "New path to " << edge << " via " << currentNodeIndex << " found! - previous cost: " << std::get<1>(nodes[currentEdgeIndex])
                        << ", new cost: " << currentNodeCost + edgeMatrix[currentNodeIndex][edge] << std::endl;
            nodes[currentEdgeIndex] = std::make_tuple(edge, currentNodeCost + edgeMatrix[currentNodeIndex][edge], currentNodeIndex);
          }
        }
      }
      visitedNodes.push_back(currentNodeIndex);

      if (std::find(visitedNodes.begin(), visitedNodes.end(), endIndex) != visitedNodes.end()) // target end node reached, optimal solution found
      {
        std::sort(nodes.begin(), nodes.end(),
            [](std::tuple<uint32_t, double, uint32_t> left, std::tuple<uint32_t, double, uint32_t> right)
            { return std::get<0>(left) < std::get<0>(right); }); // sorts the nodes according to their index
        uint32_t backTracingIndex = endIndex;
        double optimalCost = std::get<1>(nodes[backTracingIndex]);
        std::vector<uint32_t> optimalPath;
        optimalPath.push_back(endIndex);
        do
        {
          backTracingIndex = std::get<2>(nodes[backTracingIndex]);
          optimalPath.push_back(backTracingIndex);
        } while (backTracingIndex != startIndex);
        std::reverse(optimalPath.begin(), optimalPath.end()); // start-to-end

        std::cout << std::endl << "> optimal path indices: ";
        for (size_t i = 0; i < optimalPath.size(); i++)
        {
          std::cout << optimalPath[i];
          if (i < optimalPath.size() - 1)
            std::cout << " -> ";
        }

        std::cout << std::endl << "> optimal path node names: ";
        for (size_t i = 0; i < optimalPath.size(); i++)
        {
          std::cout << nodeNames[optimalPath[i]];
          if (i < optimalPath.size() - 1)
            std::cout << " -> ";
        }
        std::cout << std::endl << ">   ==================>   optimal cost = " << optimalCost << std::endl;

        return optimalPath;
      }
    }
    return std::vector<uint32_t>{0};
  }
};

std::ostream& operator<<(std::ostream& out, std::tuple<uint32_t, double, uint32_t> tuple);

void findOptimalGraphPathDebug();
