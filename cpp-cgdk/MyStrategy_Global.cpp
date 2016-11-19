#include "MyStrategy_Global.h"
#include "MyStrategy.h"


CGlobal::CGlobal(MyStrategy & strategy) : m_lane(model::_LANE_UNKNOWN_), m_bLaneChoosed(false), m_strategy(strategy), m_top(LaneState::TOWER_1), m_mid(LaneState::TOWER_1), m_bot(LaneState::TOWER_1), m_bVaidCords(false), m_bBonusT(false), m_bBonusB(false), m_bLaneRush(false), m_bMasterNotSilent(false)
{
	m_T1 = { 0.0, 0.0 };
	m_T2 = { 0.0, 0.0 };
	m_M1 = { 0.0, 0.0 };
	m_M2 = { 0.0, 0.0 };
	m_B1 = { 0.0, 0.0 };
	m_B2 = { 0.0, 0.0 };
	m_BS = { 0.0, 0.0 };
}

model::LaneType CGlobal::GetLane(const model::LivingUnit & unit)
{
	std::pair<double, double> T1 = { 0.0, 2000.0 };
	std::pair<double, double> T2 = { 2000.0, 0.0 };
	std::pair<double, double> M1 = { 1600.0, 2400.0 };
	std::pair<double, double> M2 = { 2400.0, 1600.0 };
	std::pair<double, double> B1 = { 2000.0, 4000.0 };
	std::pair<double, double> B2 = { 4000.0, 2000.0 };

	double toT1 = unit.getDistanceTo(T1.first, T1.second);
	double toT2 = unit.getDistanceTo(T2.first, T2.second);
	double toM1 = unit.getDistanceTo(M1.first, M1.second);
	double toM2 = unit.getDistanceTo(M2.first, M2.second);
	double toB1 = unit.getDistanceTo(B1.first, B1.second);
	double toB2 = unit.getDistanceTo(B2.first, B2.second);

	if (toT1 < toM1 && toT1 < toM2 && toT1 < toB1 && toT1 < toB2)
		return model::LANE_TOP;

	if (toT2 < toM1 && toT2 < toM2 && toT2 < toB1 && toT2 < toB2)
		return model::LANE_TOP;

	if (toM1 < toT1 && toM1 < toT2 && toM1 < toB1 && toM1 < toB2)
		return model::LANE_MIDDLE;

	if (toM2 < toT1 && toM2 < toT2 && toM2 < toB1 && toM2 < toB2)
		return model::LANE_MIDDLE;

	if (toB1 < toT1 && toB1 < toT2 && toB1 < toM1 && toB1 < toM2)
		return model::LANE_BOTTOM;

	if (toB2 < toT1 && toB2 < toT2 && toB2 < toM1 && toB2 < toM2)
		return model::LANE_BOTTOM;

	return model::_LANE_UNKNOWN_;
}

void CGlobal::ChooseLane()
{
	if (m_lane == model::_LANE_UNKNOWN_)
	{
		if (m_strategy.m_self->isMaster())
		{
			m_lane = model::LANE_MIDDLE;
			m_bLaneChoosed = true;
			std::vector<model::Message> m_tMessages = {
				model::Message(model::LANE_TOP, model::_SKILL_UNKNOWN_, std::vector<signed char>()),
				model::Message(model::LANE_TOP, model::_SKILL_UNKNOWN_, std::vector<signed char>()),
				model::Message(model::LANE_BOTTOM, model::_SKILL_UNKNOWN_, std::vector<signed char>()),
				model::Message(model::LANE_BOTTOM, model::_SKILL_UNKNOWN_, std::vector<signed char>())
			};
			m_strategy.m_move->setMessages(m_tMessages);
			return;
		}

		volatile int rand_number = rand();
		int l = rand_number % 3;
		switch (l)
		{
		case 0: m_lane = model::LANE_TOP; break;
		case 1: m_lane = model::LANE_MIDDLE; break;
		case 2: m_lane = model::LANE_BOTTOM; break;
		}

		m_bLaneChoosed = true; // no rechoose lane! but master wizard can do it!
	}

	for (auto & message : m_strategy.m_self->getMessages())
	{
		if (message.getLane() == model::_LANE_UNKNOWN_)
			continue;
		m_bMasterNotSilent = true;
		m_lane = message.getLane();
		m_bLaneChoosed = true;
	}

	if (m_bLaneChoosed)
		return;

	int nTopWizards = 0;
	int nMidWizards = 0;
	int nBotWizards = 0;
	for (auto & unit : m_strategy.m_world->getWizards())
	{
		if (unit.getFaction() == m_strategy.m_self->getFaction())
			continue;
		
		if (GetLane(unit) == model::LANE_TOP)
			nTopWizards++;
		else if (GetLane(unit) == model::LANE_MIDDLE)
			nMidWizards++;
		else if (GetLane(unit) == model::LANE_BOTTOM)
			nBotWizards++;
	}

	if (nTopWizards + nMidWizards + nBotWizards < 4)
		return;

	if (nTopWizards < nBotWizards)
	{
		if (nTopWizards < nMidWizards)
		{
			m_lane = model::LANE_TOP;
		}
		else
		{
			m_lane = model::LANE_MIDDLE;
		}
	}
	else
	{
		if (nBotWizards < nMidWizards)
		{
			m_lane = model::LANE_BOTTOM;
		}
		else
		{
			m_lane = model::LANE_MIDDLE;
		}
	}

	m_bLaneChoosed = true;
}

