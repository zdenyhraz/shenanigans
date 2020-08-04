#pragma once
#include "stdafx.h"
#include "diffrotResults.h"

void SaveDiffrotResultsToFile( const std::string &dir, const std::string &filename, DiffrotResults *dr )
{
	std::string path = dir + filename + ".diffrot";
	std::ofstream file( path, std::ios::out | std::ios::trunc );
	file << "Writing this to a file.\n";
}

void LoadDiffrotResultsFromFile( const std::string &dir, const std::string &filename, DiffrotResults *dr )
{
}
