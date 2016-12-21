#include "MyStrategy_Settings.h"
#include <cmath>
#include <algorithm>


const std::function<double(MyStrategy&, const model::Tree&)> CSettings::PW_TREE = [](MyStrategy & strategy, const model::Tree & unit)
{
	return -(unit.getLife() / 50.0);
};


const std::function<double(MyStrategy&, const model::Minion&)> CSettings::PW_ENEMY_CREEP_ORC = [](MyStrategy & strategy, const model::Minion & unit)
{
	double DISTANCE = std::hypot(strategy.m_self->getX() - unit.getX(), strategy.m_self->getY() - unit.getY());

	double ENEMY_RANGE = strategy.m_game->getOrcWoodcutterAttackRange() + strategy.m_self->getRadius() + strategy.m_game->getWizardForwardSpeed() + strategy.m_game->getMinionSpeed() + 1.0;

	double D = strategy.m_self->getDistanceTo(unit);
	double T = std::min(15.0, D / strategy.m_game->getMagicMissileSpeed());
	double R = std::ceil(T) * strategy.m_game->getMinionSpeed();
	double MY_RANGE = strategy.m_self->getCastRange() + unit.getRadius() - strategy.m_game->getMagicMissileRadius() - R - 0.1 + strategy.m_global.RangeLevel(*strategy.m_self) * strategy.m_game->getRangeBonusPerSkillLevel();

	double PW = 0.0;

	for (auto & minion : strategy.m_world->getMinions())
	{
		if (minion.getFaction() != strategy.m_self->getFaction() && minion.getFaction() != model::FACTION_NEUTRAL)
			continue;
		double MINION_DISTANCE = minion.getDistanceTo(unit);
		if (MINION_DISTANCE <= DISTANCE)
			PW += 1.0;
	}

	for (auto & building : strategy.m_world->getBuildings())
	{
		if (building.getFaction() != strategy.m_self->getFaction())
			continue;
		double ENEMY_RANGE_TO_BUILDING = strategy.m_game->getOrcWoodcutterAttackRange() + building.getRadius() + 1.0;
		if (building.getDistanceTo(unit) < ENEMY_RANGE_TO_BUILDING)
			PW += 10.0;
	}

	if (PW > 0.0 && !strategy.IsDangerous())
	{
		if (DISTANCE >= MY_RANGE)
		{
			PW = (PW * 200.0) / ((DISTANCE - MY_RANGE) * (DISTANCE - MY_RANGE) + 1.0);
		}
		else if (DISTANCE < MY_RANGE / RANGE_WINDOW_COEF(strategy))
		{
			if (PW < 0.5)
			{
				PW = -1750.0;
			}
			else
			{
				bool bFound = false;
				for (auto & wizard : strategy.m_world->getWizards())
				{
					if (wizard.getFaction() == strategy.m_self->getFaction())
						continue;
					if (strategy.m_self->getDistanceTo(wizard) < strategy.m_game->getWizardVisionRange())
					{
						bFound = true;
						break;
					}
				}
				if (bFound && PW < 10.0)
					PW = -1750.0;
				else
					PW = 100.0;
			}
		}
		else
		{
			PW = -10.0;
		}
	}
	else
	{
		if (DISTANCE <= std::max(MY_RANGE / RANGE_WINDOW_COEF(strategy), ENEMY_RANGE * (RANGE_WINDOW_COEF(strategy) + 0.1)))
			PW = -3000.0;
		else
			PW = 100.0 / ((DISTANCE - MY_RANGE) * (DISTANCE - MY_RANGE) + 1.0);
	}

	return PW;
};

