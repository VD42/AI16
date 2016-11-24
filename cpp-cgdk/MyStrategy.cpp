#include "MyStrategy.h"

#define PI 3.14159265358979323846
#define _USE_MATH_DEFINES

#include <cmath>
#include <cstdlib>
#include <time.h>
#include <limits>
#include <algorithm>

#include "MyStrategy_Settings.h"


MyStrategy::MyStrategy() : m_global(*this), m_bSeedReady(false)
{
	
}

void MyStrategy::move(const model::Wizard & self, const model::World & world, const model::Game & game, model::Move & move)
{
	m_self = &self;
	m_world = &world;
	m_game = &game;
	m_move = &move;

	m_tPowers.clear();

	if (!m_bSeedReady)
	{
		volatile unsigned int seed = (unsigned int)time(nullptr) + (unsigned int)m_game->getRandomSeed() + (unsigned int)m_self->getId() + (unsigned int)(m_self->getX() * 100.0) + (unsigned int)(m_self->getY() * 100.0);
		srand(seed);
		m_bSeedReady = true;
	}

	if (m_nLastReceivedTickIndex + 1 != m_world->getTickIndex())
	{
		m_global.ReCheckLane();
		m_global.m_bBonusB = true;
		m_global.m_bBonusT = true;
	}

	m_nLastReceivedTickIndex = m_world->getTickIndex();

	if (!m_bHealMode && m_self->getLife() < m_self->getMaxLife() * 0.4)
	{
		m_bHealMode = true;
		if (m_nLastHealTick + 100 > m_world->getTickIndex())
			m_bDoubleHealMode = true;
	}
	//if (m_self->getLife() < m_self->getMaxLife() * 0.15)
	//	m_bVeryHealMode = true;
	//if (m_self->getLife() >= m_self->getMaxLife() * 0.2)
	//	m_bVeryHealMode = false;
	if (m_self->getLife() >= m_self->getMaxLife() * (m_bDoubleHealMode ? 0.8 : 0.6))
	{
		m_bHealMode = false;
		m_bDoubleHealMode = false;
	}

	m_LastPositions.push_front(std::make_pair(m_self->getX(), m_self->getY()));
	if (m_LastPositions.size() > 150)
		m_LastPositions.pop_back();
	if (!m_FreeMode && !m_bCreepStop && m_world->getTickIndex() >= 150 && m_nLastHealTick < m_world->getTickIndex() - 150 && m_LastShootTick < m_world->getTickIndex() - 150)
	{
		bool good = false;
		for (auto & val : m_LastPositions)
		{
			if (std::hypot(val.first - m_LastPositions.front().first, val.second - m_LastPositions.front().second) > 50.0)
			{
				good = true;
				break;
			}
		}
		if (!good)
		{
			printf("FATAL! Run free mode! Tick: %d\r\n", m_world->getTickIndex());
			m_FreeModeTick = m_world->getTickIndex();
			m_FreeMode = true;
		}
	}
	if (m_FreeMode)
	{
		if (m_world->getTickIndex() > m_FreeModeTick + 150)
		{
			printf("Disabling free mode. Tick: %d\r\n", m_world->getTickIndex());
			m_FreeMode = false;
		}
	}

	if (m_bHealMode)
		m_nLastHealTick = m_world->getTickIndex();

	m_global.SetTowerCords();
	m_global.ChooseLane();
	m_global.Update();
	auto waypoint = m_global.GetWaypoint();

	if (m_world->getTickIndex() > 3000 && (m_world->getTickIndex() + 1000) % 2500 == 0)
	{
		m_global.ReCheckLane();
	}

	std::pair<double, double> result = { 0.0, 0.0 };

	if (m_FreeMode)
		AddPower("back", result, CalcPower(0.0, m_game->getMapSize(), 2000.0)); // back

	if (m_self->getX() - m_self->getRadius() < 3.0)
	{
		AddPower("brake", result, CalcPower(m_self->getX() + 1.0, m_self->getY() + 0.0, 10000.0));
	}
	else if (m_self->getX() - m_self->getRadius() <= 20.0)
	{
		AddPower("brake", result, CalcPower(m_self->getX() + 1.0, m_self->getY() + 0.0, (10000.0 / ((m_self->getX() - m_self->getRadius() - 2.0) * (m_self->getX() - m_self->getRadius() - 2.0))) - 30.8));
	}

	if (m_self->getX() + m_self->getRadius() > m_game->getMapSize() - 3.0)
	{
		AddPower("brake", result, CalcPower(m_self->getX() - 1.0, m_self->getY() + 0.0, 10000.0));
	}
	else if (m_self->getX() + m_self->getRadius() >= m_game->getMapSize() - 20.0)
	{
		AddPower("brake", result, CalcPower(m_self->getX() - 1.0, m_self->getY() + 0.0, (10000.0 / ((m_game->getMapSize() - m_self->getX() - m_self->getRadius() - 2.0) * (m_game->getMapSize() - m_self->getX() - m_self->getRadius() - 2.0))) - 30.8));
	}

	if (m_self->getY() - m_self->getRadius() < 3.0)
	{
		AddPower("brake", result, CalcPower(m_self->getX(), m_self->getY() + 1.0, 10000.0));
	}
	else if (m_self->getY() - m_self->getRadius() <= 20.0)
	{
		AddPower("brake", result, CalcPower(m_self->getX() + 0.0, m_self->getY() + 1.0, (10000.0 / ((m_self->getY() - m_self->getRadius() - 2.0) * (m_self->getY() - m_self->getRadius() - 2.0))) - 30.8));
	}

	if (m_self->getY() + m_self->getRadius() > m_game->getMapSize() - 3.0)
	{
		AddPower("brake", result, CalcPower(m_self->getX() + 0.0, m_self->getY() - 1.0, 10000.0));
	}
	else if (m_self->getY() + m_self->getRadius() >= m_game->getMapSize() - 20.0)
	{
		AddPower("brake", result, CalcPower(m_self->getX() + 0.0, m_self->getY() - 1.0, (10000.0 / ((m_game->getMapSize() - m_self->getY() - m_self->getRadius() - 2.0) * (m_game->getMapSize() - m_self->getY() - m_self->getRadius() - 2.0))) - 30.8));
	}

	double enMinBase = 40000.0;
	const model::CircularUnit * pEnMinUnit = nullptr;

	std::vector<model::Building> m_tBuildings = m_world->getBuildings();
	m_tBuildings.push_back(model::Building(0, m_global.m_BS.first, m_global.m_BS.second, 0.0, 0.0, 0.0, m_self->getFaction() == model::FACTION_ACADEMY ? model::FACTION_RENEGADES : model::FACTION_ACADEMY, m_game->getFactionBaseRadius(), m_game->getFactionBaseLife(), m_game->getFactionBaseLife(), std::vector<model::Status>(), model::BUILDING_FACTION_BASE, m_game->getFactionBaseVisionRange(), m_game->getFactionBaseAttackRange(), 0, 0, m_game->getFactionBaseCooldownTicks()));
	if (m_global.m_top == CGlobal::LaneState::TOWER_1)
	{
		m_tBuildings.push_back(model::Building(0, m_global.m_T1.first, m_global.m_T1.second, 0.0, 0.0, 0.0, m_self->getFaction() == model::FACTION_ACADEMY ? model::FACTION_RENEGADES : model::FACTION_ACADEMY, m_game->getGuardianTowerRadius(), m_game->getGuardianTowerLife(), m_game->getGuardianTowerLife(), std::vector<model::Status>(), model::BUILDING_GUARDIAN_TOWER, m_game->getGuardianTowerVisionRange(), m_game->getGuardianTowerAttackRange(), 0, 0, m_game->getGuardianTowerCooldownTicks()));
		m_tBuildings.push_back(model::Building(0, m_global.m_T2.first, m_global.m_T2.second, 0.0, 0.0, 0.0, m_self->getFaction() == model::FACTION_ACADEMY ? model::FACTION_RENEGADES : model::FACTION_ACADEMY, m_game->getGuardianTowerRadius(), m_game->getGuardianTowerLife(), m_game->getGuardianTowerLife(), std::vector<model::Status>(), model::BUILDING_GUARDIAN_TOWER, m_game->getGuardianTowerVisionRange(), m_game->getGuardianTowerAttackRange(), 0, 0, m_game->getGuardianTowerCooldownTicks()));
	}
	else if (m_global.m_top == CGlobal::LaneState::TOWER_2)
	{
		m_tBuildings.push_back(model::Building(0, m_global.m_T2.first, m_global.m_T2.second, 0.0, 0.0, 0.0, m_self->getFaction() == model::FACTION_ACADEMY ? model::FACTION_RENEGADES : model::FACTION_ACADEMY, m_game->getGuardianTowerRadius(), m_game->getGuardianTowerLife(), m_game->getGuardianTowerLife(), std::vector<model::Status>(), model::BUILDING_GUARDIAN_TOWER, m_game->getGuardianTowerVisionRange(), m_game->getGuardianTowerAttackRange(), 0, 0, m_game->getGuardianTowerCooldownTicks()));
	}
	if (m_global.m_mid == CGlobal::LaneState::TOWER_1)
	{
		m_tBuildings.push_back(model::Building(0, m_global.m_M1.first, m_global.m_M1.second, 0.0, 0.0, 0.0, m_self->getFaction() == model::FACTION_ACADEMY ? model::FACTION_RENEGADES : model::FACTION_ACADEMY, m_game->getGuardianTowerRadius(), m_game->getGuardianTowerLife(), m_game->getGuardianTowerLife(), std::vector<model::Status>(), model::BUILDING_GUARDIAN_TOWER, m_game->getGuardianTowerVisionRange(), m_game->getGuardianTowerAttackRange(), 0, 0, m_game->getGuardianTowerCooldownTicks()));
		m_tBuildings.push_back(model::Building(0, m_global.m_M2.first, m_global.m_M2.second, 0.0, 0.0, 0.0, m_self->getFaction() == model::FACTION_ACADEMY ? model::FACTION_RENEGADES : model::FACTION_ACADEMY, m_game->getGuardianTowerRadius(), m_game->getGuardianTowerLife(), m_game->getGuardianTowerLife(), std::vector<model::Status>(), model::BUILDING_GUARDIAN_TOWER, m_game->getGuardianTowerVisionRange(), m_game->getGuardianTowerAttackRange(), 0, 0, m_game->getGuardianTowerCooldownTicks()));
	}
	else if (m_global.m_mid == CGlobal::LaneState::TOWER_2)
	{
		m_tBuildings.push_back(model::Building(0, m_global.m_M2.first, m_global.m_M2.second, 0.0, 0.0, 0.0, m_self->getFaction() == model::FACTION_ACADEMY ? model::FACTION_RENEGADES : model::FACTION_ACADEMY, m_game->getGuardianTowerRadius(), m_game->getGuardianTowerLife(), m_game->getGuardianTowerLife(), std::vector<model::Status>(), model::BUILDING_GUARDIAN_TOWER, m_game->getGuardianTowerVisionRange(), m_game->getGuardianTowerAttackRange(), 0, 0, m_game->getGuardianTowerCooldownTicks()));
	}
	if (m_global.m_bot == CGlobal::LaneState::TOWER_1)
	{
		m_tBuildings.push_back(model::Building(0, m_global.m_B1.first, m_global.m_B1.second, 0.0, 0.0, 0.0, m_self->getFaction() == model::FACTION_ACADEMY ? model::FACTION_RENEGADES : model::FACTION_ACADEMY, m_game->getGuardianTowerRadius(), m_game->getGuardianTowerLife(), m_game->getGuardianTowerLife(), std::vector<model::Status>(), model::BUILDING_GUARDIAN_TOWER, m_game->getGuardianTowerVisionRange(), m_game->getGuardianTowerAttackRange(), 0, 0, m_game->getGuardianTowerCooldownTicks()));
		m_tBuildings.push_back(model::Building(0, m_global.m_B2.first, m_global.m_B2.second, 0.0, 0.0, 0.0, m_self->getFaction() == model::FACTION_ACADEMY ? model::FACTION_RENEGADES : model::FACTION_ACADEMY, m_game->getGuardianTowerRadius(), m_game->getGuardianTowerLife(), m_game->getGuardianTowerLife(), std::vector<model::Status>(), model::BUILDING_GUARDIAN_TOWER, m_game->getGuardianTowerVisionRange(), m_game->getGuardianTowerAttackRange(), 0, 0, m_game->getGuardianTowerCooldownTicks()));
	}
	else if (m_global.m_bot == CGlobal::LaneState::TOWER_2)
	{
		m_tBuildings.push_back(model::Building(0, m_global.m_B2.first, m_global.m_B2.second, 0.0, 0.0, 0.0, m_self->getFaction() == model::FACTION_ACADEMY ? model::FACTION_RENEGADES : model::FACTION_ACADEMY, m_game->getGuardianTowerRadius(), m_game->getGuardianTowerLife(), m_game->getGuardianTowerLife(), std::vector<model::Status>(), model::BUILDING_GUARDIAN_TOWER, m_game->getGuardianTowerVisionRange(), m_game->getGuardianTowerAttackRange(), 0, 0, m_game->getGuardianTowerCooldownTicks()));
	}

	for (auto & unit : m_tBuildings)
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
			if (m_bHealMode)
			{
				AddPower("heal", result, CalcPower(unit.getX(), unit.getY(), m_self->getDistanceTo(unit) < 900.0 ? -2000.0 : 0.0));
			}
			else
			{
				if (unit.getType() == model::BUILDING_FACTION_BASE)
					AddPower("enemy base", result, CalcPower(unit, CSettings::PW_ENEMY_BASE(*this, unit)));
				else if (unit.getType() == model::BUILDING_GUARDIAN_TOWER)
					AddPower("enemy tower", result, CalcPower(unit, CSettings::PW_ENEMY_TOWER(*this, unit)));
			}
		}
	}

	for (auto & unit : m_world->getWizards())
	{
		if (unit.isMe())
			continue;

		double D = m_self->getDistanceTo(unit);
		if (D > m_self->getVisionRange())
			continue;

		AddPower("collision", result, CalcPower(unit, CSettings::PW_CIRCULAR_UNIT(*this, unit)));

		if (unit.getFaction() == m_self->getFaction())
		{
			AddPower("friendly wizard", result, CalcPower(unit, CSettings::PW_FRIENDLY_WIZARD(*this, unit)));
		}
		else
		{
			if (!waypoint.second && m_self->getDistanceTo(unit) < m_self->getDistanceTo(waypoint.first.first, waypoint.first.second))
				waypoint.first = { unit.getX(), unit.getY() };
			if (unit.getDistanceTo(m_game->getMapSize() - m_global.m_BS.first, m_game->getMapSize() - m_global.m_BS.second) < enMinBase)
			{
				enMinBase = unit.getDistanceTo(m_game->getMapSize() - m_global.m_BS.first, m_game->getMapSize() - m_global.m_BS.second);
				pEnMinUnit = &unit;
			}
			if (m_bHealMode)
			{
				AddPower("heal", result, CalcPower(unit.getX(), unit.getY(), m_self->getDistanceTo(unit) < 600.0 ? -2000.0 : 0.0));
			}
			else
			{
				AddPower("enemy wizard", result, CalcPower(unit, CSettings::PW_ENEMY_WIZARD(*this, unit)));
			}
		}
	}

	for (auto & unit : m_world->getMinions())
	{
		double D = m_self->getDistanceTo(unit);
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
			{
				if (!waypoint.second && m_self->getDistanceTo(unit) < m_self->getDistanceTo(waypoint.first.first, waypoint.first.second))
					waypoint.first = { unit.getX(), unit.getY() };
				if (unit.getDistanceTo(m_game->getMapSize() - m_global.m_BS.first, m_game->getMapSize() - m_global.m_BS.second) < enMinBase)
				{
					enMinBase = unit.getDistanceTo(m_game->getMapSize() - m_global.m_BS.first, m_game->getMapSize() - m_global.m_BS.second);
					pEnMinUnit = &unit;
				}
				if (m_bHealMode)
				{
					AddPower("heal", result, CalcPower(unit.getX(), unit.getY(), m_self->getDistanceTo(unit) < 500.0 ? -2000.0 : 0.0));
				}
				else
				{
					AddPower("enemy creep", result, CalcPower(unit, CSettings::PW_ENEMY_CREEP_ORC(*this, unit)));
				}
			}
		}
		else if (unit.getType() == model::MINION_FETISH_BLOWDART)
		{
			if (unit.getFaction() == m_self->getFaction())
				AddPower("friendly creep", result, CalcPower(unit, CSettings::PW_FRIENDLY_CREEP_FETISH(*this, unit)));
			else if (unit.getFaction() == model::FACTION_NEUTRAL)
				AddPower("neutral creep", result, CalcPower(unit, CSettings::PW_NEUTRAL_CREEP_FETISH(*this, unit)));
			else
			{
				if (!waypoint.second && m_self->getDistanceTo(unit) < m_self->getDistanceTo(waypoint.first.first, waypoint.first.second))
					waypoint.first = { unit.getX(), unit.getY() };
				if (unit.getDistanceTo(m_game->getMapSize() - m_global.m_BS.first, m_game->getMapSize() - m_global.m_BS.second) < enMinBase)
				{
					enMinBase = unit.getDistanceTo(m_game->getMapSize() - m_global.m_BS.first, m_game->getMapSize() - m_global.m_BS.second);
					pEnMinUnit = &unit;
				}
				if (m_bHealMode)
				{
					AddPower("heal", result, CalcPower(unit.getX(), unit.getY(), m_self->getDistanceTo(unit) < 500.0 ? -2000.0 : 0.0));
				}
				else
				{
					AddPower("enemy creep", result, CalcPower(unit, CSettings::PW_ENEMY_CREEP_FETISH(*this, unit)));
				}
			}
		}
	}

	for (auto & unit : m_world->getTrees())
	{
		double D = m_self->getDistanceTo(unit);
		if (D > m_self->getVisionRange())
			continue;

		AddPower("collision", result, CalcPower(unit, CSettings::PW_CIRCULAR_UNIT(*this, unit)));

		AddPower("tree", result, CalcPower(unit, CSettings::PW_TREE(*this, unit)));
	}

	m_bNoShoot = false;

	for (auto & unit : m_world->getProjectiles())
	{
		double D = m_self->getDistanceTo(unit);
		if (D > m_self->getVisionRange())
			continue;

		if (unit.getType() != model::PROJECTILE_MAGIC_MISSILE)
			continue;

		double X = unit.getX();
		double Y = unit.getY();

		for (int i = 0; i <= 13; i++)
		{
			double tD = m_self->getDistanceTo(X, Y);
			if (tD < m_self->getRadius() + unit.getRadius() + 1.0)
			{
				m_bNoShoot = true;
				AddPower("missile", result, CalcPower(X, Y, -100000.0));
			}
			X += unit.getSpeedX();
			Y += unit.getSpeedY();
		}
	}

	for (auto & unit : m_world->getBonuses())
	{
		if (m_self->getDistanceTo(unit) < 500.0)
			AddPower("bonus", result, CalcPower(unit, 100.0));
	}

	if (m_global.m_bBonusT && (!m_global.m_bBonusB || m_self->getDistanceTo(1200.0, 1200.0) < m_self->getDistanceTo(2800.0, 2800.0)))
	{
		if (m_self->getDistanceTo(1200.0, 1200.0) <= 1800.0)
			waypoint.first = { 1200.0 + (m_world->getTickIndex() % 2500 > 2000 ? m_game->getBonusRadius() + m_self->getRadius() + 5.0 : 0.0), 1200.0 };
	}
	else if (m_global.m_bBonusB)
	{
		if (m_self->getDistanceTo(2800.0, 2800.0) <= 1800.0)
			waypoint.first = { 2800.0 - (m_world->getTickIndex() % 2500 > 2000 ? m_game->getBonusRadius() + m_self->getRadius() + 5.0 : 0.0), 2800.0 };
	}

	if (pEnMinUnit && enMinBase < 750.0 && m_self->getDistanceTo(m_game->getMapSize() - m_global.m_BS.first, m_game->getMapSize() - m_global.m_BS.second) < 1000.0)
	{
		waypoint.first = { pEnMinUnit->getX(), pEnMinUnit->getY() };
	}

	if (m_world->getTickIndex() == 300)
	{
		m_bCreepStop = true;
		m_CreepStopLane = m_global.m_lane;
	}

	if (m_bCreepStop)
	{
		if (m_global.m_lane != m_CreepStopLane || m_world->getTickIndex() >= 1500)
		{
			m_bCreepStop = false;
		}
		else
		{
			if (m_CreepStopLane == model::LANE_TOP)
			{
				if (m_world->getTickIndex() <= 750)
				{
					waypoint.first.first = 200.0;
					waypoint.first.second = m_game->getMapSize() - 1200.0;
				}
				else
				{
					double minX = 0.0;
					double minY = 4000.0;
					for (auto & minion : m_world->getMinions())
					{
						if (minion.getFaction() != m_self->getFaction())
							continue;
						if (m_global.GetLane(minion) != m_CreepStopLane)
							continue;
						if (minion.getY() < minY)
						{
							minX = minion.getX();
							minY = minion.getY();
						}
					}
					waypoint.first.first = minX;
					waypoint.first.second = minY - m_game->getMinionRadius() - m_game->getWizardRadius();
				}
			}
			else if (m_CreepStopLane == model::LANE_MIDDLE)
			{
				if (m_world->getTickIndex() <= 750)
				{
					waypoint.first.first = 1000.0;
					waypoint.first.second = m_game->getMapSize() - 1000.0;
				}
				else
				{
					double maxD = 0.0;
					double maxX = 0.0;
					double maxY = 0.0;
					for (auto & minion : m_world->getMinions())
					{
						if (minion.getFaction() != m_self->getFaction())
							continue;
						if (m_global.GetLane(minion) != m_CreepStopLane)
							continue;
						double D = minion.getX() * (1.0 / std::sqrt(2.0)) + (m_game->getMapSize() - minion.getY()) * (1.0 / std::sqrt(2.0));
						if (D > maxD)
						{
							maxX = minion.getX();
							maxY = minion.getY();
							maxD = D;
						}
					}
					waypoint.first.first = maxX + (m_game->getMinionRadius() + m_game->getWizardRadius()) * (1.0 / std::sqrt(2.0));
					waypoint.first.second = maxY - (m_game->getMinionRadius() + m_game->getWizardRadius()) * (1.0 / std::sqrt(2.0));
				}
			}
			else if (m_CreepStopLane == model::LANE_BOTTOM)
			{
				if (m_world->getTickIndex() <= 750)
				{
					waypoint.first.first = 1200.0;
					waypoint.first.second = m_game->getMapSize() - 200.0;
				}
				else
				{
					double maxX = 0.0;
					double maxY = 0.0;
					for (auto & minion : m_world->getMinions())
					{
						if (minion.getFaction() != m_self->getFaction())
							continue;
						if (m_global.GetLane(minion) != m_CreepStopLane)
							continue;
						if (minion.getX() > maxX)
						{
							maxX = minion.getX();
							maxY = minion.getY();
						}
					}
					waypoint.first.first = maxX + m_game->getMinionRadius() + m_game->getWizardRadius();
					waypoint.first.second = maxY;
				}
			}
		}
	}

	AddPower("base", result, CalcPower(waypoint.first.first, waypoint.first.second, 150.0));

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
	if (m_bNoShoot)
		return false;

	const model::CircularUnit * target = nullptr;
	double MAX_PRIORITY = 0.0;

	for (auto & unit : m_world->getMinions())
	{
		if (unit.getFaction() == m_self->getFaction())
			continue;
		double D = m_self->getDistanceTo(unit);
		double T = std::min(13.0, D / m_game->getMagicMissileSpeed());
		double R = std::ceil(T) * m_game->getMinionSpeed();
		if (D > m_self->getCastRange() + unit.getRadius() - m_game->getMagicMissileRadius() - R - 0.1)
			continue;
		if (unit.getFaction() == model::FACTION_NEUTRAL && !m_FreeMode && !(unit.getSpeedX() > 0.0 || unit.getSpeedY() > 0.0 || unit.getRemainingActionCooldownTicks() > 0))
			continue;
		double P = (unit.getFaction() == model::FACTION_NEUTRAL ? 15.0 : 100.0) * ((unit.getMaxLife() - unit.getLife() + 1.0) / unit.getMaxLife());
		if (unit.getLife() <= 12)
			P = 100000.0;
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
		double D = m_self->getDistanceTo(unit);
		double T = std::min(13.0, D / m_game->getMagicMissileSpeed());
		double R = std::ceil(T) * m_game->getWizardForwardSpeed();
		if (D > m_self->getCastRange() + unit.getRadius() - m_game->getMagicMissileRadius() - R - 0.1)
			continue;
		double P = 1000.0 * ((unit.getMaxLife() - unit.getLife() + 1.0) / unit.getMaxLife());
		if (unit.getLife() <= 12)
			P = 100000.0;
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
		if (m_self->getDistanceTo(unit) > m_self->getCastRange() + unit.getRadius() - m_game->getMagicMissileRadius() - 0.1)
			continue;
		double P = (unit.getType() == model::BUILDING_FACTION_BASE ? 20000.0 : 10000.0);
		if (P > MAX_PRIORITY)
		{
			target = &unit;
			MAX_PRIORITY = P;
		}
	}

	for (auto & unit : m_world->getTrees())
	{
		double D = m_self->getDistanceTo(unit);
		if (D > m_self->getCastRange() + unit.getRadius() - m_game->getMagicMissileRadius() - 0.1)
			continue;
		if (D - m_self->getRadius() - unit.getRadius() > 25.0 || (!m_FreeMode && std::abs(m_self->getAngleTo(unit)) > PI / 5.0))
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

	if (m_bVeryHealMode)
	{
		BestShoot(*target, false);
		return false;
	}

	BestShoot(*target, true);
	return true;
}