std::pair<std::pair<double, double>, bool> CGlobal::GetWaypoint()
{
	bool priority = false;

	if (m_lane == model::LANE_TOP)
	{
		if (m_top == LaneState::TOWER_1) return { { (m_strategy.m_self->getY() < 700.0 ? m_T1.first : 250.0), m_T1.second }, priority };
		if (m_top == LaneState::TOWER_2) return { { (m_strategy.m_self->getY() < 700.0 ? m_T2.first : 250.0), m_T2.second }, priority };
		if (m_top == LaneState::BASE)
		{
			double baseX = m_BS.first - 200.0;
			double baseY = m_BS.second - 150.0;

			if ((m_strategy.m_world->getTickIndex() - 1) % 750 > 550)
			{
				baseX = m_BS.first - 800.0;
				priority = true;
			}

			return { { (m_strategy.m_self->getY() < 700.0 ? baseX : 250.0), baseY }, priority };
		}
	}
	else if (m_lane == model::LANE_MIDDLE)
	{
		if (m_mid == LaneState::TOWER_1) return { { m_M1.first, m_M1.second }, priority };
		if (m_mid == LaneState::TOWER_2) return { { m_M2.first, m_M2.second }, priority };
		if (m_mid == LaneState::BASE)
		{
			double baseX = m_BS.first - 200.0;
			double baseY = m_BS.second + 200.0;

			if ((m_strategy.m_world->getTickIndex() - 1) % 750 > 550)
			{
				baseX = m_BS.first - 800.0;
				baseY = m_BS.second + 800.0;
				priority = true;
			}

			return { { baseX, baseY }, priority };
		}
	}
	else if (m_lane == model::LANE_BOTTOM)
	{
		if (m_bot == LaneState::TOWER_1) return { { m_B1.first, (m_strategy.m_self->getX() > m_strategy.m_game->getMapSize() - 700.0 ? m_B1.second : m_strategy.m_game->getMapSize() - 250.0) }, priority };
		if (m_bot == LaneState::TOWER_2) return { { m_B2.first, (m_strategy.m_self->getX() > m_strategy.m_game->getMapSize() - 700.0 ? m_B2.second : m_strategy.m_game->getMapSize() - 250.0) }, priority };
		if (m_bot == LaneState::BASE)
		{
			double baseX = m_BS.first + 150.0;
			double baseY = m_BS.second + 200.0;

			if ((m_strategy.m_world->getTickIndex() - 1) % 750 > 550)
			{
				baseX = m_BS.second + 800.0;
				priority = true;
			}

			return { { baseX, (m_strategy.m_self->getX() > m_strategy.m_game->getMapSize() - 700.0 ? baseY : m_strategy.m_game->getMapSize() - 250.0) }, priority };
		}
	}
	return { { m_BS.first, m_BS.second }, priority };
}

