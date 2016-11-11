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

	AddPower("base", result, CalcPower(m_game->getMapSize() - 800.0, 800.0, 100.0)); // enemy base

	if (m_self->getLife() < m_self->getMaxLife() * 0.5)
		AddPower("heal", result, CalcPower(800.0, m_game->getMapSize() - 800.0, 1000.0)); // friendly base

	for (auto & unit : m_world->getBuildings())
	{
		AddPower("collision", result, CalcPower(unit, CSettings::PW_CIRCULAR_UNIT(*this, unit)));

		if (unit.getFaction() == m_self->getFaction())
		{
			if (unit.getType() == model::BUILDING_FACTION_BASE)
				AddPower("friendly base", result, CalcPower(unit, CSettings::PW_FRIENDLY_BASE(*this, unit)));
			else if (unit.getType() == model::BUILDING_GUARDIAN_TOWER)
				AddPower("friendly tower", result, CalcPower(unit, CSettings::PW_FRIENDLY_TOWER(*this, unit)));
		}
		else
		{
			if (unit.getType() == model::BUILDING_FACTION_BASE)
				AddPower("enemy base", result, CalcPower(unit, CSettings::PW_ENEMY_BASE(*this, unit)));
			else if (unit.getType() == model::BUILDING_GUARDIAN_TOWER)
				AddPower("enemy tower", result, CalcPower(unit, CSettings::PW_ENEMY_TOWER(*this, unit)));
		}
	}

	for (auto & unit : m_world->getWizards())
	{
		if (unit.isMe())
			continue;

		double D = std::hypot(m_self->getX() - unit.getX(), m_self->getY() - unit.getY());
		if (D > m_self->getVisionRange())
			continue;

		AddPower("collision", result, CalcPower(unit, CSettings::PW_CIRCULAR_UNIT(*this, unit)));

		if (unit.getFaction() == m_self->getFaction())
			AddPower("friendly wizard", result, CalcPower(unit, CSettings::PW_FRIENDLY_WIZARD(*this, unit)));
		else
			AddPower("enemy wizard", result, CalcPower(unit, CSettings::PW_ENEMY_WIZARD(*this, unit)));
	}

	for (auto & unit : m_world->getMinions())
	{
		double D = std::hypot(m_self->getX() - unit.getX(), m_self->getY() - unit.getY());
		if (D > m_self->getVisionRange())
			continue;

		AddPower("collision", result, CalcPower(unit, CSettings::PW_CIRCULAR_UNIT(*this, unit)));

		if (unit.getType() == model::MINION_ORC_WOODCUTTER)
		{
			if (unit.getFaction() == m_self->getFaction())
				AddPower("friendly creep", result, CalcPower(unit, CSettings::PW_FRIENDLY_CREEP_ORC(*this, unit)));
			else if (unit.getFaction() == model::FACTION_NEUTRAL)
				AddPower("neutral creep", result, CalcPower(unit, CSettings::PW_NEUTRAL_CREEP_ORC(*this, unit)));
			else
				AddPower("enemy creep", result, CalcPower(unit, CSettings::PW_ENEMY_CREEP_ORC(*this, unit)));
		}
		else if (unit.getType() == model::MINION_FETISH_BLOWDART)
		{
			if (unit.getFaction() == m_self->getFaction())
				AddPower("friendly creep", result, CalcPower(unit, CSettings::PW_FRIENDLY_CREEP_FETISH(*this, unit)));
			else if (unit.getFaction() == model::FACTION_NEUTRAL)
				AddPower("neutral creep", result, CalcPower(unit, CSettings::PW_NEUTRAL_CREEP_FETISH(*this, unit)));
			else
				AddPower("enemy creep", result, CalcPower(unit, CSettings::PW_ENEMY_CREEP_FETISH(*this, unit)));
		}
	}

	for (auto & unit : m_world->getTrees())
	{
		double D = std::hypot(m_self->getX() - unit.getX(), m_self->getY() - unit.getY());
		if (D > m_self->getVisionRange())
			continue;

		AddPower("collision", result, CalcPower(unit, CSettings::PW_CIRCULAR_UNIT(*this, unit)));

		AddPower("tree", result, CalcPower(unit, CSettings::PW_TREE(*this, unit)));
	}

	Step(result, Shoot());
}

std::pair<double, double> MyStrategy::CalcPower(double X, double Y, double PW)
{
	return{ (X - m_self->getX()) / std::hypot(X - m_self->getX(), Y - m_self->getY()) * PW, (Y - m_self->getY()) / std::hypot(X - m_self->getX(), Y - m_self->getY()) * PW };
}

std::pair<double, double> MyStrategy::CalcPower(const model::CircularUnit & unit, double PW)
{
	return CalcPower(unit.getX(), unit.getY(), PW);
}

