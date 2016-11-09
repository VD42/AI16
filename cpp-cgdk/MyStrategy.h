#pragma once

#ifndef _MY_STRATEGY_H_
#define _MY_STRATEGY_H_

#include "Strategy.h"
#include "MyStrategy_PathFinding.h"

class MyStrategy : public Strategy
{
public:
	struct CState
	{
		const model::Wizard & self;
		const model::World & world;
		const model::Game & game;
		model::Move & move;

		struct tmp
		{
			static const model::Wizard s;
			static const model::World w;
			static const model::Game g;
			static model::Move m;
		};

		CState(const model::Wizard & s, const model::World & w, const model::Game & g, model::Move & m)
			: self(s), world(w), game(g), move(m)
		{
		}

		CState & operator=(CState && other)
		{
			memcpy(this, &other, sizeof(CState));
			return *this;
		}
	} m_state;

	const model::Game * game;
	std::vector<std::pair<double, double>> m_tWaypoints;

	CPathFinder m_pathFinder;

	MyStrategy();
    void move(const model::Wizard & self, const model::World & world, const model::Game & game, model::Move & move) override;

	void Tick();

	void Init(const model::Game & g);

	const model::LivingUnit * GetNearestTarget();
	void GoTo(std::pair<double, double> point, bool bSecondChance = false);
	std::pair<double, double> GetNextWaypoint();
	std::pair<double, double> GetPreviousWaypoint();

	void CheckBlock();
};

#endif