void MyStrategy::Step(std::pair<double, double> direction, bool shoot)
{
	double angle = m_self->getAngleTo(m_self->getX() + direction.first, m_self->getY() + direction.second);

	if (!shoot)
	{
		m_move->setTurn(angle);

		const model::Tree * tree = nullptr;
		double minD = 40000.0;

		for (auto & unit : m_world->getTrees())
		{
			double D = m_self->getDistanceTo(unit);
			if (D > m_self->getCastRange() + unit.getRadius() - m_game->getMagicMissileRadius() - 0.1)
				continue;
			if (std::abs(m_self->getAngleTo(unit)) > m_game->getStaffSector() / 2.0)
				continue;
			if (D < minD)
			{
				minD = D;
				tree = &unit;
			}
		}

		if (tree)
		{
			BestShoot(*tree, false);
		}
	}

	m_move->setSpeed(std::cos(angle) * 40.0);
	m_move->setStrafeSpeed(std::sin(angle) * 30.0);

	/*
	if (-PI / 4.0 <= angle && angle <= PI / 4.0)
	{
		m_move->setSpeed(m_game->getWizardForwardSpeed() * 10.0);
	}
	else if (PI / 4.0 < angle && angle <= 3.0 * PI / 4.0)
	{
		m_move->setStrafeSpeed(m_game->getWizardStrafeSpeed() * 10.0);
	}
	else if (angle > 3.0 * PI / 4.0)
	{
		m_move->setSpeed(-m_game->getWizardBackwardSpeed() * 10.0);
	}
	else if (-PI / 4.0 >= angle && angle > -3.0 * PI / 4.0)
	{
		m_move->setStrafeSpeed(-m_game->getWizardStrafeSpeed() * 10.0);
	}
	else if (angle < -3.0 * PI / 4.0)
	{
		m_move->setSpeed(-m_game->getWizardBackwardSpeed() * 10.0);
	}
	*/
}

