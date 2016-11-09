#pragma once

#include <vector>


class MyStrategy;

class CPathFinder
{
private:
	MyStrategy & m_strategy;
	int m_nMapSize;
	int m_nTileSize;

	std::vector<std::vector<bool>> GenerateCoverageMap();
	void PaintMap(std::vector<std::vector<bool>> & map, double X, double Y, double R);

public:
	CPathFinder(MyStrategy & strategy);
	void Init();

	std::vector<std::pair<double, double>> SearchPath(double X, double Y, std::vector<std::pair<double, double>> & lastPath);
};