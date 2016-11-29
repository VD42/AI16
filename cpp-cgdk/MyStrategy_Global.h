#pragma once

#include "Strategy.h"
#include <map>


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
	bool m_bLaneRush;

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
    model::LaneType GetLane(const model::CircularUnit & unit);
	void ChooseLane();
	std::pair<std::pair<double, double>, bool> GetWaypoint();
	void Update();
	void SetTowerCords();

	bool EnemyTowerNotExists(std::pair<double, double> position);
	bool FriendlyTowerNotExists(std::pair<double, double> position);
	bool BonusNotExists(std::pair<double, double> position);
	bool Tower2Exists();

	bool m_bMasterNotSilent;
	bool OwnLaneControl();

	void ReCheckLane(bool after_death);

	bool m_bEgoistMode = false;

	std::map<long long, model::LaneType> m_mapFriendlyWizardsLane;
	std::map<long long, model::LaneType> m_mapEnemyWizardsLane;

	int m_bBonusTStart = -1;
	int m_bBonusBStart = -1;

	static int SpeedLevel(const model::Wizard & wizard);
	static int RangeLevel(const model::Wizard & wizard);

	bool CanGoToBonus();
};