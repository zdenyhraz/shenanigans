#pragma once
#include "Utils/FunctionsBaseSTL.h"

unsigned getTupleIndexWithIndex(std::vector<std::tuple<unsigned, f64, unsigned>> vec, unsigned index);

struct Graph
{
  unsigned N;                               //# of nodes
  unsigned E = 0;                           //# of edges
  std::vector<std::vector<f64>> edgeMatrix; // connection cost [from][to]
  std::vector<std::string> nodeNames;       // names of individual nodes in order

  Graph(unsigned N_) : N(N_), edgeMatrix(zerovect2(N_, N_, -1.)), nodeNames(std::vector<std::string>(N_, "unnamed_node")){};

  void addEdge(unsigned from, unsigned to, f64 cost) // assuming bi-directional edges
  {
    edgeMatrix[from][to] = cost;
    edgeMatrix[to][from] = cost;
    E++;
  }

  std::vector<unsigned> findOptimalPath_Dijkstra(unsigned startIndex, unsigned endIndex)
  {
    std::vector<std::tuple<unsigned, f64, unsigned>> nodes(N); //<node index> <path cost> <via node index>
    std::vector<unsigned> visitedNodes;

    for (unsigned nodeIndex = 0; nodeIndex < N; nodeIndex++) // initialize the nodes in order
    {
      nodes[nodeIndex] = std::make_tuple(nodeIndex, nodeIndex != startIndex ? std::numeric_limits<f64>::max() : 0, startIndex);
    }

    for (unsigned nodeExplore = 0; nodeExplore < N; nodeExplore++) // expand all N nodes (worst-case)
    {
      std::sort(nodes.begin(), nodes.end(),
          [](std::tuple<unsigned, f64, unsigned> left, std::tuple<unsigned, f64, unsigned> right) { return std::get<1>(left) < std::get<1>(right); }); // sorts the nodes according to their path costs
      std::tuple<unsigned, f64, unsigned> currentNode;
      for (unsigned nodeIndex = 0; nodeIndex < N; nodeIndex++)
      {
        if (std::find(visitedNodes.begin(), visitedNodes.end(), std::get<0>(nodes[nodeIndex])) == visitedNodes.end()) // node not yet expanded
        {
          currentNode = nodes[nodeIndex]; // get the expanded node - first in the sorted vector not considering nodes already expanded
          break;
        }
      }
      unsigned currentNodeIndex = std::get<0>(currentNode); // get the expanded node index
      f64 currentNodeCost = std::get<1>(currentNode);       // get the expanded node cost

      for (unsigned edge = 0; edge < edgeMatrix[currentNodeIndex].size(); edge++) // check all connections to current node
      {
        if ((edgeMatrix[currentNodeIndex][edge] >= 0) and (edge != currentNodeIndex)) // check if connection from current node to target node exists
        {
          unsigned currentEdgeIndex = getTupleIndexWithIndex(nodes, edge);
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
            [](std::tuple<unsigned, f64, unsigned> left, std::tuple<unsigned, f64, unsigned> right) { return std::get<0>(left) < std::get<0>(right); }); // sorts the nodes according to their index
        unsigned backTracingIndex = endIndex;
        f64 optimalCost = std::get<1>(nodes[backTracingIndex]);
        std::vector<unsigned> optimalPath;
        optimalPath.push_back(endIndex);
        do
        {
          backTracingIndex = std::get<2>(nodes[backTracingIndex]);
          optimalPath.push_back(backTracingIndex);
        } while (backTracingIndex != startIndex);
        std::reverse(optimalPath.begin(), optimalPath.end()); // start-to-end

        std::cout << std::endl << "> optimal path indices: ";
        for (usize i = 0; i < optimalPath.size(); i++)
        {
          std::cout << optimalPath[i];
          if (i < optimalPath.size() - 1)
            std::cout << " -> ";
        }

        std::cout << std::endl << "> optimal path node names: ";
        for (usize i = 0; i < optimalPath.size(); i++)
        {
          std::cout << nodeNames[optimalPath[i]];
          if (i < optimalPath.size() - 1)
            std::cout << " -> ";
        }
        std::cout << std::endl << ">   ==================>   optimal cost = " << optimalCost << std::endl;

        return optimalPath;
      }
    }
    return std::vector<unsigned>{0};
  }
};

std::ostream& operator<<(std::ostream& out, std::tuple<unsigned, f64, unsigned> tuple);

void findOptimalGraphPathDebug();
