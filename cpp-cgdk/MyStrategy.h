#pragma once

#include "Strategy.h"

class MyStrategy : public Strategy
{
private:
	std::pair<double, double> CalcPower(double X, double Y, double PW);
	std::pair<double, double> CalcPower(const model::CircularUnit & unit, double PW);
	void AddPower(std::pair<double, double> & result, std::pair<double, double> add);

	bool Shoot();
	void Step(std::pair<double, double> direction);

public:
	const model::Wizard * m_self;
	const model::World * m_world;
	const model::Game * m_game;
	model::Move * m_move;

	MyStrategy();
    void move(const model::Wizard & self, const model::World & world, const model::Game & game, model::Move & move) override;
};
