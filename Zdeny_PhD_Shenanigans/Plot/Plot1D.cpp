#include "stdafx.h"
#include "Plot1D.h"

std::map<std::string, WindowPlot *> Plot1D::plots;

std::function<void( std::string )> Plot1D::OnClose = []( std::string name )
{

};

void Plot1D::plot( const std::vector<std::vector<double>> &z, std::string name, std::string xlabel, std::string ylabel, std::string zlabel, double xmin, double xmax, double ymin, double ymax, std::string savepath )
{

}

