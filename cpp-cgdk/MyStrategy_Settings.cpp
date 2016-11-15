#include "MyStrategy_Settings.h"
#include <cmath>


const std::function<double(MyStrategy&, const model::Tree&)> CSettings::PW_TREE = [](MyStrategy & strategy, const model::Tree & unit)
{
	return -0.5;
};


const std::function<double(MyStrategy&, const model::Minion&)> CSettings::PW_ENEMY_CREEP_ORC = [](MyStrategy & strategy, const model::Minion & unit)
{
	double DISTANCE = std::hypot(strategy.m_self->getX() - unit.getX(), strategy.m_self->getY() - unit.getY());

	double ENEMY_RANGE = strategy.m_game->getOrcWoodcutterAttackRange() * 5.0 + strategy.m_game->getWizardForwardSpeed() + strategy.m_game->getMinionSpeed();
	double MY_RANGE = strategy.m_self->getCastRange() - unit.getRadius() + strategy.m_game->getMagicMissileRadius();

	double PW = 0.0;

	for (auto & minion : strategy.m_world->getMinions())
	{
		if (minion.getFaction() != strategy.m_self->getFaction() && minion.getFaction() != model::FACTION_NEUTRAL)
			continue;
		double MINION_DISTANCE = std::hypot(minion.getX() - unit.getX(), minion.getY() - unit.getY());
		if (MINION_DISTANCE < DISTANCE)
			PW += 1.0;
	}

	if (PW > 0.0 || HAVE_SHIELD(strategy, *strategy.m_self))
	{
		if (DISTANCE >= MY_RANGE)
			PW = (PW * 200.0) / ((DISTANCE - MY_RANGE) * (DISTANCE - MY_RANGE) + 1.0);
		else if (DISTANCE < MY_RANGE / 2.0)
			PW = -500.0;
		else
			PW = 0.0;
	}
	else
	{
		if (DISTANCE <= ENEMY_RANGE)
			PW = -10000.0;
		else
			PW = 100.0 / ((DISTANCE - MY_RANGE) * (DISTANCE - MY_RANGE) + 1.0);
	}

	return PW;
};

const std::function<double(MyStrategy&, const model::Minion&)> CSettings::PW_ENEMY_CREEP_FETISH = [](MyStrategy & strategy, const model::Minion & unit)
{
	double DISTANCE = std::hypot(strategy.m_self->getX() - unit.getX(), strategy.m_self->getY() - unit.getY());

	double ENEMY_RANGE = strategy.m_game->getFetishBlowdartAttackRange() * 1.5 + strategy.m_game->getWizardForwardSpeed() + strategy.m_game->getMinionSpeed();
	double MY_RANGE = strategy.m_self->getCastRange() - unit.getRadius() + strategy.m_game->getMagicMissileRadius();

	double PW = 0.0;

	for (auto & minion : strategy.m_world->getMinions())
	{
		if (minion.getFaction() != strategy.m_self->getFaction() && minion.getFaction() != model::FACTION_NEUTRAL)
			continue;
		double MINION_DISTANCE = std::hypot(minion.getX() - unit.getX(), minion.getY() - unit.getY());
		if (MINION_DISTANCE < DISTANCE)
			PW += 1.0;
	}

	if (PW > 0.0 || HAVE_SHIELD(strategy, *strategy.m_self))
	{
		if (DISTANCE >= MY_RANGE)
			PW = (PW * 200.0) / ((DISTANCE - MY_RANGE) * (DISTANCE - MY_RANGE) + 1.0);
		else if (DISTANCE < MY_RANGE / 2.0)
			PW = -500.0;
		else
			PW = 0.0;
	}
	else
	{
		if (DISTANCE <= ENEMY_RANGE)
			PW = -10000.0;
		else
			PW = 100.0 / ((DISTANCE - MY_RANGE) * (DISTANCE - MY_RANGE) + 1.0);
	}

	return PW;
};