const std::function<double(MyStrategy&, const model::Minion&)> CSettings::PW_ENEMY_CREEP_FETISH = [](MyStrategy & strategy, const model::Minion & unit)
{
	double DISTANCE = std::hypot(strategy.m_self->getX() - unit.getX(), strategy.m_self->getY() - unit.getY());

	double ENEMY_RANGE = strategy.m_game->getFetishBlowdartAttackRange() + strategy.m_game->getDartRadius() + strategy.m_self->getRadius() + strategy.m_game->getWizardForwardSpeed() + strategy.m_game->getMinionSpeed() + 1.0;
	
	double D = strategy.m_self->getDistanceTo(unit);
	double T = std::min(15.0, D / strategy.m_game->getMagicMissileSpeed());
	double R = std::ceil(T) * strategy.m_game->getMinionSpeed();
	double MY_RANGE = strategy.m_self->getCastRange() + unit.getRadius() - strategy.m_game->getMagicMissileRadius() - R - 0.1 + strategy.m_global.RangeLevel(*strategy.m_self) * strategy.m_game->getRangeBonusPerSkillLevel();

	double PW = 0.0;

	for (auto & minion : strategy.m_world->getMinions())
	{
		if (minion.getFaction() != strategy.m_self->getFaction() && minion.getFaction() != model::FACTION_NEUTRAL)
			continue;
		double MINION_DISTANCE = minion.getDistanceTo(unit);
		if (MINION_DISTANCE <= DISTANCE)
			PW += 1.0;
	}

	for (auto & building : strategy.m_world->getBuildings())
	{
		if (building.getFaction() != strategy.m_self->getFaction())
			continue;
		double ENEMY_RANGE_TO_BUILDING = strategy.m_game->getFetishBlowdartAttackRange() + strategy.m_game->getDartRadius() + building.getRadius() + 1.0;
		if (building.getDistanceTo(unit) < ENEMY_RANGE_TO_BUILDING)
			PW += 10.0;
	}

	if (PW > 0.0 && !strategy.IsDangerous())
	{
		if (DISTANCE >= MY_RANGE)
		{
			PW = (PW * 200.0) / ((DISTANCE - MY_RANGE) * (DISTANCE - MY_RANGE) + 1.0);
		}
		else if (DISTANCE < MY_RANGE / RANGE_WINDOW_COEF(strategy))
		{
			if (PW < 0.5)
			{
				PW = -1750.0;
			}
			else
			{
				bool bFound = false;
				for (auto & wizard : strategy.m_world->getWizards())
				{
					if (wizard.getFaction() == strategy.m_self->getFaction())
						continue;
					if (strategy.m_self->getDistanceTo(wizard) < strategy.m_game->getWizardVisionRange())
					{
						bFound = true;
						break;
					}
				}
				if (bFound && PW < 10.0)
					PW = -1750.0;
				else
					PW = 100.0;
			}
		}
		else
		{
			PW = -10.0;
		}
	}
	else
	{
		if (DISTANCE <= std::max(MY_RANGE / RANGE_WINDOW_COEF(strategy), ENEMY_RANGE * (RANGE_WINDOW_COEF(strategy) + 0.1)))
			PW = -3000.0;
		else
			PW = 100.0 / ((DISTANCE - MY_RANGE) * (DISTANCE - MY_RANGE) + 1.0);
	}

	return PW;
};

