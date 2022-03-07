#pragma once

u32 getTupleIndexWithIndex(std::vector<std::tuple<u32, f64, u32>> vec, u32 index);

struct Graph
{
  u32 N;                                    //# of nodes
  u32 E = 0;                                //# of edges
  std::vector<std::vector<f64>> edgeMatrix; // connection cost [from][to]
  std::vector<std::string> nodeNames;       // names of individual nodes in order

  explicit Graph(u32 N_) : N(N_), edgeMatrix(Zerovect2(N_, N_, -1.)), nodeNames(std::vector<std::string>(N_, "unnamed_node")) {}

  void addEdge(u32 from, u32 to, f64 cost) // assuming bi-directional edges
  {
    edgeMatrix[from][to] = cost;
    edgeMatrix[to][from] = cost;
    E++;
  }

  std::vector<u32> findOptimalPath_Dijkstra(u32 startIndex, u32 endIndex)
  {
    std::vector<std::tuple<u32, f64, u32>> nodes(N); //<node index> <path cost> <via node index>
    std::vector<u32> visitedNodes;

    for (u32 nodeIndex = 0; nodeIndex < N; nodeIndex++) // initialize the nodes in order
    {
      nodes[nodeIndex] = std::make_tuple(nodeIndex, nodeIndex != startIndex ? std::numeric_limits<f64>::max() : 0, startIndex);
    }

    for (u32 nodeExplore = 0; nodeExplore < N; nodeExplore++) // expand all N nodes (worst-case)
    {
      std::sort(nodes.begin(), nodes.end(),
          [](std::tuple<u32, f64, u32> left, std::tuple<u32, f64, u32> right) { return std::get<1>(left) < std::get<1>(right); }); // sorts the nodes according to their path costs
      std::tuple<u32, f64, u32> currentNode;
      for (u32 nodeIndex = 0; nodeIndex < N; nodeIndex++)
      {
        if (std::find(visitedNodes.begin(), visitedNodes.end(), std::get<0>(nodes[nodeIndex])) == visitedNodes.end()) // node not yet expanded
        {
          currentNode = nodes[nodeIndex]; // get the expanded node - first in the sorted vector not considering nodes already expanded
          break;
        }
      }
      u32 currentNodeIndex = std::get<0>(currentNode); // get the expanded node index
      f64 currentNodeCost = std::get<1>(currentNode);  // get the expanded node cost

      for (u32 edge = 0; edge < edgeMatrix[currentNodeIndex].size(); edge++) // check all connections to current node
      {
        if ((edgeMatrix[currentNodeIndex][edge] >= 0) and (edge != currentNodeIndex)) // check if connection from current node to target node exists
        {
          u32 currentEdgeIndex = getTupleIndexWithIndex(nodes, edge);
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
            [](std::tuple<u32, f64, u32> left, std::tuple<u32, f64, u32> right) { return std::get<0>(left) < std::get<0>(right); }); // sorts the nodes according to their index
        u32 backTracingIndex = endIndex;
        f64 optimalCost = std::get<1>(nodes[backTracingIndex]);
        std::vector<u32> optimalPath;
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
    return std::vector<u32>{0};
  }
};

std::ostream& operator<<(std::ostream& out, std::tuple<u32, f64, u32> tuple);

void findOptimalGraphPathDebug();
