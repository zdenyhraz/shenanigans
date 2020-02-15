//various graph optimization functions and features
//updated frequently @ https://github.com/zdenyhraz
//PhD work of Zdenek Hrazdira
//made during 2019

#include "stdafx.h"
#include "graphOptimization.h"

std::ostream& operator<<(std::ostream& out, std::tuple<unsigned, double, unsigned> tuple)
{
	out << "<" << std::get<0>(tuple) << "," << std::get<1>(tuple) << "," << std::get<2>(tuple) << ">";
	return out;
}

unsigned getTupleIndexWithIndex(std::vector<std::tuple<unsigned, double, unsigned>> vec, unsigned index)
{
	for (unsigned i = 0; i < vec.size(); i++)
	{
		if (std::get<0>(vec[i]) == index)
		{
			return i;
		}
	}
	return -1;
}

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
	graph.nodeNames = vector<string>{ "A","B","C","D","E","F","G","H","I" };
	graph.findOptimalPath_Dijkstra(0, 7);
}