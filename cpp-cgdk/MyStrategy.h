#pragma once

#include "Strategy.h"
#include "MyStrategy_Global.h"
#include <deque>
#include <set>

class MyStrategy : public Strategy
{
private:
	std::pair<double, double> CalcPower(double X, double Y, double PW);
	std::pair<double, double> CalcPower(const model::CircularUnit & unit, double PW);
	void AddPower(std::string comment, std::pair<double, double> & result, std::pair<double, double> add);

	bool Shoot();
	void Step(std::pair<double, double> direction, bool shoot);

public:
	CGlobal m_global;

	const model::Wizard * m_self;
	const model::World * m_world;
	const model::Game * m_game;
	model::Move * m_move;

	bool m_bSeedReady;

	MyStrategy();
    void move(const model::Wizard & self, const model::World & world, const model::Game & game, model::Move & move) override;

	std::vector<std::pair<std::string, std::pair<double, double >> > m_tPowers;

	std::deque<std::pair<double, double>> m_LastPositions;
	int m_LastShootTick = -1;
	bool m_FreeMode = false;
	int m_FreeModeTick = -1;
	int m_nLastHealTick = -1;

	void BestShoot(const model::CircularUnit & unit, bool turn);

	int m_nLastReceivedTickIndex = -1;

	bool m_bHealMode = false;
	bool m_bVeryHealMode = false;
	bool m_bDoubleHealMode = false;

	bool m_bCreepStop = false;
	model::LaneType m_CreepStopLane = model::_LANE_UNKNOWN_;

	bool m_bNoShoot = false;

	std::vector<model::SkillType> m_tSkillsOrder;

	int m_nLastLevel = 0;

	std::set<model::SkillType> m_setCurrentSkills;

	double GetAttackingCount();
	double GetMaxDangerous();
	bool IsDangerous();
};