const std::function<double(MyStrategy&, const model::Building&)> CSettings::PW_ENEMY_TOWER = [](MyStrategy & strategy, const model::Building & unit)
{
	double DISTANCE = std::hypot(strategy.m_self->getX() - unit.getX(), strategy.m_self->getY() - unit.getY());

	double ENEMY_RANGE = strategy.m_game->getGuardianTowerAttackRange() + strategy.m_game->getWizardForwardSpeed();
	double MY_RANGE = strategy.m_self->getCastRange() - unit.getRadius() + strategy.m_game->getMagicMissileRadius();

	double PW = 0.0;

	for (auto & minion : strategy.m_world->getMinions())
	{
		if (minion.getFaction() != strategy.m_self->getFaction() && minion.getFaction() != model::FACTION_NEUTRAL)
			continue;
		double MINION_DISTANCE = std::hypot(minion.getX() - unit.getX(), minion.getY() - unit.getY());
		if (MINION_DISTANCE - unit.getRadius() < DISTANCE - strategy.m_self->getRadius())
			PW += 1.0;
	}

	if (PW > 0.0 || HAVE_SHIELD(strategy, *strategy.m_self))
	{
		if (DISTANCE >= MY_RANGE)
			PW = (PW * 100.0) / ((DISTANCE - MY_RANGE) * (DISTANCE - MY_RANGE) + 1.0);
		else if (DISTANCE < MY_RANGE / 2.0)
			PW = -500.0;
		else
			PW = 0.0;
	}
	else
	{
		if (DISTANCE <= ENEMY_RANGE)
			PW = -10000.0;
		else
			PW = 50.0 / ((DISTANCE - MY_RANGE) * (DISTANCE - MY_RANGE) + 1.0);
	}

	return PW;
};

const std::function<double(MyStrategy&, const model::Wizard&)> CSettings::PW_ENEMY_WIZARD = [](MyStrategy & strategy, const model::Wizard & unit)
{
	double DISTANCE = std::hypot(strategy.m_self->getX() - unit.getX(), strategy.m_self->getY() - unit.getY());

	double ENEMY_RANGE = strategy.m_game->getWizardCastRange() + strategy.m_game->getWizardForwardSpeed() * 2.0;
	double MY_RANGE = strategy.m_self->getCastRange() - unit.getRadius() + strategy.m_game->getMagicMissileRadius();

	double PW = 0.0;

	if ((strategy.m_self->getLife() > unit.getLife()) || HAVE_SHIELD(strategy, *strategy.m_self) || HAVE_EMPOWER(strategy, *strategy.m_self))
	{
		if (DISTANCE >= MY_RANGE)
			PW = (PW * 100.0) / ((DISTANCE - MY_RANGE) * (DISTANCE - MY_RANGE) + 1.0);
		else if (DISTANCE < MY_RANGE / 2.0)
			PW = -500.0;
		else
			PW = 0.0;
	}
	else
	{
		if (DISTANCE <= ENEMY_RANGE)
			PW = -10000.0;
		else
			PW = 50.0 / ((DISTANCE - MY_RANGE) * (DISTANCE - MY_RANGE) + 1.0);
	}

	return PW;
};

const std::function<double(MyStrategy&, const model::Building&)> CSettings::PW_ENEMY_BASE = [](MyStrategy & strategy, const model::Building & unit)
{
	double DISTANCE = std::hypot(strategy.m_self->getX() - unit.getX(), strategy.m_self->getY() - unit.getY());

	double ENEMY_RANGE = strategy.m_game->getFactionBaseAttackRange() + strategy.m_game->getWizardForwardSpeed();
	double MY_RANGE = strategy.m_self->getCastRange() - unit.getRadius() + strategy.m_game->getMagicMissileRadius();

	double PW = 0.0;

	for (auto & minion : strategy.m_world->getMinions())
	{
		if (minion.getFaction() != strategy.m_self->getFaction() && minion.getFaction() != model::FACTION_NEUTRAL)
			continue;
		double MINION_DISTANCE = std::hypot(minion.getX() - unit.getX(), minion.getY() - unit.getY());
		if (MINION_DISTANCE < DISTANCE)
			PW += 1.0;
	}

	if (PW > 0.0 || HAVE_SHIELD(strategy, *strategy.m_self))
	{
		if (DISTANCE >= MY_RANGE)
			PW = (PW * 120.0) / ((DISTANCE - MY_RANGE) * (DISTANCE - MY_RANGE) + 1.0);
		else if (DISTANCE < MY_RANGE / 2.0)
			PW = -500.0;
		else
			PW = 0.0;
	}
	else
	{
		if (DISTANCE <= ENEMY_RANGE)
			PW = -10000.0;
		else
			PW = 60.0 / ((DISTANCE - MY_RANGE) * (DISTANCE - MY_RANGE) + 1.0);
	}

	return PW;
};


const std::function<double(MyStrategy&, const model::Minion&)> CSettings::PW_FRIENDLY_CREEP_ORC = [](MyStrategy & strategy, const model::Minion & unit)
{
	return 0.0;
};

const std::function<double(MyStrategy&, const model::Minion&)> CSettings::PW_FRIENDLY_CREEP_FETISH = [](MyStrategy & strategy, const model::Minion & unit)
{
	return 0.0;
};

const std::function<double(MyStrategy&, const model::Building&)> CSettings::PW_FRIENDLY_TOWER = [](MyStrategy & strategy, const model::Building & unit)
{
	return 0.0;
};