void MyStrategy::BestShoot(const model::CircularUnit & unit, bool turn)
{
	double D = m_self->getDistanceTo(unit);
	double angle = m_self->getAngleTo(unit);
	if (turn)
	{
		m_LastShootTick = m_world->getTickIndex();
		m_move->setTurn(angle);
	}
	if (D - unit.getRadius() < m_game->getStaffRange() && m_self->getRemainingActionCooldownTicks() == 0 && m_self->getRemainingCooldownTicksByAction()[1] == 0)
	{
		if (std::abs(angle) < m_game->getStaffSector() / 2.0)
			m_move->setAction(model::ACTION_STAFF);
	}
	else if (m_self->getRemainingActionCooldownTicks() == 0 && m_self->getRemainingCooldownTicksByAction()[2] == 0)
	{
		double T = std::min(13.0, D / m_game->getMagicMissileSpeed());
		double newX = unit.getX() + unit.getSpeedX() * T;
		double newY = unit.getY() + unit.getSpeedY() * T;
		double newD = std::hypot(m_self->getX() - newX, m_self->getY() - newY);
		double newAngle = m_self->getAngleTo(newX, newY);
		if (turn)
			m_move->setTurn(newAngle);
		if (std::abs(newAngle) < m_game->getStaffSector() / 2.0)
		{
			m_move->setAction(model::ACTION_MAGIC_MISSILE);
			m_move->setCastAngle(newAngle);
			m_move->setMinCastDistance(newD - unit.getRadius() + m_game->getMagicMissileRadius());
		}
	}
}