const std::function<double(MyStrategy&, const model::Building&)> CSettings::PW_ENEMY_TOWER = [](MyStrategy & strategy, const model::Building & unit)
{
	bool bImmortal = false;
	if (std::abs(unit.getX() - strategy.m_global.m_T2.first) < 0.1 && std::abs(unit.getY() - strategy.m_global.m_T2.second) < 0.1 && strategy.m_global.m_top == CGlobal::LaneState::TOWER_1)
		bImmortal = true;
	if (std::abs(unit.getX() - strategy.m_global.m_M2.first) < 0.1 && std::abs(unit.getY() - strategy.m_global.m_M2.second) < 0.1 && strategy.m_global.m_mid == CGlobal::LaneState::TOWER_1)
		bImmortal = true;
	if (std::abs(unit.getX() - strategy.m_global.m_B2.first) < 0.1 && std::abs(unit.getY() - strategy.m_global.m_B2.second) < 0.1 && strategy.m_global.m_bot == CGlobal::LaneState::TOWER_1)
		bImmortal = true;
	
	double DISTANCE = std::hypot(strategy.m_self->getX() - unit.getX(), strategy.m_self->getY() - unit.getY());

	double ENEMY_RANGE = strategy.m_game->getGuardianTowerAttackRange() + strategy.m_game->getWizardForwardSpeed() + 20.0;
	double MY_RANGE = strategy.m_self->getCastRange() + unit.getRadius() - strategy.m_game->getMagicMissileRadius() - 0.1 + strategy.m_global.RangeLevel(*strategy.m_self) * strategy.m_game->getRangeBonusPerSkillLevel();

	double PW = 0.0;

	for (auto & minion : strategy.m_world->getMinions())
	{
		if (minion.getFaction() != strategy.m_self->getFaction() && minion.getFaction() != model::FACTION_NEUTRAL)
			continue;
		double MINION_DISTANCE = minion.getDistanceTo(unit);
		if (MINION_DISTANCE <= strategy.m_game->getGuardianTowerAttackRange())
			PW += 1.0;
	}

	for (auto & wizard : strategy.m_world->getWizards())
	{
		if (wizard.getFaction() != strategy.m_self->getFaction())
			continue;
		if (wizard.isMe())
			continue;
		double WIZARD_DISTANCE = wizard.getDistanceTo(unit);
		if (WIZARD_DISTANCE <= strategy.m_game->getGuardianTowerAttackRange())
			PW += 1.0;
	}

	if (strategy.m_global.m_bIsFinal)
		PW += 4.0;

	if (!bImmortal && PW > 1.0 && !strategy.IsDangerous())
	{
		if (DISTANCE >= MY_RANGE)
		{
			PW = (PW * 100.0) / ((DISTANCE - MY_RANGE) * (DISTANCE - MY_RANGE) + 1.0);
		}
		else if (DISTANCE < MY_RANGE / RANGE_WINDOW_COEF(strategy))
		{
			if (PW < 2.5)
			{
				PW = -1750.0;
			}
			else
			{
				bool bFound = false;
				if (!strategy.m_global.m_bIsFinal)
				{
					for (auto & wizard : strategy.m_world->getWizards())
					{
						if (wizard.getFaction() == strategy.m_self->getFaction())
							continue;
						if (strategy.m_self->getDistanceTo(wizard) < strategy.m_game->getWizardVisionRange())
						{
							bFound = true;
							break;
						}
					}
				}
				if (bFound)
					PW = -1750.0;
				else
					PW = 100.0;
			}
		}
		else
		{
			PW = -10.0;
		}
	}
	else
	{
		if (DISTANCE <= std::max(MY_RANGE / RANGE_WINDOW_COEF(strategy), ENEMY_RANGE * (RANGE_WINDOW_COEF(strategy) + 0.1)))
			PW = -3000.0;
		else
			PW = bImmortal ? 0.0 : 50.0 / ((DISTANCE - MY_RANGE) * (DISTANCE - MY_RANGE) + 1.0);
	}

	return PW;
};

