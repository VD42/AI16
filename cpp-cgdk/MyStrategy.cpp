#include "MyStrategy.h"

#define PI 3.14159265358979323846
#define _USE_MATH_DEFINES

#include <cmath>
#include <cstdlib>
#include <time.h>
#include <limits>

#include "MyStrategy_Settings.h"


MyStrategy::MyStrategy()
{
	srand((unsigned int)time(nullptr));
}

void MyStrategy::move(const model::Wizard & self, const model::World & world, const model::Game & game, model::Move & move)
{
	m_self = &self;
	m_world = &world;
	m_game = &game;
	m_move = &move;

	std::pair<double, double> result = { 0.0, 0.0 };

	AddPower(result, CalcPower(m_game->getMapSize() - 800.0, 800.0, 1.0)); // enemy base

	for (auto & unit : m_world->getBuildings())
	{
		if (unit.getFaction() == m_self->getFaction())
		{
			if (unit.getType() == model::BUILDING_FACTION_BASE)
				AddPower(result, CalcPower(unit, CSettings::PW_FRIENDLY_BASE(*this, unit)));
			else if (unit.getType() == model::BUILDING_GUARDIAN_TOWER)
				AddPower(result, CalcPower(unit, CSettings::PW_FRIENDLY_TOWER(*this, unit)));
		}
		else
		{
			if (unit.getType() == model::BUILDING_FACTION_BASE)
				AddPower(result, CalcPower(unit, CSettings::PW_ENEMY_BASE(*this, unit)));
			else if (unit.getType() == model::BUILDING_GUARDIAN_TOWER)
				AddPower(result, CalcPower(unit, CSettings::PW_ENEMY_TOWER(*this, unit)));
		}
	}

	for (auto & unit : m_world->getWizards())
	{
		if (unit.isMe())
			continue;
		if (unit.getFaction() == m_self->getFaction())
			AddPower(result, CalcPower(unit, CSettings::PW_FRIENDLY_WIZARD(*this, unit)));
		else
			AddPower(result, CalcPower(unit, CSettings::PW_ENEMY_WIZARD(*this, unit)));
	}

	for (auto & unit : m_world->getMinions())
	{
		if (unit.getFaction() == m_self->getFaction())
			AddPower(result, CalcPower(unit, CSettings::PW_FRIENDLY_CREEP(*this, unit)));
		else if (unit.getFaction() == model::FACTION_NEUTRAL)
			AddPower(result, CalcPower(unit, CSettings::PW_NEUTRAL_CREEP(*this, unit)));
		else
			AddPower(result, CalcPower(unit, CSettings::PW_ENEMY_CREEP(*this, unit)));
	}

	for (auto & unit : m_world->getTrees())
		AddPower(result, CalcPower(unit, CSettings::PW_TREE(*this, unit)));

	Step(result);
}

std::pair<double, double> MyStrategy::CalcPower(double X, double Y, double PW)
{
	return{ (X - m_self->getX()) / std::hypot(X - m_self->getX(), Y - m_self->getY()) * PW, (Y - m_self->getY()) / std::hypot(X - m_self->getX(), Y - m_self->getY()) * PW };
}

std::pair<double, double> MyStrategy::CalcPower(const model::CircularUnit & unit, double PW)
{
	return CalcPower(unit.getX(), unit.getY(), PW);
}

void MyStrategy::AddPower(std::pair<double, double> & result, std::pair<double, double> & add)
{
	result.first += add.first;
	result.second += add.second;
}

void MyStrategy::Step(std::pair<double, double> direction)
{
	double angleFrom = m_self->getAngle();
	double angle = m_self->getAngleTo(m_self->getX() + direction.first, m_self->getY() + direction.second);
	
	m_move->setSpeed(0.0);
	m_move->setStrafeSpeed(0.0);

	if (0 <= angle && angle <= PI / 2.0)
	{
		m_move->setTurn(angle);
		m_move->setSpeed(m_game->getWizardForwardSpeed());
	}
	else if (PI / 2.0 < angle && angle <= PI)
	{
		m_move->setTurn(angle - PI / 2.0);
		m_move->setStrafeSpeed(m_game->getWizardForwardSpeed());
	}
	else if (angle > PI)
	{
		// TODO
		m_move->setTurn(angle);
		m_move->setSpeed(m_game->getWizardForwardSpeed());
	}
	else if (-PI / 2.0 <= angle && angle < 0.0)
	{
		m_move->setTurn(angle);
		m_move->setSpeed(m_game->getWizardForwardSpeed());
	}
	else if (-PI <= angle && angle < -PI / 2.0)
	{
		m_move->setTurn(angle + PI / 2.0);
		m_move->setStrafeSpeed(-m_game->getWizardForwardSpeed());
	}
	else if (angle < PI)
	{
		// TODO
		m_move->setTurn(angle);
		m_move->setSpeed(m_game->getWizardForwardSpeed());
	}
}