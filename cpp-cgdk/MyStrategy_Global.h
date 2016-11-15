#pragma once

#include "Strategy.h"


class MyStrategy;

class CGlobal
{
private:
	MyStrategy & m_strategy;

public:
	enum class LaneState
	{
		TOWER_1,
		TOWER_2,
		BASE
	};

	model::LaneType m_lane;
	bool m_bLaneChoosed;

	LaneState m_top;
	LaneState m_mid;
	LaneState m_bot;

	bool m_bVaidCords;
	std::pair<double, double> m_T1;
	std::pair<double, double> m_T2;
	std::pair<double, double> m_M1;
	std::pair<double, double> m_M2;
	std::pair<double, double> m_B1;
	std::pair<double, double> m_B2;
	std::pair<double, double> m_BS;

	bool m_bBonusT;
	bool m_bBonusB;

	CGlobal(MyStrategy & strategy);
    model::LaneType GetLane(const model::LivingUnit & unit);
	void ChooseLane();
	std::pair<double, double> GetWaypoint();
	void Update();
	void SetTowerCords();
};