void MyStrategy::AddPower(std::string comment, std::pair<double, double> & result, std::pair<double, double> add)
{
	m_tPowers.push_back(std::make_pair(comment, add));
	result.first += add.first;
	result.second += add.second;
}

bool MyStrategy::Shoot()
{
	const model::CircularUnit * target = nullptr;
	double MAX_PRIORITY = 0.0;

	for (auto & unit : m_world->getMinions())
	{
		if (unit.getFaction() == m_self->getFaction() || unit.getFaction() == model::FACTION_NEUTRAL)
			continue;
		double D = std::hypot(m_self->getX() - unit.getX(), m_self->getY() - unit.getY());
		if (D > m_self->getCastRange())
			continue;
		double P = 100.0 * ((unit.getMaxLife() - unit.getLife() + 1.0) / unit.getMaxLife());
		if (P > MAX_PRIORITY)
		{
			target = &unit;
			MAX_PRIORITY = P;
		}
	}

	for (auto & unit : m_world->getWizards())
	{
		if (unit.getFaction() == m_self->getFaction())
			continue;
		double D = std::hypot(m_self->getX() - unit.getX(), m_self->getY() - unit.getY());
		if (D > m_self->getCastRange())
			continue;
		double P = 75.0 * ((unit.getMaxLife() - unit.getLife() + 1.0) / unit.getMaxLife());
		if (P > MAX_PRIORITY)
		{
			target = &unit;
			MAX_PRIORITY = P;
		}
	}

	for (auto & unit : m_world->getBuildings())
	{
		if (unit.getFaction() == m_self->getFaction())
			continue;
		double D = std::hypot(m_self->getX() - unit.getX(), m_self->getY() - unit.getY());
		if (D > m_self->getCastRange())
			continue;
		double P = 50.0 * ((unit.getMaxLife() - unit.getLife() + 1.0) / unit.getMaxLife());
		if (P > MAX_PRIORITY)
		{
			target = &unit;
			MAX_PRIORITY = P;
		}
	}

	for (auto & unit : m_world->getTrees())
	{
		double D = std::hypot(m_self->getX() - unit.getX(), m_self->getY() - unit.getY());
		if (D > m_self->getCastRange())
			continue;
		if (D - m_self->getRadius() - unit.getRadius() > 25.0)
			continue;
		double P = 10.0 * (1.0 / (D + 1.0));
		if (P > MAX_PRIORITY)
		{
			target = &unit;
			MAX_PRIORITY = P;
		}
	}

	if (!target)
		return false;

	double D = std::hypot(m_self->getX() - target->getX(), m_self->getY() - target->getY());
	double angle = m_self->getAngleTo(*target);
	m_move->setTurn(angle);
	if (std::abs(angle) < m_game->getStaffSector() / 2.0)
	{
		m_move->setAction(model::ACTION_MAGIC_MISSILE);
		m_move->setCastAngle(angle);
		m_move->setMinCastDistance(D - target->getRadius() + m_game->getMagicMissileRadius());
	}
	return true;
}

void MyStrategy::Step(std::pair<double, double> direction, bool shoot)
{
	double angle = m_self->getAngleTo(m_self->getX() + direction.first, m_self->getY() + direction.second);

	printf("%f / %f / %f - ", direction.first, direction.second, angle);

	if (!shoot)
	{
		if (std::abs(angle) > PI / 30.0)
		{
			m_move->setTurn(angle > 0.0 ? PI / 30.0 : -PI / 30.0);
		}
		else
		{
			m_move->setTurn(angle);
		}
	}
	
	m_move->setSpeed(0.0);
	m_move->setStrafeSpeed(0.0);

	if (-PI / 4.0 <= angle && angle <= PI / 4.0)
	{
		printf("F: %f\r\n", angle);
		m_move->setSpeed(m_game->getWizardForwardSpeed());
	}
	else if (PI / 4.0 < angle && angle <= 3.0 * PI / 4.0)
	{
		printf("R: %f\r\n", angle - PI / 2.0);
		m_move->setStrafeSpeed(m_game->getWizardStrafeSpeed());
	}
	else if (angle > 3.0 * PI / 4.0)
	{
		printf("B: %f\r\n", angle - PI);
		m_move->setSpeed(-m_game->getWizardBackwardSpeed());
	}
	else if (-PI / 4.0 >= angle && angle > -3.0 * PI / 4.0)
	{
		printf("L: %f\r\n", angle + PI / 2.0);
		m_move->setStrafeSpeed(-m_game->getWizardStrafeSpeed());
	}
	else if (angle < -3.0 * PI / 4.0)
	{
		printf("B: %f\r\n", angle + PI);
		m_move->setSpeed(-m_game->getWizardBackwardSpeed());
	}
}