const std::function<double(MyStrategy&, const model::Wizard&)> CSettings::PW_FRIENDLY_WIZARD = [](MyStrategy & strategy, const model::Wizard & unit)
{
	return 0.0;
};

const std::function<double(MyStrategy&, const model::Building&)> CSettings::PW_FRIENDLY_BASE = [](MyStrategy & strategy, const model::Building & unit)
{
	return 0.0;
};


const std::function<double(MyStrategy&, const model::Minion&)> CSettings::PW_NEUTRAL_CREEP_ORC = [](MyStrategy & strategy, const model::Minion & unit)
{
	double DISTANCE = std::hypot(strategy.m_self->getX() - unit.getX(), strategy.m_self->getY() - unit.getY());

	double ENEMY_RANGE = strategy.m_game->getOrcWoodcutterAttackRange() * 5.0 + strategy.m_game->getWizardForwardSpeed() + strategy.m_game->getMinionSpeed();
	double MY_RANGE = strategy.m_self->getCastRange() - unit.getRadius() + strategy.m_game->getMagicMissileRadius();

	double PW = 0.0;

	for (auto & minion : strategy.m_world->getMinions())
	{
		if (minion.getFaction() != strategy.m_self->getFaction() && minion.getFaction() != model::FACTION_NEUTRAL)
			continue;
		double MINION_DISTANCE = std::hypot(minion.getX() - unit.getX(), minion.getY() - unit.getY());
		if (MINION_DISTANCE < DISTANCE)
			PW += 1.0;
	}

	if (PW > 0.0 || HAVE_SHIELD(strategy, *strategy.m_self))
	{
		if (DISTANCE >= MY_RANGE)
			PW = 0.0;
		else if (DISTANCE < MY_RANGE / 2.0)
			PW = -0.5;
		else
			PW = 0.0;
	}
	else
	{
		if (DISTANCE <= ENEMY_RANGE)
			PW = -1.0;
		else
			PW = 0.0;
	}

	return PW;
};

const std::function<double(MyStrategy&, const model::Minion&)> CSettings::PW_NEUTRAL_CREEP_FETISH = [](MyStrategy & strategy, const model::Minion & unit)
{
	double DISTANCE = std::hypot(strategy.m_self->getX() - unit.getX(), strategy.m_self->getY() - unit.getY());

	double ENEMY_RANGE = strategy.m_game->getFetishBlowdartAttackRange() * 1.5 + strategy.m_game->getWizardForwardSpeed() + strategy.m_game->getMinionSpeed();
	double MY_RANGE = strategy.m_self->getCastRange() - unit.getRadius() + strategy.m_game->getMagicMissileRadius();

	double PW = 0.0;

	for (auto & minion : strategy.m_world->getMinions())
	{
		if (minion.getFaction() != strategy.m_self->getFaction() && minion.getFaction() != model::FACTION_NEUTRAL)
			continue;
		double MINION_DISTANCE = std::hypot(minion.getX() - unit.getX(), minion.getY() - unit.getY());
		if (MINION_DISTANCE < DISTANCE)
			PW += 1.0;
	}

	if (PW > 0.0 || HAVE_SHIELD(strategy, *strategy.m_self))
	{
		if (DISTANCE >= MY_RANGE)
			PW = 0.0;
		else if (DISTANCE < MY_RANGE / 2.0)
			PW = -0.5;
		else
			PW = 0.0;
	}
	else
	{
		if (DISTANCE <= ENEMY_RANGE)
			PW = -1.0;
		else
			PW = 0.0;
	}

	return PW;
};


const std::function<double(MyStrategy&, const model::CircularUnit&)> CSettings::PW_CIRCULAR_UNIT = [](MyStrategy & strategy, const model::CircularUnit & unit)
{
	double D = std::hypot(strategy.m_self->getX() - unit.getX(), strategy.m_self->getY() - unit.getY()) - strategy.m_self->getRadius() - unit.getRadius();
	if (D > 20.0)
		return 0.0;
	if (D < 5.0)
		return -10000.0;
	return -((10000.0 / ((D - 4.0) * (D - 4.0))) - 38.0);
};

bool CSettings::HAVE_SHIELD(MyStrategy & strategy, const model::Wizard & wizard)
{
	for (auto status : wizard.getStatuses())
	{
		if (status.getType() == model::STATUS_SHIELDED)
			return true;
	}
	return false;
}

bool CSettings::HAVE_EMPOWER(MyStrategy & strategy, const model::Wizard & wizard)
{
	for (auto status : wizard.getStatuses())
	{
		if (status.getType() == model::STATUS_EMPOWERED)
			return true;
	}
	return false;
}