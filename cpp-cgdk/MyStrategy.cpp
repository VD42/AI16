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
	m_tSkillsOrder = CSettings::GET_SKILLS_ORDER();
	std::reverse(m_tSkillsOrder.begin(), m_tSkillsOrder.end());
}

void MyStrategy::move(const model::Wizard & self, const model::World & world, const model::Game & game, model::Move & move)
{
	m_self = &self;
	m_world = &world;
	m_game = &game;
	m_move = &move;

	m_move->setAction(model::ACTION_NONE);

	m_setCurrentSkills.clear();
	m_setCurrentSkills.insert(m_self->getSkills().begin(), m_self->getSkills().end());
	
	if (m_nLastLevel < m_self->getLevel())
	{
		m_move->setSkillToLearn(m_tSkillsOrder.back());
		printf("Level up, learning: %d\r\n", m_move->getSkillToLearn());
		m_setCurrentSkills.insert(m_move->getSkillToLearn());
		m_tSkillsOrder.pop_back();
		m_nLastLevel++;
	}

	if (m_move->getAction() == model::ACTION_NONE && m_setCurrentSkills.find(model::SKILL_SHIELD) != m_setCurrentSkills.end() && m_self->getRemainingActionCooldownTicks() == 0 && m_self->getRemainingCooldownTicksByAction()[6] == 0 && m_self->getMana() >= m_game->getShieldManacost())
	{
		if (!CSettings::HAVE_SHIELD(*this, *m_self))
		{
			m_move->setStatusTargetId(m_self->getId());
			m_move->setAction(model::ACTION_SHIELD);
		}
		else
		{
			for (auto & unit : m_world->getWizards())
			{
				if (unit.isMe())
					continue;
				if (unit.getFaction() != m_self->getFaction())
					continue;
				if (m_self->getDistanceTo(unit) > m_self->getCastRange())
					continue;
				if (std::abs(m_self->getAngleTo(unit)) > m_game->getStaffSector() / 2.0)
					continue;
				if (CSettings::HAVE_SHIELD(*this, unit))
					continue;
				m_move->setStatusTargetId(unit.getId());
				m_move->setAction(model::ACTION_SHIELD);
				break;
			}
		}
	}

	if (m_move->getAction() == model::ACTION_NONE && m_setCurrentSkills.find(model::SKILL_HASTE) != m_setCurrentSkills.end() && m_self->getRemainingActionCooldownTicks() == 0 && m_self->getRemainingCooldownTicksByAction()[5] == 0 && m_self->getMana() >= m_game->getHasteManacost())
	{
		if (!CSettings::HAVE_HASTE(*this, *m_self))
		{
			m_move->setStatusTargetId(m_self->getId());
			m_move->setAction(model::ACTION_HASTE);
		}
		else
		{
			for (auto & unit : m_world->getWizards())
			{
				if (unit.isMe())
					continue;
				if (unit.getFaction() != m_self->getFaction())
					continue;
				if (m_self->getDistanceTo(unit) > m_self->getCastRange())
					continue;
				if (std::abs(m_self->getAngleTo(unit)) > m_game->getStaffSector() / 2.0)
					continue;
				if (CSettings::HAVE_HASTE(*this, unit))
					continue;
				m_move->setStatusTargetId(unit.getId());
				m_move->setAction(model::ACTION_HASTE);
				break;
			}
		}
	}

	m_tPowers.clear();

	if (!m_bSeedReady)
	{
		volatile unsigned int seed = (unsigned int)time(nullptr) + (unsigned int)m_game->getRandomSeed() + (unsigned int)m_self->getId() + (unsigned int)(m_self->getX() * 100.0) + (unsigned int)(m_self->getY() * 100.0);
		srand(seed);
		m_bSeedReady = true;
	}

	if (!m_bHealMode && m_self->getLife() < m_self->getMaxLife() * 0.25)
	{
		m_bHealMode = true;
		if (m_nLastHealTick + 100 > m_world->getTickIndex())
			m_bDoubleHealMode = true;
	}
	//if (m_self->getLife() < m_self->getMaxLife() * 0.15)
	//	m_bVeryHealMode = true;
	//if (m_self->getLife() >= m_self->getMaxLife() * 0.2)
	//	m_bVeryHealMode = false;
	if (m_self->getLife() >= m_self->getMaxLife() * (m_bDoubleHealMode ? 0.6 : 0.4))
	{
		m_bHealMode = false;
		m_bDoubleHealMode = false;
	}

	m_LastPositions.push_front(std::make_pair(m_self->getX(), m_self->getY()));
	if (m_LastPositions.size() > 150)
		m_LastPositions.pop_back();

	if (!m_FreeMode && !m_bCreepStop && m_world->getTickIndex() >= 150 && m_nLastHealTick < m_world->getTickIndex() - 150 && m_LastShootTick < m_world->getTickIndex() - 150 && m_self->getDistanceTo(1200.0, 1200.0) > m_self->getRadius() + m_game->getBonusRadius() + 60.0 && m_self->getDistanceTo(2800.0, 2800.0) > m_self->getRadius() + m_game->getBonusRadius() + 60.0)
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

	if (m_nLastReceivedTickIndex + 1 != m_world->getTickIndex())
		m_global.ReCheckLane(true);
	m_nLastReceivedTickIndex = m_world->getTickIndex();

	if (m_global.m_bEgoistMode)
	{
		if (m_world->getTickIndex() > 3000 && (m_world->getTickIndex() + 1000) % 2500 == 0)
			m_global.ReCheckLane(false);
	}
	else
	{
		if (m_world->getTickIndex() > 1500 && m_world->getTickIndex() % 1000 == 0)
			m_global.ReCheckLane(false);
	}

	auto waypoint = m_global.GetWaypoint();

	std::pair<double, double> result = { 0.0, 0.0 };

	if (m_FreeMode)
		AddPower("back", result, CalcPower(0.0, m_game->getMapSize(), 2000.0)); // back

	int nFoots = 0;
	for (auto & pos : m_LastPositions)
	{
		if (nFoots == 0)
		{
			nFoots++;
			continue;
		}
		if (std::abs(m_self->getX() - pos.first) < 0.1 || std::abs(m_self->getY() - pos.second) < 0.1)
			continue;
		nFoots++;
		AddPower("foot", result, CalcPower(pos.first, pos.second, -10.0));
		if (nFoots > 11)
			break;
	}

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
	m_tBuildings.push_back(model::Building(0, m_global.m_BS.first, m_global.m_BS.second, 0.0, 0.0, 0.0, m_self->getFaction() == model::FACTION_ACADEMY ? model::FACTION_RENEGADES : model::FACTION_ACADEMY, m_game->getFactionBaseRadius(), (int)m_game->getFactionBaseLife(), (int)m_game->getFactionBaseLife(), std::vector<model::Status>(), model::BUILDING_FACTION_BASE, m_game->getFactionBaseVisionRange(), m_game->getFactionBaseAttackRange(), 0, 0, m_game->getFactionBaseCooldownTicks()));
	if (m_global.m_top == CGlobal::LaneState::TOWER_1)
	{
		m_tBuildings.push_back(model::Building(0, m_global.m_T1.first, m_global.m_T1.second, 0.0, 0.0, 0.0, m_self->getFaction() == model::FACTION_ACADEMY ? model::FACTION_RENEGADES : model::FACTION_ACADEMY, m_game->getGuardianTowerRadius(), (int)m_game->getGuardianTowerLife(), (int)m_game->getGuardianTowerLife(), std::vector<model::Status>(), model::BUILDING_GUARDIAN_TOWER, m_game->getGuardianTowerVisionRange(), m_game->getGuardianTowerAttackRange(), 0, 0, m_game->getGuardianTowerCooldownTicks()));
		m_tBuildings.push_back(model::Building(0, m_global.m_T2.first, m_global.m_T2.second, 0.0, 0.0, 0.0, m_self->getFaction() == model::FACTION_ACADEMY ? model::FACTION_RENEGADES : model::FACTION_ACADEMY, m_game->getGuardianTowerRadius(), (int)m_game->getGuardianTowerLife(), (int)m_game->getGuardianTowerLife(), std::vector<model::Status>(), model::BUILDING_GUARDIAN_TOWER, m_game->getGuardianTowerVisionRange(), m_game->getGuardianTowerAttackRange(), 0, 0, m_game->getGuardianTowerCooldownTicks()));
	}
	else if (m_global.m_top == CGlobal::LaneState::TOWER_2)
	{
		m_tBuildings.push_back(model::Building(0, m_global.m_T2.first, m_global.m_T2.second, 0.0, 0.0, 0.0, m_self->getFaction() == model::FACTION_ACADEMY ? model::FACTION_RENEGADES : model::FACTION_ACADEMY, m_game->getGuardianTowerRadius(), (int)m_game->getGuardianTowerLife(), (int)m_game->getGuardianTowerLife(), std::vector<model::Status>(), model::BUILDING_GUARDIAN_TOWER, m_game->getGuardianTowerVisionRange(), m_game->getGuardianTowerAttackRange(), 0, 0, m_game->getGuardianTowerCooldownTicks()));
	}
	if (m_global.m_mid == CGlobal::LaneState::TOWER_1)
	{
		m_tBuildings.push_back(model::Building(0, m_global.m_M1.first, m_global.m_M1.second, 0.0, 0.0, 0.0, m_self->getFaction() == model::FACTION_ACADEMY ? model::FACTION_RENEGADES : model::FACTION_ACADEMY, m_game->getGuardianTowerRadius(), (int)m_game->getGuardianTowerLife(), (int)m_game->getGuardianTowerLife(), std::vector<model::Status>(), model::BUILDING_GUARDIAN_TOWER, m_game->getGuardianTowerVisionRange(), m_game->getGuardianTowerAttackRange(), 0, 0, m_game->getGuardianTowerCooldownTicks()));
		m_tBuildings.push_back(model::Building(0, m_global.m_M2.first, m_global.m_M2.second, 0.0, 0.0, 0.0, m_self->getFaction() == model::FACTION_ACADEMY ? model::FACTION_RENEGADES : model::FACTION_ACADEMY, m_game->getGuardianTowerRadius(), (int)m_game->getGuardianTowerLife(), (int)m_game->getGuardianTowerLife(), std::vector<model::Status>(), model::BUILDING_GUARDIAN_TOWER, m_game->getGuardianTowerVisionRange(), m_game->getGuardianTowerAttackRange(), 0, 0, m_game->getGuardianTowerCooldownTicks()));
	}
	else if (m_global.m_mid == CGlobal::LaneState::TOWER_2)
	{
		m_tBuildings.push_back(model::Building(0, m_global.m_M2.first, m_global.m_M2.second, 0.0, 0.0, 0.0, m_self->getFaction() == model::FACTION_ACADEMY ? model::FACTION_RENEGADES : model::FACTION_ACADEMY, m_game->getGuardianTowerRadius(), (int)m_game->getGuardianTowerLife(), (int)m_game->getGuardianTowerLife(), std::vector<model::Status>(), model::BUILDING_GUARDIAN_TOWER, m_game->getGuardianTowerVisionRange(), m_game->getGuardianTowerAttackRange(), 0, 0, m_game->getGuardianTowerCooldownTicks()));
	}
	if (m_global.m_bot == CGlobal::LaneState::TOWER_1)
	{
		m_tBuildings.push_back(model::Building(0, m_global.m_B1.first, m_global.m_B1.second, 0.0, 0.0, 0.0, m_self->getFaction() == model::FACTION_ACADEMY ? model::FACTION_RENEGADES : model::FACTION_ACADEMY, m_game->getGuardianTowerRadius(), (int)m_game->getGuardianTowerLife(), (int)m_game->getGuardianTowerLife(), std::vector<model::Status>(), model::BUILDING_GUARDIAN_TOWER, m_game->getGuardianTowerVisionRange(), m_game->getGuardianTowerAttackRange(), 0, 0, m_game->getGuardianTowerCooldownTicks()));
		m_tBuildings.push_back(model::Building(0, m_global.m_B2.first, m_global.m_B2.second, 0.0, 0.0, 0.0, m_self->getFaction() == model::FACTION_ACADEMY ? model::FACTION_RENEGADES : model::FACTION_ACADEMY, m_game->getGuardianTowerRadius(), (int)m_game->getGuardianTowerLife(), (int)m_game->getGuardianTowerLife(), std::vector<model::Status>(), model::BUILDING_GUARDIAN_TOWER, m_game->getGuardianTowerVisionRange(), m_game->getGuardianTowerAttackRange(), 0, 0, m_game->getGuardianTowerCooldownTicks()));
	}
	else if (m_global.m_bot == CGlobal::LaneState::TOWER_2)
	{
		m_tBuildings.push_back(model::Building(0, m_global.m_B2.first, m_global.m_B2.second, 0.0, 0.0, 0.0, m_self->getFaction() == model::FACTION_ACADEMY ? model::FACTION_RENEGADES : model::FACTION_ACADEMY, m_game->getGuardianTowerRadius(), (int)m_game->getGuardianTowerLife(), (int)m_game->getGuardianTowerLife(), std::vector<model::Status>(), model::BUILDING_GUARDIAN_TOWER, m_game->getGuardianTowerVisionRange(), m_game->getGuardianTowerAttackRange(), 0, 0, m_game->getGuardianTowerCooldownTicks()));
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

		AddPower("collision", result, CalcPower(unit, CSettings::PW_CIRCULAR_UNIT(*this, unit)));

		if (unit.getFaction() == m_self->getFaction())
		{
			AddPower("friendly wizard", result, CalcPower(unit, D > 2.0 * m_self->getVisionRange() ? 0.0 : CSettings::PW_FRIENDLY_WIZARD(*this, unit)));
		}
		else
		{
			if (!waypoint.second && D <= m_self->getVisionRange() && m_self->getDistanceTo(unit) < m_self->getDistanceTo(waypoint.first.first, waypoint.first.second))
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
				AddPower("enemy wizard", result, CalcPower(unit, D > 2.0 * m_self->getVisionRange() ? 0.0 : CSettings::PW_ENEMY_WIZARD(*this, unit)));
			}
		}
	}

	for (auto & unit : m_world->getMinions())
	{
		double D = m_self->getDistanceTo(unit);

		AddPower("collision", result, CalcPower(unit, CSettings::PW_CIRCULAR_UNIT(*this, unit)));

		if (unit.getType() == model::MINION_ORC_WOODCUTTER)
		{
			if (unit.getFaction() == m_self->getFaction())
				AddPower("friendly creep", result, CalcPower(unit, D > 2.0 * m_self->getVisionRange() ? 0.0 : CSettings::PW_FRIENDLY_CREEP_ORC(*this, unit)));
			else if (unit.getFaction() == model::FACTION_NEUTRAL)
				AddPower("neutral creep", result, CalcPower(unit, D > 2.0 * m_self->getVisionRange() ? 0.0 : CSettings::PW_NEUTRAL_CREEP_ORC(*this, unit)));
			else
			{
				if (!waypoint.second && D <= m_self->getVisionRange() && m_self->getDistanceTo(unit) < m_self->getDistanceTo(waypoint.first.first, waypoint.first.second))
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
					AddPower("enemy creep", result, CalcPower(unit, D > 2.0 * m_self->getVisionRange() ? 0.0 : CSettings::PW_ENEMY_CREEP_ORC(*this, unit)));
				}
			}
		}
		else if (unit.getType() == model::MINION_FETISH_BLOWDART)
		{
			if (unit.getFaction() == m_self->getFaction())
				AddPower("friendly creep", result, CalcPower(unit, D > 2.0 * m_self->getVisionRange() ? 0.0 : CSettings::PW_FRIENDLY_CREEP_FETISH(*this, unit)));
			else if (unit.getFaction() == model::FACTION_NEUTRAL)
				AddPower("neutral creep", result, CalcPower(unit, D > 2.0 * m_self->getVisionRange() ? 0.0 : CSettings::PW_NEUTRAL_CREEP_FETISH(*this, unit)));
			else
			{
				if (!waypoint.second && D <= m_self->getVisionRange() && m_self->getDistanceTo(unit) < m_self->getDistanceTo(waypoint.first.first, waypoint.first.second))
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
					AddPower("enemy creep", result, CalcPower(unit, D > 2.0 * m_self->getVisionRange() ? 0.0 : CSettings::PW_ENEMY_CREEP_FETISH(*this, unit)));
				}
			}
		}
	}

	for (auto & unit : m_world->getTrees())
	{
		double D = m_self->getDistanceTo(unit);
		if (D > 2.0 * m_self->getVisionRange())
			continue;

		AddPower("collision", result, CalcPower(unit, CSettings::PW_CIRCULAR_UNIT(*this, unit)));

		AddPower("tree", result, CalcPower(unit, CSettings::PW_TREE(*this, unit)));
	}

	m_bNoShoot = false;

	for (auto & unit : m_world->getProjectiles())
	{
		double D = m_self->getDistanceTo(unit);
		if (D > 2.0 * m_self->getVisionRange())
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
				printf("Tick %d: found projectile, trying to evade!\r\n", m_world->getTickIndex());
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
		if (m_self->getDistanceTo(1200.0, 1200.0) <= 1800.0 && (m_global.CanGoToBonus() || m_self->getDistanceTo(1200.0, 1200.0) <= 500.0))
			waypoint.first = { 1200.0 + (m_world->getTickIndex() % 2500 > 2000 ? m_game->getBonusRadius() + m_self->getRadius() + 5.9 : 0.0), 1200.0 };
	}
	else if (m_global.m_bBonusB)
	{
		if (m_self->getDistanceTo(2800.0, 2800.0) <= 1800.0 && (m_global.CanGoToBonus() || m_self->getDistanceTo(2800.0, 2800.0) <= 500.0))
			waypoint.first = { 2800.0 - (m_world->getTickIndex() % 2500 > 2000 ? m_game->getBonusRadius() + m_self->getRadius() + 5.9 : 0.0), 2800.0 };
	}

	if (pEnMinUnit && enMinBase < 1300.0 && m_global.OwnLaneControl())
	{
		m_global.m_lane = m_global.GetLane(*pEnMinUnit);
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
		double T = std::min(15.0, D / m_game->getMagicMissileSpeed());
		double R = std::ceil(T) * m_game->getMinionSpeed();
		if (D > m_self->getCastRange() + unit.getRadius() - m_game->getMagicMissileRadius() - R - 0.1 + m_global.RangeLevel(*m_self) * m_game->getRangeBonusPerSkillLevel())
			continue;
		if (unit.getFaction() == model::FACTION_NEUTRAL)
		{
			if (!(unit.getSpeedX() > 0.0 || unit.getSpeedY() > 0.0 || unit.getLife() < unit.getMaxLife() || unit.getRemainingActionCooldownTicks() > 0))
			{
				const model::CircularUnit * tarMinND = nullptr;
				double minND = 400000.0;
				for (auto & u : m_world->getWizards())
				{
					if (unit.getDistanceTo(u) < minND)
					{
						minND = unit.getDistanceTo(u);
						tarMinND = &u;
					}
				}
				for (auto & u : m_world->getMinions())
				{
					if (u.getFaction() == model::FACTION_NEUTRAL)
						continue;
					if (unit.getDistanceTo(u) < minND)
					{
						minND = unit.getDistanceTo(u);
						tarMinND = &u;
					}
				}
				for (auto & u : m_world->getBuildings())
				{
					if (unit.getDistanceTo(u) < minND)
					{
						minND = unit.getDistanceTo(u);
						tarMinND = &u;
					}
				}
				if (tarMinND && tarMinND->getFaction() != m_self->getFaction() && minND < tarMinND->getRadius() + 300.0)
				{
					if (dynamic_cast<const model::Wizard *>(tarMinND))
					{
						double P = 20000.0 * ((dynamic_cast<const model::Wizard *>(tarMinND)->getMaxLife() - dynamic_cast<const model::Wizard *>(tarMinND)->getLife() + 1.0) / dynamic_cast<const model::Wizard *>(tarMinND)->getMaxLife());
						if (P > MAX_PRIORITY)
						{
							target = &unit;
							MAX_PRIORITY = P;
						}
					}
					if (dynamic_cast<const model::Minion *>(tarMinND))
					{
						double P = 100.0 * ((dynamic_cast<const model::Minion *>(tarMinND)->getMaxLife() - dynamic_cast<const model::Minion *>(tarMinND)->getLife() + 1.0) / dynamic_cast<const model::Minion *>(tarMinND)->getMaxLife());
						if (P > MAX_PRIORITY)
						{
							target = &unit;
							MAX_PRIORITY = P;
						}
					}
					if (dynamic_cast<const model::Building *>(tarMinND))
					{
						double P = (dynamic_cast<const model::Building *>(tarMinND)->getType() == model::BUILDING_FACTION_BASE ? 100000.0 : 50000.0);
						if (P > MAX_PRIORITY)
						{
							target = &unit;
							MAX_PRIORITY = P;
						}
					}
				}
			}
			if (!m_FreeMode && !(unit.getSpeedX() > 0.0 || unit.getSpeedY() > 0.0 || unit.getLife() < unit.getMaxLife() || unit.getRemainingActionCooldownTicks() > 0))
				continue;
		}
		double P = (unit.getFaction() == model::FACTION_NEUTRAL ? 15.0 : 100.0) * ((unit.getMaxLife() - unit.getLife() + 1.0) / unit.getMaxLife());
		if (unit.getLife() <= 12)
			P = 1000000.0;
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
		double T = std::min(15.0, D / m_game->getMagicMissileSpeed());
		double R = std::ceil(T) * m_game->getWizardForwardSpeed();
		if (D > m_self->getCastRange() + unit.getRadius() - m_game->getMagicMissileRadius() - R - 0.1 + m_global.RangeLevel(*m_self) * m_game->getRangeBonusPerSkillLevel())
			continue;
		double P = 20000.0 * ((unit.getMaxLife() - unit.getLife() + 1.0) / unit.getMaxLife());
		if (unit.getLife() <= 12)
			P = 1000000.0;
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
		if (m_self->getDistanceTo(unit) > m_self->getCastRange() + unit.getRadius() - m_game->getMagicMissileRadius() - 0.1 + m_global.RangeLevel(*m_self) * m_game->getRangeBonusPerSkillLevel())
			continue;
		double P = (unit.getType() == model::BUILDING_FACTION_BASE ? 100000.0 : 50000.0);
		if (P > MAX_PRIORITY)
		{
			target = &unit;
			MAX_PRIORITY = P;
		}
	}

	for (auto & unit : m_world->getTrees())
	{
		double D = m_self->getDistanceTo(unit);
		if (D > m_self->getCastRange() + unit.getRadius() - m_game->getMagicMissileRadius() - 0.1 + m_global.RangeLevel(*m_self) * m_game->getRangeBonusPerSkillLevel())
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

	if (std::hypot(direction.first, direction.second) > 5000.0)
	{
		printf("GOGOGO! (%f)\r\n", std::hypot(direction.first, direction.second));
		shoot = false;
	}

	if (!shoot)
	{
		bool bFound = false;
		double evAngle = 0.0;;
		for (auto & wizard : m_world->getWizards())
		{
			if (wizard.getFaction() == m_self->getFaction())
				continue;

			double D = m_self->getDistanceTo(wizard);
			if (D > m_game->getWizardCastRange() + m_self->getRadius() + m_game->getMagicMissileRadius() + m_global.RangeLevel(wizard) * m_game->getRangeBonusPerSkillLevel())
				continue;

			if (D < m_self->getRadius() + m_game->getMagicMissileRadius() + 400.0)
				continue;

			if (std::abs(wizard.getAngleTo(*m_self)) > PI / 2.0)
				continue;

			bFound = true;
			evAngle = m_self->getAngleTo(wizard);
			break;
		}

		if (bFound)
		{
			printf("Tick %d: found appropriate wizard!\r\n", m_world->getTickIndex());
			m_move->setTurn(evAngle < 0.0 ? evAngle + PI / 2.0 : evAngle - PI / 2.0);
		}
		else
		{
			m_move->setTurn(angle);
		}

		const model::Tree * tree = nullptr;
		double minD = 40000.0;

		if (m_self->getDistanceTo(1200.0 + m_game->getBonusRadius() + m_self->getRadius() + 5.9, 1200.0) > 10.0 && m_self->getDistanceTo(2800.0 - m_game->getBonusRadius() - m_self->getRadius() - 5.9, 2800.0) > 10.0)
		{
			for (auto & unit : m_world->getTrees())
			{
				double D = m_self->getDistanceTo(unit);
				if (D > m_self->getCastRange() + unit.getRadius() - m_game->getMagicMissileRadius() + m_global.RangeLevel(*m_self) * m_game->getRangeBonusPerSkillLevel() - 0.1)
					continue;
				if (std::abs(m_self->getAngleTo(unit)) > m_game->getStaffSector() / 2.0)
					continue;
				if (D < minD)
				{
					minD = D;
					tree = &unit;
				}
			}
		}

		if (tree)
		{
			BestShoot(*tree, false);
		}
	}

	bool nHaste = false;
	for (auto & status : m_self->getStatuses())
	{
		if (status.getType() == model::STATUS_HASTENED)
		{
			nHaste = true;
			break;
		}
	}

	double ver = std::cos(angle);
	double hor = std::sin(angle);

	m_move->setSpeed(ver * (ver > 0.0 ? 4.0 : 3.0) * (1.0 + (nHaste ? m_game->getHastenedMovementBonusFactor() : 0.0) + m_global.SpeedLevel(*m_self) * m_game->getMovementBonusFactorPerSkillLevel()));
	m_move->setStrafeSpeed(hor * 3.0 * (1.0 + (nHaste ? m_game->getHastenedMovementBonusFactor() : 0.0) + m_global.SpeedLevel(*m_self) * m_game->getMovementBonusFactorPerSkillLevel()));
}