bool CGlobal::EnemyTowerNotExists(std::pair<double, double> position)
{
	for (auto & unit : m_strategy.m_world->getWizards())
	{
		if (unit.getFaction() != m_strategy.m_self->getFaction())
			continue;
		if (unit.getDistanceTo(position.first, position.second) >= m_strategy.m_game->getWizardVisionRange())
			continue;
		bool bFound = false;
		for (auto & building : m_strategy.m_world->getBuildings())
		{
			if (building.getFaction() == m_strategy.m_self->getFaction())
				continue;
			if (building.getType() == model::BUILDING_FACTION_BASE)
				continue;
			if (position.first - 0.1 < building.getX() && building.getX() < position.first + 0.1 && position.second - 0.1 < building.getY() && building.getY() < position.second + 0.1)
			{
				bFound = true;
				break;
			}
		}
		if (!bFound)
			return true;
	}
	for (auto & unit : m_strategy.m_world->getMinions())
	{
		if (unit.getFaction() != m_strategy.m_self->getFaction())
			continue;
		if (unit.getDistanceTo(position.first, position.second) >= m_strategy.m_game->getMinionVisionRange())
			continue;
		bool bFound = false;
		for (auto & building : m_strategy.m_world->getBuildings())
		{
			if (building.getFaction() == m_strategy.m_self->getFaction())
				continue;
			if (building.getType() == model::BUILDING_FACTION_BASE)
				continue;
			if (position.first - 0.1 < building.getX() && building.getX() < position.first + 0.1 && position.second - 0.1 < building.getY() && building.getY() < position.second + 0.1)
			{
				bFound = true;
				break;
			}
		}
		if (!bFound)
			return true;
	}
	return false;
}

bool CGlobal::FriendlyTowerNotExists(std::pair<double, double> position)
{
	bool bFound = false;
	for (auto & building : m_strategy.m_world->getBuildings())
	{
		if (building.getFaction() != m_strategy.m_self->getFaction())
			continue;
		if (building.getType() == model::BUILDING_FACTION_BASE)
			continue;
		if (position.first - 0.1 < building.getX() && building.getX() < position.first + 0.1 && position.second - 0.1 < building.getY() && building.getY() < position.second + 0.1)
		{
			bFound = true;
			break;
		}
	}
	return !bFound;
}

bool CGlobal::BonusNotExists(std::pair<double, double> position)
{
	for (auto & unit : m_strategy.m_world->getWizards())
	{
		if (unit.getFaction() != m_strategy.m_self->getFaction())
			continue;
		if (unit.getDistanceTo(position.first, position.second) >= m_strategy.m_game->getWizardVisionRange())
			continue;
		bool bFound = false;
		for (auto & bonus : m_strategy.m_world->getBonuses())
		{
			if (position.first - 0.1 < bonus.getX() && bonus.getX() < position.first + 0.1 && position.second - 0.1 < bonus.getY() && bonus.getY() < position.second + 0.1)
			{
				bFound = true;
				break;
			}
		}
		if (!bFound)
			return true;
	}
	for (auto & unit : m_strategy.m_world->getMinions())
	{
		if (unit.getFaction() != m_strategy.m_self->getFaction())
			continue;
		if (unit.getDistanceTo(position.first, position.second) >= m_strategy.m_game->getMinionVisionRange())
			continue;
		bool bFound = false;
		for (auto & bonus : m_strategy.m_world->getBonuses())
		{
			if (position.first - 0.1 < bonus.getX() && bonus.getX() < position.first + 0.1 && position.second - 0.1 < bonus.getY() && bonus.getY() < position.second + 0.1)
			{
				bFound = true;
				break;
			}
		}
		if (!bFound)
			return true;
	}
	return false;
}