const std::function<double(MyStrategy&, const model::Wizard&)> CSettings::PW_ENEMY_WIZARD = [](MyStrategy & strategy, const model::Wizard & unit)
{
	double DISTANCE = std::hypot(strategy.m_self->getX() - unit.getX(), strategy.m_self->getY() - unit.getY());

	if (DISTANCE < strategy.m_self->getVisionRange() + 100.0 && unit.getId() == strategy.m_global.m_nTargetId)
		return 2000.0;

	double ENEMY_RANGE = strategy.m_game->getWizardCastRange() + strategy.m_self->getRadius() + strategy.m_game->getMagicMissileRadius() + strategy.m_game->getWizardForwardSpeed() * 2.0 + 1.0 + strategy.m_global.RangeLevel(unit) * strategy.m_game->getRangeBonusPerSkillLevel();

	double D = strategy.m_self->getDistanceTo(unit);
	double T = std::min(15.0, D / strategy.m_game->getMagicMissileSpeed());
	double R = std::ceil(T) * strategy.m_game->getWizardForwardSpeed();
	double MY_RANGE = strategy.m_self->getCastRange() + unit.getRadius() - strategy.m_game->getMagicMissileRadius() - R - 0.1 + strategy.m_global.RangeLevel(*strategy.m_self) * strategy.m_game->getRangeBonusPerSkillLevel();

	double PW = 0.0;

	for (auto & building : strategy.m_world->getBuildings())
	{
		if (building.getFaction() != strategy.m_self->getFaction())
			continue;
		if (building.getDistanceTo(unit) <= (building.getType() == model::BUILDING_GUARDIAN_TOWER ? strategy.m_game->getGuardianTowerAttackRange() : strategy.m_game->getFactionBaseAttackRange()))
			PW += 1.0;
	}

	for (auto & wizards : strategy.m_world->getWizards())
	{
		if (wizards.getFaction() != strategy.m_self->getFaction())
			continue;
		if (wizards.getDistanceTo(unit) <= strategy.m_game->getWizardCastRange())
			PW += 1.0;
	}

	for (auto & building : strategy.m_world->getBuildings())
	{
		if (building.getFaction() != strategy.m_self->getFaction())
			continue;
		if (building.getType() != model::BUILDING_FACTION_BASE)
			continue;
		double ENEMY_RANGE_TO_BUILDING = strategy.m_game->getWizardCastRange() + building.getRadius() + strategy.m_game->getMagicMissileRadius() + strategy.m_global.RangeLevel(unit) * strategy.m_game->getRangeBonusPerSkillLevel() + 1.0;
		if (building.getDistanceTo(unit) < ENEMY_RANGE_TO_BUILDING)
			PW += 10.0;
	}

	if ((PW > 0.0 || strategy.m_self->getLife() > (unit.getLife() / 2.0) || HAVE_SHIELD(strategy, *strategy.m_self) || HAVE_EMPOWER(strategy, *strategy.m_self)) && !strategy.IsDangerous())
	{
		if (DISTANCE >= MY_RANGE)
		{
			PW = ((PW + (strategy.m_self->getLife() > (unit.getLife() / 2.0) ? 1.0 : 0.0) + (HAVE_SHIELD(strategy, *strategy.m_self) ? 1.0 : 0.0) + (HAVE_EMPOWER(strategy, *strategy.m_self) ? 1.0 : 0.0)) * 150.0) / ((DISTANCE - MY_RANGE) * (DISTANCE - MY_RANGE) + 1.0);
		}
		else if (DISTANCE < MY_RANGE / RANGE_WINDOW_COEF(strategy))
		{
			bool bFound = false;
			for (auto & wizard : strategy.m_world->getWizards())
			{
				if (wizard.getFaction() == strategy.m_self->getFaction())
					continue;
				if (wizard.getId() == unit.getId())
					continue;
				if (strategy.m_self->getDistanceTo(wizard) < strategy.m_game->getWizardVisionRange())
				{
					bFound = true;
					break;
				}
			}
			for (auto & building : strategy.m_world->getBuildings())
			{
				if (building.getFaction() == strategy.m_self->getFaction())
					continue;
				if (strategy.m_self->getDistanceTo(building) < strategy.m_game->getWizardVisionRange())
				{
					int nUnits = 0;
					for (auto & minion : strategy.m_world->getMinions())
					{
						if (minion.getFaction() != strategy.m_self->getFaction() && minion.getFaction() != model::FACTION_NEUTRAL)
							continue;
						if (building.getDistanceTo(minion) <= building.getAttackRange())
							nUnits++;
					}
					if (nUnits < 2)
					{
						bFound = true;
						break;
					}
				}
			}
			if (bFound && PW < 10.0)
				PW = -1750.0;
			else
				PW = 75.0;
		}
		else
		{
			PW = -10.0;
		}
	}
	else
	{
		if (DISTANCE <= std::max(MY_RANGE / RANGE_WINDOW_COEF(strategy), ENEMY_RANGE * (RANGE_WINDOW_COEF(strategy) + 0.1)))
			PW = -3000.0;
		else
			PW = 75.0 / ((DISTANCE - MY_RANGE) * (DISTANCE - MY_RANGE) + 1.0);
	}

	return PW;
};