void MyStrategy::BestShoot(const model::CircularUnit & unit, bool turn)
{
	bool bWizard = false;
	if (dynamic_cast<const model::Wizard *>(&unit))
		bWizard = true;
	bool bBuilding = false;
	if (dynamic_cast<const model::Building *>(&unit))
		bBuilding = true;

	double D = m_self->getDistanceTo(unit);
	double angle = m_self->getAngleTo(unit);
	if (turn)
	{
		m_LastShootTick = m_world->getTickIndex();
		m_move->setTurn(angle);
	}

	bool bCanUseFireball = true;
	for (auto & minion : m_world->getMinions())
	{
		if (minion.getFaction() != m_self->getFaction())
			continue;
		if (minion.getDistanceTo(unit) < unit.getRadius() + 100.0)
		{
			bCanUseFireball = false;
			break;
		}
	}
	if (bCanUseFireball)
	{
		for (auto & wizard : m_world->getWizards())
		{
			if (wizard.getFaction() != m_self->getFaction())
				continue;
			if (wizard.getDistanceTo(unit) < unit.getRadius() + 100.0)
			{
				bCanUseFireball = false;
				break;
			}
		}
	}

	if (D - unit.getRadius() < m_game->getStaffRange() && m_self->getRemainingActionCooldownTicks() == 0 && m_self->getRemainingCooldownTicksByAction()[1] == 0)
	{
		if (std::abs(angle) < m_game->getStaffSector() / 2.0 && m_move->getAction() == model::ACTION_NONE)
			m_move->setAction(model::ACTION_STAFF);
	}
	else if (m_setCurrentSkills.find(model::SKILL_FROST_BOLT) != m_setCurrentSkills.end() && m_self->getRemainingActionCooldownTicks() == 0 && m_self->getRemainingCooldownTicksByAction()[3] == 0 && bWizard && m_self->getMana() >= m_game->getFrostBoltManacost())
	{
		if (std::abs(angle) < m_game->getStaffSector() / 2.0 && m_move->getAction() == model::ACTION_NONE)
		{
			m_move->setAction(model::ACTION_FROST_BOLT);
			m_move->setCastAngle(angle);
			m_move->setMinCastDistance(D - unit.getRadius() + m_game->getFrostBoltRadius());
		}
	}
	else if (m_setCurrentSkills.find(model::SKILL_FIREBALL) != m_setCurrentSkills.end() && m_self->getRemainingActionCooldownTicks() == 0 && m_self->getRemainingCooldownTicksByAction()[4] == 0 && bCanUseFireball && (bWizard || bBuilding) && m_self->getMana() >= m_game->getFireballManacost())
	{
		if (std::abs(angle) < m_game->getStaffSector() / 2.0 && m_move->getAction() == model::ACTION_NONE)
		{
			m_move->setAction(model::ACTION_FIREBALL);
			m_move->setCastAngle(angle);
			m_move->setMinCastDistance(D - unit.getRadius() + m_game->getFireballRadius());
		}
	}
	else if (m_self->getRemainingActionCooldownTicks() == 0 && m_self->getRemainingCooldownTicksByAction()[2] == 0 && m_self->getMana() >= m_game->getMagicMissileManacost())
	{
		double T = std::min(15.0, D / m_game->getMagicMissileSpeed());
		double newX = bWizard ? unit.getX() : unit.getX() + unit.getSpeedX() * T;
		double newY = bWizard ? unit.getY() : unit.getY() + unit.getSpeedY() * T;
		double newD = std::hypot(m_self->getX() - newX, m_self->getY() - newY);
		double newAngle = m_self->getAngleTo(newX, newY);
		if (turn)
			m_move->setTurn(newAngle);
		if (std::abs(newAngle) < m_game->getStaffSector() / 2.0 && m_move->getAction() == model::ACTION_NONE)
		{
			m_move->setAction(model::ACTION_MAGIC_MISSILE);
			m_move->setCastAngle(newAngle);
			m_move->setMinCastDistance(newD - unit.getRadius() + m_game->getMagicMissileRadius());
		}
	}
}