void CGlobal::Update()
{
	if (m_top == LaneState::TOWER_1 && EnemyTowerNotExists(m_T1))
	{
		m_top = LaneState::TOWER_2;
	}
	if (m_top == LaneState::TOWER_2 && EnemyTowerNotExists(m_T2))
	{
		m_top = LaneState::BASE;
		if (OwnLaneControl() && !m_bLaneRush && m_strategy.m_world->getTickIndex() < 10000 && Tower2Exists())
		{
			m_lane = model::LANE_TOP;
			m_bLaneChoosed = true;
			m_bLaneRush = true;
			if (m_strategy.m_self->isMaster())
			{
				std::vector<model::Message> m_tMessages = {
					model::Message(model::LANE_TOP, model::_SKILL_UNKNOWN_, std::vector<signed char>()),
					model::Message(model::LANE_TOP, model::_SKILL_UNKNOWN_, std::vector<signed char>()),
					model::Message(model::LANE_TOP, model::_SKILL_UNKNOWN_, std::vector<signed char>()),
					model::Message(model::LANE_TOP, model::_SKILL_UNKNOWN_, std::vector<signed char>())
				};
				m_strategy.m_move->setMessages(m_tMessages);
			}
		}
	}

	if (m_mid == LaneState::TOWER_1 && EnemyTowerNotExists(m_M1))
	{
		m_mid = LaneState::TOWER_2;
	}
	if (m_mid == LaneState::TOWER_2 && EnemyTowerNotExists(m_M2))
	{
		m_mid = LaneState::BASE;
		if (OwnLaneControl() && !m_bLaneRush && m_strategy.m_world->getTickIndex() < 10000 && Tower2Exists())
		{
			m_lane = model::LANE_MIDDLE;
			m_bLaneChoosed = true;
			m_bLaneRush = true;
			if (m_strategy.m_self->isMaster())
			{
				std::vector<model::Message> m_tMessages = {
					model::Message(model::LANE_MIDDLE, model::_SKILL_UNKNOWN_, std::vector<signed char>()),
					model::Message(model::LANE_MIDDLE, model::_SKILL_UNKNOWN_, std::vector<signed char>()),
					model::Message(model::LANE_MIDDLE, model::_SKILL_UNKNOWN_, std::vector<signed char>()),
					model::Message(model::LANE_MIDDLE, model::_SKILL_UNKNOWN_, std::vector<signed char>())
				};
				m_strategy.m_move->setMessages(m_tMessages);
			}
		}
	}

	if (m_bot == LaneState::TOWER_1 && EnemyTowerNotExists(m_B1))
	{
		m_bot = LaneState::TOWER_2;
	}
	if (m_bot == LaneState::TOWER_2 && EnemyTowerNotExists(m_B2))
	{
		m_bot = LaneState::BASE;
		if (OwnLaneControl() && !m_bLaneRush && m_strategy.m_world->getTickIndex() < 10000 && Tower2Exists())
		{
			m_lane = model::LANE_BOTTOM;
			m_bLaneChoosed = true;
			m_bLaneRush = true;
			if (m_strategy.m_self->isMaster())
			{
				std::vector<model::Message> m_tMessages = {
					model::Message(model::LANE_BOTTOM, model::_SKILL_UNKNOWN_, std::vector<signed char>()),
					model::Message(model::LANE_BOTTOM, model::_SKILL_UNKNOWN_, std::vector<signed char>()),
					model::Message(model::LANE_BOTTOM, model::_SKILL_UNKNOWN_, std::vector<signed char>()),
					model::Message(model::LANE_BOTTOM, model::_SKILL_UNKNOWN_, std::vector<signed char>())
				};
				m_strategy.m_move->setMessages(m_tMessages);
			}
		}
	}

	if ((m_strategy.m_world->getTickIndex() - 1) % 2500 >= 2400)
	{
		m_bBonusT = true;
		m_bBonusB = true;
	}
	else if ((m_strategy.m_world->getTickIndex() - 1) % 2500 > 2000)
	{
		m_bBonusT = false;
		m_bBonusB = false;
	}
	if (m_bBonusT)
	{
		if ((m_strategy.m_world->getTickIndex() - 1) % 2500 < 2400)
		{
			if (BonusNotExists(std::make_pair(1200.0, 1200.0)))
				m_bBonusT = false;
		}
	}
	if (m_bBonusB)
	{
		if ((m_strategy.m_world->getTickIndex() - 1) % 2500 < 2400)
		{
			if (BonusNotExists(std::make_pair(2800.0, 2800.0)))
				m_bBonusB = false;
		}
	}
}

void CGlobal::SetTowerCords()
{
	if (m_bVaidCords)
		return;

	for (auto & unit : m_strategy.m_world->getBuildings())
	{
		if (unit.getFaction() != m_strategy.m_self->getFaction())
			continue;

		if (unit.getType() == model::BUILDING_FACTION_BASE)
			m_BS = { m_strategy.m_game->getMapSize() - unit.getX(), m_strategy.m_game->getMapSize() - unit.getY() };

		if (unit.getType() == model::BUILDING_GUARDIAN_TOWER)
		{
			if (m_strategy.m_game->getMapSize() - unit.getX() > m_strategy.m_game->getMapSize() - 600.0) // bot
			{
				if (m_strategy.m_game->getMapSize() - unit.getY() > m_B1.second)
				{
					std::swap(m_B1, m_B2);
					m_B1 = { m_strategy.m_game->getMapSize() - unit.getX(), m_strategy.m_game->getMapSize() - unit.getY() };
				}
				else
				{
					m_B2 = { m_strategy.m_game->getMapSize() - unit.getX(), m_strategy.m_game->getMapSize() - unit.getY() };
				}
			}
			else if (m_strategy.m_game->getMapSize() - unit.getY() < 600.0) // top
			{
				if (m_strategy.m_game->getMapSize() - unit.getX() > m_T2.first)
				{
					std::swap(m_T1, m_T2);
					m_T2 = { m_strategy.m_game->getMapSize() - unit.getX(), m_strategy.m_game->getMapSize() - unit.getY() };
				}
				else
				{
					m_T1 = { m_strategy.m_game->getMapSize() - unit.getX(), m_strategy.m_game->getMapSize() - unit.getY() };
				}
			}
			else
			{
				if (m_strategy.m_game->getMapSize() - unit.getY() > m_M1.second)
				{
					std::swap(m_M1, m_M2);
					m_M1 = { m_strategy.m_game->getMapSize() - unit.getX(), m_strategy.m_game->getMapSize() - unit.getY() };
				}
				else
				{
					m_M2 = { m_strategy.m_game->getMapSize() - unit.getX(), m_strategy.m_game->getMapSize() - unit.getY() };
				}
			}
		}
	}

	m_bVaidCords = true;
}