const std::function<double(MyStrategy&, const model::Building&)> CSettings::PW_ENEMY_BASE = [](MyStrategy & strategy, const model::Building & unit)
{
	bool bImmortal = false;
	if (std::abs(unit.getX() - strategy.m_global.m_BS.first) < 0.1 && std::abs(unit.getY() - strategy.m_global.m_BS.second) < 0.1 && strategy.m_global.m_top != CGlobal::LaneState::BASE && strategy.m_global.m_mid != CGlobal::LaneState::BASE && strategy.m_global.m_bot != CGlobal::LaneState::BASE)
		bImmortal = true;

	double DISTANCE = std::hypot(strategy.m_self->getX() - unit.getX(), strategy.m_self->getY() - unit.getY());

	double ENEMY_RANGE = strategy.m_game->getFactionBaseAttackRange() + strategy.m_game->getWizardForwardSpeed() + 20.0;
	double MY_RANGE = strategy.m_self->getCastRange() + unit.getRadius() - strategy.m_game->getMagicMissileRadius() - 0.1 + strategy.m_global.RangeLevel(*strategy.m_self) * strategy.m_game->getRangeBonusPerSkillLevel();

	double PW = 0.0;

	for (auto & minion : strategy.m_world->getMinions())
	{
		if (minion.getFaction() != strategy.m_self->getFaction() && minion.getFaction() != model::FACTION_NEUTRAL)
			continue;
		double MINION_DISTANCE = minion.getDistanceTo(unit);
		if (MINION_DISTANCE <= strategy.m_game->getGuardianTowerAttackRange())
			PW += 1.0;
	}

	for (auto & wizard : strategy.m_world->getWizards())
	{
		if (wizard.getFaction() != strategy.m_self->getFaction())
			continue;
		if (wizard.isMe())
			continue;
		double WIZARD_DISTANCE = wizard.getDistanceTo(unit);
		if (WIZARD_DISTANCE <= strategy.m_game->getGuardianTowerAttackRange())
			PW += 1.0;
	}

	if (strategy.m_global.m_bIsFinal)
		PW += 4.0;

	if (!bImmortal && PW > 1.0 && !strategy.IsDangerous())
	{
		if (DISTANCE >= MY_RANGE)
		{
			PW = (PW * 120.0) / ((DISTANCE - MY_RANGE) * (DISTANCE - MY_RANGE) + 1.0);
		}
		else if (DISTANCE < MY_RANGE / RANGE_WINDOW_COEF(strategy))
		{
			if (PW < 2.5)
			{
				PW = -1750.0;
			}
			else
			{
				bool bFound = false;
				if (!strategy.m_global.m_bIsFinal)
				{
					for (auto & wizard : strategy.m_world->getWizards())
					{
						if (wizard.getFaction() == strategy.m_self->getFaction())
							continue;
						if (strategy.m_self->getDistanceTo(wizard) < strategy.m_game->getWizardVisionRange())
						{
							bFound = true;
							break;
						}
					}
				}
				if (bFound)
					PW = -1750.0;
				else
					PW = 100.0;
			}
		}
		else
		{
			PW = -10.0;
		}
	}
	else
	{
		if (DISTANCE <= std::max(MY_RANGE / RANGE_WINDOW_COEF(strategy), ENEMY_RANGE * (RANGE_WINDOW_COEF(strategy) + 0.1)))
			PW = -3000.0;
		else
			PW = bImmortal ? 0.0 : 60.0 / ((DISTANCE - MY_RANGE) * (DISTANCE - MY_RANGE) + 1.0);
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
	if (!(unit.getSpeedX() > 0.0 || unit.getSpeedY() > 0.0 || unit.getLife() < unit.getMaxLife() || unit.getRemainingActionCooldownTicks() > 0))
		return 0.0;

	double DISTANCE = std::hypot(strategy.m_self->getX() - unit.getX(), strategy.m_self->getY() - unit.getY());

	double ENEMY_RANGE = strategy.m_game->getOrcWoodcutterAttackRange() + strategy.m_self->getRadius() + strategy.m_game->getWizardForwardSpeed() + strategy.m_game->getMinionSpeed() + 1.0;
	
	double D = strategy.m_self->getDistanceTo(unit);
	double T = std::min(15.0, D / strategy.m_game->getMagicMissileSpeed());
	double R = std::ceil(T) * strategy.m_game->getMinionSpeed();
	double MY_RANGE = strategy.m_self->getCastRange() + unit.getRadius() - strategy.m_game->getMagicMissileRadius() - R - 0.1 + strategy.m_global.RangeLevel(*strategy.m_self) * strategy.m_game->getRangeBonusPerSkillLevel();

	double PW = 0.0;

	for (auto & minion : strategy.m_world->getMinions())
	{
		if (minion.getFaction() != strategy.m_self->getFaction())
			continue;
		double MINION_DISTANCE = minion.getDistanceTo(unit);
		if (MINION_DISTANCE <= DISTANCE)
			PW += 1.0;
	}

	for (auto & building : strategy.m_world->getBuildings())
	{
		if (building.getFaction() != strategy.m_self->getFaction())
			continue;
		double ENEMY_RANGE_TO_BUILDING = strategy.m_game->getOrcWoodcutterAttackRange() + building.getRadius() + 1.0;
		if (building.getDistanceTo(unit) < ENEMY_RANGE_TO_BUILDING)
			PW += 10.0;
	}

	if (PW > 0.0 && !strategy.IsDangerous())
	{
		if (DISTANCE >= MY_RANGE)
		{
			PW = (PW * 200.0) / ((DISTANCE - MY_RANGE) * (DISTANCE - MY_RANGE) + 1.0);
		}
		else if (DISTANCE < MY_RANGE / RANGE_WINDOW_COEF(strategy))
		{
			if (PW < 0.5)
			{
				PW = -1750.0;
			}
			else
			{
				bool bFound = false;
				for (auto & wizard : strategy.m_world->getWizards())
				{
					if (wizard.getFaction() == strategy.m_self->getFaction())
						continue;
					if (strategy.m_self->getDistanceTo(wizard) < strategy.m_game->getWizardVisionRange())
					{
						bFound = true;
						break;
					}
				}
				if (bFound && PW < 10.0)
					PW = -1750.0;
				else
					PW = 100.0;
			}
		}
		else
		{
			PW = -10.0;
		}
	}
	else
	{
		if (DISTANCE <= std::max(MY_RANGE / RANGE_WINDOW_COEF(strategy), ENEMY_RANGE * (RANGE_WINDOW_COEF(strategy) + 0.1)))
			PW = -3000.0;
		else
			PW = 100.0 / ((DISTANCE - MY_RANGE) * (DISTANCE - MY_RANGE) + 1.0);
	}

	return PW;
};

const std::function<double(MyStrategy&, const model::Minion&)> CSettings::PW_NEUTRAL_CREEP_FETISH = [](MyStrategy & strategy, const model::Minion & unit)
{
	if (!(unit.getSpeedX() > 0.0 || unit.getSpeedY() > 0.0 || unit.getLife() < unit.getMaxLife() || unit.getRemainingActionCooldownTicks() > 0))
		return 0.0;

	double DISTANCE = std::hypot(strategy.m_self->getX() - unit.getX(), strategy.m_self->getY() - unit.getY());

	double ENEMY_RANGE = strategy.m_game->getFetishBlowdartAttackRange() + strategy.m_game->getDartRadius() + strategy.m_self->getRadius() + strategy.m_game->getWizardForwardSpeed() + strategy.m_game->getMinionSpeed() + 1.0;
	
	double D = strategy.m_self->getDistanceTo(unit);
	double T = std::min(15.0, D / strategy.m_game->getMagicMissileSpeed());
	double R = std::ceil(T) * strategy.m_game->getMinionSpeed();
	double MY_RANGE = strategy.m_self->getCastRange() + unit.getRadius() - strategy.m_game->getMagicMissileRadius() - R - 0.1 + strategy.m_global.RangeLevel(*strategy.m_self) * strategy.m_game->getRangeBonusPerSkillLevel();

	double PW = 0.0;

	for (auto & minion : strategy.m_world->getMinions())
	{
		if (minion.getFaction() != strategy.m_self->getFaction())
			continue;
		double MINION_DISTANCE = minion.getDistanceTo(unit);
		if (MINION_DISTANCE <= DISTANCE)
			PW += 1.0;
	}

	for (auto & building : strategy.m_world->getBuildings())
	{
		if (building.getFaction() != strategy.m_self->getFaction())
			continue;
		double ENEMY_RANGE_TO_BUILDING = strategy.m_game->getFetishBlowdartAttackRange() + strategy.m_game->getDartRadius() + building.getRadius() + 1.0;
		if (building.getDistanceTo(unit) < ENEMY_RANGE_TO_BUILDING)
			PW += 10.0;
	}

	if (PW > 0.0 && !strategy.IsDangerous())
	{
		if (DISTANCE >= MY_RANGE)
		{
			PW = (PW * 200.0) / ((DISTANCE - MY_RANGE) * (DISTANCE - MY_RANGE) + 1.0);
		}
		else if (DISTANCE < MY_RANGE / RANGE_WINDOW_COEF(strategy))
		{
			if (PW < 0.5)
			{
				PW = -1750.0;
			}
			else
			{
				bool bFound = false;
				for (auto & wizard : strategy.m_world->getWizards())
				{
					if (wizard.getFaction() == strategy.m_self->getFaction())
						continue;
					if (strategy.m_self->getDistanceTo(wizard) < strategy.m_game->getWizardVisionRange())
					{
						bFound = true;
						break;
					}
				}
				if (bFound && PW < 10.0)
					PW = -1750.0;
				else
					PW = 100.0;
			}
		}
		else
		{
			PW = -10.0;
		}
	}
	else
	{
		if (DISTANCE <= std::max(MY_RANGE / RANGE_WINDOW_COEF(strategy), ENEMY_RANGE * (RANGE_WINDOW_COEF(strategy) + 0.1)))
			PW = -3000.0;
		else
			PW = 100.0 / ((DISTANCE - MY_RANGE) * (DISTANCE - MY_RANGE) + 1.0);
	}

	return PW;
};


const std::function<double(MyStrategy&, const model::CircularUnit&)> CSettings::PW_CIRCULAR_UNIT = [](MyStrategy & strategy, const model::CircularUnit & unit)
{
	double D = std::hypot(strategy.m_self->getX() - unit.getX(), strategy.m_self->getY() - unit.getY()) - strategy.m_self->getRadius() - unit.getRadius();
	if (D > 10.0)
		return 0.0;
	if (D < 3.0)
		return -10000.0;
	return -((10000.0 / ((D - 2.0) * (D - 2.0))) - 155.0);
};


const std::function<double(MyStrategy&, const model::Projectile&)> CSettings::PW_PROJECTILE = [](MyStrategy & strategy, const model::Projectile & unit)
{
	double D = std::hypot(strategy.m_self->getX() - unit.getX(), strategy.m_self->getY() - unit.getY()) - strategy.m_self->getRadius() - unit.getRadius();
	if (D > 50.0)
		return 0.0;
	if (D < 10.0)
		return -10000.0;
	return -((10000.0 / ((D - 9.0) * (D - 9.0))) - 5.8);
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

bool CSettings::HAVE_HASTE(MyStrategy & strategy, const model::Wizard & wizard)
{
	for (auto status : wizard.getStatuses())
	{
		if (status.getType() == model::STATUS_HASTENED)
			return true;
	}
	return false;
}


const std::vector<model::SkillType> CSettings::GET_SKILLS_ORDER(MyStrategy & strategy)
{
	if (strategy.m_global.m_bEgoistMode)
	{
		return {

			model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_1,
			model::SKILL_STAFF_DAMAGE_BONUS_AURA_1,
			model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_2,
			model::SKILL_STAFF_DAMAGE_BONUS_AURA_2,
			model::SKILL_FIREBALL,

			model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_1,
			model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_1,
			model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_2,
			model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_2,
			model::SKILL_FROST_BOLT,

			model::SKILL_RANGE_BONUS_PASSIVE_1,
			model::SKILL_RANGE_BONUS_AURA_1,
			model::SKILL_RANGE_BONUS_PASSIVE_2,
			model::SKILL_RANGE_BONUS_AURA_2,
			model::SKILL_ADVANCED_MAGIC_MISSILE,

			model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_1,
			model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_1,
			model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_2,
			model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_2,
			model::SKILL_HASTE,

			model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_1,
			model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_1,
			model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_2,
			model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_2,
			model::SKILL_SHIELD

		};
	}

	return {

		model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_1,
		model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_1,
		model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_2,
		model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_2,
		model::SKILL_HASTE,

		model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_1,
		model::SKILL_STAFF_DAMAGE_BONUS_AURA_1,
		model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_2,
		model::SKILL_STAFF_DAMAGE_BONUS_AURA_2,
		model::SKILL_FIREBALL,

		model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_1,
		model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_1,
		model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_2,
		model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_2,
		model::SKILL_SHIELD,

		model::SKILL_RANGE_BONUS_PASSIVE_1,
		model::SKILL_RANGE_BONUS_AURA_1,
		model::SKILL_RANGE_BONUS_PASSIVE_2,
		model::SKILL_RANGE_BONUS_AURA_2,
		model::SKILL_ADVANCED_MAGIC_MISSILE,

		model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_1,
		model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_1,
		model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_2,
		model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_2,
		model::SKILL_FROST_BOLT

	};
}

const std::vector<model::SkillType> CSettings::GET_SKILLS_ORDER_FOR_SKILL(MyStrategy & strategy, model::SkillType skill)
{
	if (skill == model::SKILL_SHIELD)
	{
		return {

			model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_1,
			model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_1,
			model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_2,
			model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_2,
			model::SKILL_SHIELD,

			model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_1,
			model::SKILL_STAFF_DAMAGE_BONUS_AURA_1,
			model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_2,
			model::SKILL_STAFF_DAMAGE_BONUS_AURA_2,
			model::SKILL_FIREBALL,

			model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_1,
			model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_1,
			model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_2,
			model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_2,
			model::SKILL_FROST_BOLT,

			model::SKILL_RANGE_BONUS_PASSIVE_1,
			model::SKILL_RANGE_BONUS_AURA_1,
			model::SKILL_RANGE_BONUS_PASSIVE_2,
			model::SKILL_RANGE_BONUS_AURA_2,
			model::SKILL_ADVANCED_MAGIC_MISSILE,

			model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_1,
			model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_1,
			model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_2,
			model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_2,
			model::SKILL_HASTE

		};
	}

	if (skill == model::SKILL_FIREBALL)
	{
		return {

			model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_1,
			model::SKILL_STAFF_DAMAGE_BONUS_AURA_1,
			model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_2,
			model::SKILL_STAFF_DAMAGE_BONUS_AURA_2,
			model::SKILL_FIREBALL,

			model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_1,
			model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_1,
			model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_2,
			model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_2,
			model::SKILL_FROST_BOLT,

			model::SKILL_RANGE_BONUS_PASSIVE_1,
			model::SKILL_RANGE_BONUS_AURA_1,
			model::SKILL_RANGE_BONUS_PASSIVE_2,
			model::SKILL_RANGE_BONUS_AURA_2,
			model::SKILL_ADVANCED_MAGIC_MISSILE,

			model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_1,
			model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_1,
			model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_2,
			model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_2,
			model::SKILL_HASTE,

			model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_1,
			model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_1,
			model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_2,
			model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_2,
			model::SKILL_SHIELD

		};
	}

	if (skill == model::SKILL_FROST_BOLT)
	{
		return {

			model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_1,
			model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_1,
			model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_2,
			model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_2,
			model::SKILL_FROST_BOLT,

			model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_1,
			model::SKILL_STAFF_DAMAGE_BONUS_AURA_1,
			model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_2,
			model::SKILL_STAFF_DAMAGE_BONUS_AURA_2,
			model::SKILL_FIREBALL,

			model::SKILL_RANGE_BONUS_PASSIVE_1,
			model::SKILL_RANGE_BONUS_AURA_1,
			model::SKILL_RANGE_BONUS_PASSIVE_2,
			model::SKILL_RANGE_BONUS_AURA_2,
			model::SKILL_ADVANCED_MAGIC_MISSILE,

			model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_1,
			model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_1,
			model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_2,
			model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_2,
			model::SKILL_HASTE,

			model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_1,
			model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_1,
			model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_2,
			model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_2,
			model::SKILL_SHIELD

		};
	}

	if (skill == model::SKILL_HASTE)
	{
		return {

			model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_1,
			model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_1,
			model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_2,
			model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_2,
			model::SKILL_HASTE,

			model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_1,
			model::SKILL_STAFF_DAMAGE_BONUS_AURA_1,
			model::SKILL_STAFF_DAMAGE_BONUS_PASSIVE_2,
			model::SKILL_STAFF_DAMAGE_BONUS_AURA_2,
			model::SKILL_FIREBALL,

			model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_1,
			model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_1,
			model::SKILL_MAGICAL_DAMAGE_BONUS_PASSIVE_2,
			model::SKILL_MAGICAL_DAMAGE_BONUS_AURA_2,
			model::SKILL_FROST_BOLT,

			model::SKILL_RANGE_BONUS_PASSIVE_1,
			model::SKILL_RANGE_BONUS_AURA_1,
			model::SKILL_RANGE_BONUS_PASSIVE_2,
			model::SKILL_RANGE_BONUS_AURA_2,
			model::SKILL_ADVANCED_MAGIC_MISSILE,

			model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_1,
			model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_1,
			model::SKILL_MAGICAL_DAMAGE_ABSORPTION_PASSIVE_2,
			model::SKILL_MAGICAL_DAMAGE_ABSORPTION_AURA_2,
			model::SKILL_SHIELD

		};
	}

	return GET_SKILLS_ORDER(strategy);
}

double CSettings::RANGE_WINDOW_COEF(MyStrategy & strategy)
{
	double coef = (double)strategy.m_self->getRemainingActionCooldownTicks() / (double)strategy.m_game->getWizardActionCooldownTicks(); // [0, 1] 0 - action, 1 - wait
	double val = 0.9 + (1.0 - coef) * 0.2;
	return val;
}