#include "MyStrategy_PathFinding.h"
#include "MyStrategy.h"
#include <cmath>
#include <limits>


CPathFinder::CPathFinder(MyStrategy & strategy) : m_strategy(strategy)
{
}

void CPathFinder::Init()
{
	m_nTileSize = 10;
	m_nMapSize = (int)(m_strategy.m_state.game.getMapSize() / m_nTileSize) + 1;
}

double GetDistanceTo(int X1, int Y1, double X2, double Y2)
{
	double xRange = (double)X1 - X2;
	double yRange = (double)Y1 - Y2;
	return std::sqrt(xRange * xRange + yRange * yRange);
}

void CPathFinder::PaintMap(std::vector<std::vector<bool>> & map, double X, double Y, double R)
{
	R += m_strategy.m_state.self.getRadius();

	for (int x = (int)((X - R) / m_nTileSize); x <= (int)((X + R) / m_nTileSize); x++)
	{
		for (int y = (int)((Y - R) / m_nTileSize); y <= (int)((Y + R) / m_nTileSize); y++)
		{
			if (x < 0 || y < 0 || x > m_nMapSize - 1 || y > m_nMapSize - 1)
				continue;
			if (
				   GetDistanceTo(x * m_nTileSize, y * m_nTileSize, X, Y) >= R
				|| GetDistanceTo((x + 1) * m_nTileSize, y * m_nTileSize, X, Y) >= R
				|| GetDistanceTo(x * m_nTileSize, (y + 1) * m_nTileSize, X, Y) >= R
				|| GetDistanceTo((x + 1) * m_nTileSize, (y + 1) * m_nTileSize, X, Y) >= R
			)
				map[x][y] = false;
		}
	}
}

std::vector<std::vector<bool>> CPathFinder::GenerateCoverageMap()
{
	std::vector<std::vector<bool>> map(m_nMapSize, std::vector<bool>(m_nMapSize, true));

	for (auto & object : m_strategy.m_state.world.getBuildings())
		PaintMap(map, object.getX(), object.getY(), object.getRadius());

	for (auto & object : m_strategy.m_state.world.getWizards())
	{
		if (object.isMe())
			continue;
		PaintMap(map, object.getX(), object.getY(), object.getRadius());
	}

	for (auto & object : m_strategy.m_state.world.getMinions())
		PaintMap(map, object.getX(), object.getY(), object.getRadius());

	for (auto & object : m_strategy.m_state.world.getTrees())
		PaintMap(map, object.getX(), object.getY(), object.getRadius());

	return map;
}

std::vector<std::pair<double, double>> CPathFinder::SearchPath(double X, double Y, std::vector<std::pair<double, double>> & lastPath)
{
	std::vector<std::vector<bool>> coverageMap = GenerateCoverageMap();

	bool bValid = true;
	int firstStep = -1;
	for (int step = 0; step < (int)lastPath.size(); step++)
	{
		if (
			(int)(lastPath[step].first / m_nTileSize) == (int)(m_strategy.m_state.self.getX() / m_nTileSize)
			&& (int)(lastPath[step].second / m_nTileSize) == (int)(m_strategy.m_state.self.getY() / m_nTileSize)
		)
			firstStep = step;
		if (firstStep == -1)
			continue;
		if (!coverageMap[(int)(lastPath[step].first / m_nTileSize)][(int)(lastPath[step].second / m_nTileSize)])
		{
			bValid = false;
			break;
		}
	}
	if (bValid && firstStep > -1)
		return std::vector<std::pair<double, double>>(lastPath.begin() + firstStep, lastPath.end());

	std::vector<std::vector<int>> wayMap(m_nMapSize, std::vector<int>(m_nMapSize, 0));

	for (int x = 0; x < m_nMapSize; x++)
		for (int y = 0; y < m_nMapSize; y++)
			if (!coverageMap[x][y])
				wayMap[x][y] = std::numeric_limits<int>::max();

	wayMap[(int)(m_strategy.m_state.self.getX() / m_nTileSize)][(int)(m_strategy.m_state.self.getY() / m_nTileSize)] = 1;

	for (int step = 1; step < 1000; step++)
	{
		if (wayMap[(int)(X / m_nTileSize)][(int)(Y / m_nTileSize)] != 0)
			break;
		for (int x = 0; x < m_nMapSize; x++)
			for (int y = 0; y < m_nMapSize; y++)
				if (wayMap[x][y] == step)
				{
					if (x > 0 && wayMap[x - 1][y] == 0) wayMap[x - 1][y] = step + 1;
					if (x < m_nMapSize - 1 && wayMap[x + 1][y] == 0) wayMap[x + 1][y] = step + 1;
					if (y > 0 && wayMap[x][y - 1] == 0) wayMap[x][y - 1] = step + 1;
					if (y < m_nMapSize - 1 && wayMap[x][y + 1] == 0) wayMap[x][y + 1] = step + 1;

					if (x > 0 && y > 0 && wayMap[x - 1][y - 1] == 0) wayMap[x - 1][y - 1] = step + 1;
					if (x < m_nMapSize - 1 && y < m_nMapSize - 1 && wayMap[x + 1][y + 1] == 0) wayMap[x + 1][y + 1] = step + 1;
					if (x < m_nMapSize - 1 && y > 0 && wayMap[x + 1][y - 1] == 0) wayMap[x + 1][y - 1] = step + 1;
					if (x > 0 && y < m_nMapSize - 1 && wayMap[x - 1][y + 1] == 0) wayMap[x - 1][y + 1] = step + 1;
				}
	}
	std::vector<std::pair<double, double>> result;
	if (wayMap[(int)(X / m_nTileSize)][(int)(Y / m_nTileSize)] == std::numeric_limits<int>::max())
		return result;

	int lastX = (int)(X / m_nTileSize);
	int lastY = (int)(Y / m_nTileSize);
	result.insert(result.begin(), std::make_pair(((double)lastX + 0.5) * (double)m_nTileSize, ((double)lastY + 0.5) * (double)m_nTileSize));

	for (int step = wayMap[lastX][lastY] - 1; step > 0; step--)
	{
		if (lastX > 0 && wayMap[lastX - 1][lastY] == step) lastX = lastX - 1;
		else if (lastX < m_nMapSize - 1 && wayMap[lastX + 1][lastY] == step) lastX = lastX + 1;
		else if (lastY > 0 && wayMap[lastX][lastY - 1] == step) lastY = lastY - 1;
		else if (lastY < m_nMapSize - 1 && wayMap[lastX][lastY + 1] == step) lastY = lastY + 1;

		else if (lastX > 0 && lastY > 0 && wayMap[lastX - 1][lastY - 1] == step) { lastX = lastX - 1; lastY = lastY - 1; }
		else if (lastX < m_nMapSize - 1 && lastY < m_nMapSize - 1 && wayMap[lastX + 1][lastY + 1] == step) { lastX = lastX + 1; lastY = lastY + 1; }
		else if (lastX < m_nMapSize - 1 && lastY > 0 && wayMap[lastX + 1][lastY - 1] == step) { lastX = lastX + 1; lastY = lastY - 1; }
		else if (lastX > 0 && lastY < m_nMapSize - 1 && wayMap[lastX - 1][lastY + 1] == step) { lastX = lastX - 1; lastY = lastY + 1; }

		result.insert(result.begin(), std::make_pair(((double)lastX + 0.5) * (double)m_nTileSize, ((double)lastY + 0.5) * (double)m_nTileSize));
	}

	return result;
}