bool CGlobal::OwnLaneControl()
{
	if (m_strategy.m_self->isMaster())
		return true;
	if (m_bMasterNotSilent)
		return false;
	return true;
}

bool CGlobal::Tower2Exists()
{
	if (FriendlyTowerNotExists({ m_strategy.m_game->getMapSize() - m_T2.first, m_strategy.m_game->getMapSize() - m_T2.second }))
		return false;
	if (FriendlyTowerNotExists({ m_strategy.m_game->getMapSize() - m_M2.first, m_strategy.m_game->getMapSize() - m_M2.second }))
		return false;
	if (FriendlyTowerNotExists({ m_strategy.m_game->getMapSize() - m_B2.first, m_strategy.m_game->getMapSize() - m_B2.second }))
		return false;
	return true;
}

void CGlobal::ReCheckLane()
{
	if (!OwnLaneControl() && !m_bLaneRush)
		return;

	int nTopWizards = 0;
	int nMidWizards = 0;
	int nBotWizards = 0;
	int nTopWizardsEnemy = 0;
	int nMidWizardsEnemy = 0;
	int nBotWizardsEnemy = 0;
	for (auto & unit : m_strategy.m_world->getWizards())
	{
		if (unit.getFaction() == m_strategy.m_self->getFaction())
		{
			if (GetLane(unit) == model::LANE_TOP)
				nTopWizards++;
			else if (GetLane(unit) == model::LANE_MIDDLE)
				nMidWizards++;
			else if (GetLane(unit) == model::LANE_BOTTOM)
				nBotWizards++;
		}
		else
		{
			if (GetLane(unit) == model::LANE_TOP)
				nTopWizardsEnemy++;
			else if (GetLane(unit) == model::LANE_MIDDLE)
				nMidWizardsEnemy++;
			else if (GetLane(unit) == model::LANE_BOTTOM)
				nBotWizardsEnemy++;
		}
	}

	printf("FW: T - %d / M - %d / B - %d\r\n", nTopWizards, nMidWizards, nBotWizards);
	printf("EW: T - %d / M - %d / B - %d\r\n", nTopWizardsEnemy, nMidWizardsEnemy, nBotWizardsEnemy);

	if (nTopWizardsEnemy + nMidWizardsEnemy + nBotWizardsEnemy == 5)
	{
		if (nTopWizards < nTopWizardsEnemy)
			m_lane = model::LANE_TOP;
		else if (nBotWizards < nBotWizardsEnemy)
			m_lane = model::LANE_BOTTOM;
		else if (nMidWizards < nMidWizardsEnemy)
			m_lane = model::LANE_MIDDLE;

		m_bLaneChoosed = true;
		return;
	}

	if (nTopWizards == 0)
	{
		m_lane = model::LANE_TOP;
		m_bLaneChoosed = true;
		return;
	}

	if (nMidWizards == 0)
	{
		m_lane = model::LANE_MIDDLE;
		m_bLaneChoosed = true;
		return;
	}


	if (nBotWizards == 0)
	{
		m_lane = model::LANE_BOTTOM;
		m_bLaneChoosed = true;
		return;
	}

	if (nTopWizards >= 3)
	{
		if (nMidWizards < nBotWizards)
			m_lane = model::LANE_MIDDLE;
		else
			m_lane = model::LANE_BOTTOM;
	}
	if (nMidWizards >= 3)
	{
		if (nBotWizards < nTopWizards)
			m_lane = model::LANE_BOTTOM;
		else
			m_lane = model::LANE_TOP;
	}
	if (nBotWizards >= 3)
	{
		if (nMidWizards < nTopWizards)
			m_lane = model::LANE_MIDDLE;
		else
			m_lane = model::LANE_TOP;
	}

	m_bLaneChoosed = true;
}