#include "MyStrategy.h"

#define PI 3.14159265358979323846
#define _USE_MATH_DEFINES

#include <cmath>
#include <cstdlib>
#include <time.h>
#include <limits>

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

	if (!m_bSeedReady)
	{
		volatile unsigned int seed = (unsigned int)time(nullptr) + (unsigned int)m_game->getRandomSeed() + (unsigned int)m_self->getId() + (unsigned int)(m_self->getX() * 100.0) + (unsigned int)(m_self->getY() * 100.0);
		srand(seed);
		m_bSeedReady = true;
	}

	if (m_world->getTickIndex() == 500 || (m_world->getTickIndex() + 1000) % 2500 == 0)
	{
		m_global.ReCheckLane();
	}

	m_LastPositions.push_front(std::make_pair(m_self->getX(), m_self->getY()));
	if (m_LastPositions.size() > 150)
		m_LastPositions.pop_back();
	if (!m_FreeMode && m_world->getTickIndex() >= 150 && m_nLastHealTick < m_world->getTickIndex() - 150 && m_LastShootTick < m_world->getTickIndex() - 150)
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

	if (m_self->getLife() < m_self->getMaxLife() * 0.4)
		m_nLastHealTick = m_world->getTickIndex();

	m_global.SetTowerCords();
	m_global.ChooseLane();
	m_global.Update();
	auto waypoint = m_global.GetWaypoint();

	std::pair<double, double> result = { 0.0, 0.0 };

	if (m_FreeMode/* || m_self->getLife() < m_self->getMaxLife() * 0.4*/)
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
	m_tBuildings.push_back(model::Building(0, m_global.m_BS.first, m_global.m_BS.second, 0.0, 0.0, 0.0, m_self->getFaction() == model::FACTION_ACADEMY ? model::FACTION_RENEGADES : model::FACTION_ACADEMY, m_game->getFactionBaseRadius(), m_game->getFactionBaseLife(), m_game->getFactionBaseLife(), std::vector<model::Status>(), model::BUILDING_FACTION_BASE, m_game->getFactionBaseVisionRange(), m_game->getFactionBaseAttackRange(), 0, 0, 0));
	if (m_global.m_top == CGlobal::LaneState::TOWER_1)
	{
		m_tBuildings.push_back(model::Building(0, m_global.m_T1.first, m_global.m_T1.second, 0.0, 0.0, 0.0, m_self->getFaction() == model::FACTION_ACADEMY ? model::FACTION_RENEGADES : model::FACTION_ACADEMY, m_game->getGuardianTowerRadius(), m_game->getGuardianTowerLife(), m_game->getGuardianTowerLife(), std::vector<model::Status>(), model::BUILDING_GUARDIAN_TOWER, m_game->getGuardianTowerVisionRange(), m_game->getGuardianTowerAttackRange(), 0, 0, 0));
		m_tBuildings.push_back(model::Building(0, m_global.m_T2.first, m_global.m_T2.second, 0.0, 0.0, 0.0, m_self->getFaction() == model::FACTION_ACADEMY ? model::FACTION_RENEGADES : model::FACTION_ACADEMY, m_game->getGuardianTowerRadius(), m_game->getGuardianTowerLife(), m_game->getGuardianTowerLife(), std::vector<model::Status>(), model::BUILDING_GUARDIAN_TOWER, m_game->getGuardianTowerVisionRange(), m_game->getGuardianTowerAttackRange(), 0, 0, 0));
	}
	else if (m_global.m_top == CGlobal::LaneState::TOWER_2)
	{
		m_tBuildings.push_back(model::Building(0, m_global.m_T2.first, m_global.m_T2.second, 0.0, 0.0, 0.0, m_self->getFaction() == model::FACTION_ACADEMY ? model::FACTION_RENEGADES : model::FACTION_ACADEMY, m_game->getGuardianTowerRadius(), m_game->getGuardianTowerLife(), m_game->getGuardianTowerLife(), std::vector<model::Status>(), model::BUILDING_GUARDIAN_TOWER, m_game->getGuardianTowerVisionRange(), m_game->getGuardianTowerAttackRange(), 0, 0, 0));
	}
	if (m_global.m_mid == CGlobal::LaneState::TOWER_1)
	{
		m_tBuildings.push_back(model::Building(0, m_global.m_M1.first, m_global.m_M1.second, 0.0, 0.0, 0.0, m_self->getFaction() == model::FACTION_ACADEMY ? model::FACTION_RENEGADES : model::FACTION_ACADEMY, m_game->getGuardianTowerRadius(), m_game->getGuardianTowerLife(), m_game->getGuardianTowerLife(), std::vector<model::Status>(), model::BUILDING_GUARDIAN_TOWER, m_game->getGuardianTowerVisionRange(), m_game->getGuardianTowerAttackRange(), 0, 0, 0));
		m_tBuildings.push_back(model::Building(0, m_global.m_M2.first, m_global.m_M2.second, 0.0, 0.0, 0.0, m_self->getFaction() == model::FACTION_ACADEMY ? model::FACTION_RENEGADES : model::FACTION_ACADEMY, m_game->getGuardianTowerRadius(), m_game->getGuardianTowerLife(), m_game->getGuardianTowerLife(), std::vector<model::Status>(), model::BUILDING_GUARDIAN_TOWER, m_game->getGuardianTowerVisionRange(), m_game->getGuardianTowerAttackRange(), 0, 0, 0));
	}
	else if (m_global.m_mid == CGlobal::LaneState::TOWER_2)
	{
		m_tBuildings.push_back(model::Building(0, m_global.m_M2.first, m_global.m_M2.second, 0.0, 0.0, 0.0, m_self->getFaction() == model::FACTION_ACADEMY ? model::FACTION_RENEGADES : model::FACTION_ACADEMY, m_game->getGuardianTowerRadius(), m_game->getGuardianTowerLife(), m_game->getGuardianTowerLife(), std::vector<model::Status>(), model::BUILDING_GUARDIAN_TOWER, m_game->getGuardianTowerVisionRange(), m_game->getGuardianTowerAttackRange(), 0, 0, 0));
	}
	if (m_global.m_bot == CGlobal::LaneState::TOWER_1)
	{
		m_tBuildings.push_back(model::Building(0, m_global.m_B1.first, m_global.m_B1.second, 0.0, 0.0, 0.0, m_self->getFaction() == model::FACTION_ACADEMY ? model::FACTION_RENEGADES : model::FACTION_ACADEMY, m_game->getGuardianTowerRadius(), m_game->getGuardianTowerLife(), m_game->getGuardianTowerLife(), std::vector<model::Status>(), model::BUILDING_GUARDIAN_TOWER, m_game->getGuardianTowerVisionRange(), m_game->getGuardianTowerAttackRange(), 0, 0, 0));
		m_tBuildings.push_back(model::Building(0, m_global.m_B2.first, m_global.m_B2.second, 0.0, 0.0, 0.0, m_self->getFaction() == model::FACTION_ACADEMY ? model::FACTION_RENEGADES : model::FACTION_ACADEMY, m_game->getGuardianTowerRadius(), m_game->getGuardianTowerLife(), m_game->getGuardianTowerLife(), std::vector<model::Status>(), model::BUILDING_GUARDIAN_TOWER, m_game->getGuardianTowerVisionRange(), m_game->getGuardianTowerAttackRange(), 0, 0, 0));
	}
	else if (m_global.m_bot == CGlobal::LaneState::TOWER_2)
	{
		m_tBuildings.push_back(model::Building(0, m_global.m_B2.first, m_global.m_B2.second, 0.0, 0.0, 0.0, m_self->getFaction() == model::FACTION_ACADEMY ? model::FACTION_RENEGADES : model::FACTION_ACADEMY, m_game->getGuardianTowerRadius(), m_game->getGuardianTowerLife(), m_game->getGuardianTowerLife(), std::vector<model::Status>(), model::BUILDING_GUARDIAN_TOWER, m_game->getGuardianTowerVisionRange(), m_game->getGuardianTowerAttackRange(), 0, 0, 0));
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
			if (m_self->getLife() < m_self->getMaxLife() * 0.4)
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

		double D = std::hypot(m_self->getX() - unit.getX(), m_self->getY() - unit.getY());
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
			if (m_self->getLife() < m_self->getMaxLife() * 0.4)
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
			{
				if (!waypoint.second && m_self->getDistanceTo(unit) < m_self->getDistanceTo(waypoint.first.first, waypoint.first.second))
					waypoint.first = { unit.getX(), unit.getY() };
				if (unit.getDistanceTo(m_game->getMapSize() - m_global.m_BS.first, m_game->getMapSize() - m_global.m_BS.second) < enMinBase)
				{
					enMinBase = unit.getDistanceTo(m_game->getMapSize() - m_global.m_BS.first, m_game->getMapSize() - m_global.m_BS.second);
					pEnMinUnit = &unit;
				}
				if (m_self->getLife() < m_self->getMaxLife() * 0.4)
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
				if (m_self->getLife() < m_self->getMaxLife() * 0.4)
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
		double D = std::hypot(m_self->getX() - unit.getX(), m_self->getY() - unit.getY());
		if (D > m_self->getVisionRange())
			continue;

		AddPower("collision", result, CalcPower(unit, CSettings::PW_CIRCULAR_UNIT(*this, unit)));

		AddPower("tree", result, CalcPower(unit, CSettings::PW_TREE(*this, unit)));
	}

	for (auto & unit : m_world->getProjectiles()) // test
	{
		double D = std::hypot(m_self->getX() - unit.getX(), m_self->getY() - unit.getY());
		if (D > m_self->getVisionRange())
			continue;

		AddPower("projectile", result, CalcPower(unit, CSettings::PW_PROJECTILE(*this, unit)));
	}

	if (m_global.m_bBonusT && (!m_global.m_bBonusB || m_self->getDistanceTo(1200.0, 1200.0) < m_self->getDistanceTo(2800.0, 2800.0) - 100.0))
	{
		if (m_self->getDistanceTo(1200.0, 1200.0) < m_self->getDistanceTo(m_global.m_BS.first, m_global.m_BS.second) - 250.0 && m_self->getDistanceTo(1200.0, 1200.0) < m_self->getDistanceTo(m_game->getMapSize() - m_global.m_BS.first, m_game->getMapSize() - m_global.m_BS.second) - 250.0)
			waypoint.first = { 1200.0 + (m_world->getTickIndex() % 2500 > 2000 ? m_game->getBonusRadius() + m_self->getRadius() + 50.0 : 0.0), 1200.0 };
	}
	else if (m_global.m_bBonusB)
	{
		if (m_self->getDistanceTo(2800.0, 2800.0) < m_self->getDistanceTo(m_global.m_BS.first, m_global.m_BS.second) - 250.0 && m_self->getDistanceTo(2800.0, 2800.0) < m_self->getDistanceTo(m_game->getMapSize() - m_global.m_BS.first, m_game->getMapSize() - m_global.m_BS.second) - 250.0)
			waypoint.first = { 2800.0 - (m_world->getTickIndex() % 2500 > 2000 ? m_game->getBonusRadius() + m_self->getRadius() + 50.0 : 0.0), 2800.0 };
	}

	if (pEnMinUnit && enMinBase < 750.0 && m_self->getDistanceTo(m_global.m_BS.first, m_global.m_BS.first) > 1000.0)
	{
		waypoint.first = { pEnMinUnit->getX(), pEnMinUnit->getY() };
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
	const model::CircularUnit * target = nullptr;
	double MAX_PRIORITY = 0.0;

	for (auto & unit : m_world->getMinions())
	{
		if (unit.getFaction() == m_self->getFaction())
			continue;
		double D = std::hypot(m_self->getX() - unit.getX(), m_self->getY() - unit.getY());
		if (D > m_self->getCastRange())
			continue;
		if (unit.getFaction() == model::FACTION_NEUTRAL && !m_FreeMode)
			continue;
		if (unit.getFaction() == model::FACTION_NEUTRAL && (D - m_self->getRadius() - unit.getRadius() > 25.0 || (!m_FreeMode && std::abs(m_self->getAngleTo(unit)) > PI / 5.0)))
			continue;
		double P = (unit.getFaction() == model::FACTION_NEUTRAL ? 15.0 : 100.0) * ((unit.getMaxLife() - unit.getLife() + 1.0) / unit.getMaxLife());
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
		double P = 150.0 * ((unit.getMaxLife() - unit.getLife() + 1.0) / unit.getMaxLife());
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
		double P = (unit.getType() == model::BUILDING_FACTION_BASE  ? 200.0 : 50.0) * ((unit.getMaxLife() - unit.getLife() + 1.0) / unit.getMaxLife());
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

	m_LastShootTick = m_world->getTickIndex();

	double D = std::hypot(m_self->getX() - target->getX(), m_self->getY() - target->getY());
	double angle = m_self->getAngleTo(*target);
	m_move->setTurn(angle);
	if (D - target->getRadius() < m_game->getStaffRange())
	{
		if (std::abs(angle) < m_game->getStaffSector() / 2.0)
			m_move->setAction(model::ACTION_STAFF);
	}
	else
	{
		if (std::abs(angle) < m_game->getStaffSector() / 2.0)
		{
			m_move->setAction(model::ACTION_MAGIC_MISSILE);
			m_move->setCastAngle(angle);
			m_move->setMinCastDistance(D - target->getRadius() + m_game->getMagicMissileRadius());
		}
	}
	return true;
}

void MyStrategy::Step(std::pair<double, double> direction, bool shoot)
{
	double angle = m_self->getAngleTo(m_self->getX() + direction.first, m_self->getY() + direction.second);

	/*printf("%d - ", m_global.m_lane);
	if (m_global.m_lane == model::LANE_TOP)
		printf("%d - ", m_global.m_top);
	if (m_global.m_lane == model::LANE_MIDDLE)
		printf("%d - ", m_global.m_mid);
	if (m_global.m_lane == model::LANE_BOTTOM)
		printf("%d - ", m_global.m_bot);
	printf("%f / %f / %f - ", direction.first, direction.second, angle);*/

	if (!shoot)
		m_move->setTurn(angle);
	
	m_move->setSpeed(0.0);
	m_move->setStrafeSpeed(0.0);

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
}