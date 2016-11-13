#include "MyStrategy_Global.h"
#include "MyStrategy.h"


CGlobal::CGlobal(MyStrategy & strategy) : m_lane(model::_LANE_UNKNOWN_), m_bLaneChoosed(false), m_strategy(strategy), m_top(LaneState::TOWER_1), m_mid(LaneState::TOWER_1), m_bot(LaneState::TOWER_1), m_bVaidCords(false)
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
	double toTop = unit.getDistanceTo(200.0, 200.0);
	double toMid = unit.getDistanceTo(2000.0, 2000.0);
	double toBot = unit.getDistanceTo(m_strategy.m_game->getMapSize() - 200.0, m_strategy.m_game->getMapSize() - 200.0);

	if (toTop < toMid)
	{
		if (toTop < toBot)
		{
			return model::LANE_TOP;
		}
		else
		{
			return model::LANE_BOTTOM;
		}
	}
	else
	{
		if (toMid < toBot)
		{
			return model::LANE_MIDDLE;
		}
		else
		{
			return model::LANE_BOTTOM;
		}
	}
}

void CGlobal::ChooseLane()
{
	if (m_lane == model::_LANE_UNKNOWN_)
	{
		int l = rand() % 3;
		switch (l)
		{
		case 0: m_lane = model::LANE_TOP; break;
		case 1: m_lane = model::LANE_MIDDLE; break;
		case 2: m_lane = model::LANE_BOTTOM; break;
		}
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

std::pair<double, double> CGlobal::GetWaypoint()
{
	if (m_lane == model::LANE_TOP)
	{
		if (m_top == LaneState::TOWER_1) return { (m_strategy.m_self->getY() < 400.0 ? m_T1.first : 200.0), m_T1.second };
		if (m_top == LaneState::TOWER_2) return { (m_strategy.m_self->getY() < 400.0 ? m_T2.first : 200.0), m_T2.second };
		if (m_top == LaneState::BASE) return { m_BS.first, m_BS.second };
	}
	else if (m_lane == model::LANE_MIDDLE)
	{
		if (m_mid == LaneState::TOWER_1) return { m_M1.first, m_M1.second };
		if (m_mid == LaneState::TOWER_2) return { m_M2.first, m_M2.second };
		if (m_mid == LaneState::BASE) return { m_BS.first, m_BS.second };
	}
	else if (m_lane == model::LANE_BOTTOM)
	{
		if (m_bot == LaneState::TOWER_1) return { m_B1.first, (m_strategy.m_self->getX() > m_strategy.m_game->getMapSize() - 400.0 ? m_B1.second : m_strategy.m_game->getMapSize() - 200.0) };
		if (m_bot == LaneState::TOWER_2) return { m_B2.first, (m_strategy.m_self->getX() > m_strategy.m_game->getMapSize() - 400.0 ? m_B2.second : m_strategy.m_game->getMapSize() - 200.0) };
		if (m_bot == LaneState::BASE) return { m_BS.first, m_BS.second };
	}
	return { m_BS.first, m_BS.second };
}

void CGlobal::Update()
{
	if (m_lane == model::LANE_TOP)
	{
		if (m_top == LaneState::TOWER_1 && m_strategy.m_self->getDistanceTo(m_T1.first, m_T1.second) <= m_strategy.m_game->getWizardVisionRange())
		{
			bool bFound = false;
			for (auto & unit : m_strategy.m_world->getBuildings())
			{
				if (unit.getFaction() == m_strategy.m_self->getFaction())
					continue;
				if (unit.getType() == model::BUILDING_FACTION_BASE)
					continue;
				if (m_T1.first - 0.1 < unit.getX() && unit.getX() < m_T1.first + 0.1 && m_T1.second - 0.1 < unit.getY() && unit.getY() < m_T1.second + 0.1)
				{
					bFound = true;
					break;
				}
			}
			if (!bFound)
				m_top = LaneState::TOWER_2;
		}
		if (m_top == LaneState::TOWER_2 && m_strategy.m_self->getDistanceTo(m_T2.first, m_T2.second) <= m_strategy.m_game->getWizardVisionRange())
		{
			bool bFound = false;
			for (auto & unit : m_strategy.m_world->getBuildings())
			{
				if (unit.getFaction() == m_strategy.m_self->getFaction())
					continue;
				if (unit.getType() == model::BUILDING_FACTION_BASE)
					continue;
				if (m_T2.first - 0.1 < unit.getX() && unit.getX() < m_T2.first + 0.1 && m_T2.second - 0.1 < unit.getY() && unit.getY() < m_T2.second + 0.1)
				{
					bFound = true;
					break;
				}
			}
			if (!bFound)
				m_top = LaneState::BASE;
		}
	}
	if (m_lane == model::LANE_MIDDLE)
	{
		if (m_mid == LaneState::TOWER_1 && m_strategy.m_self->getDistanceTo(m_M1.first, m_M1.second) <= m_strategy.m_game->getWizardVisionRange())
		{
			bool bFound = false;
			for (auto & unit : m_strategy.m_world->getBuildings())
			{
				if (unit.getFaction() == m_strategy.m_self->getFaction())
					continue;
				if (unit.getType() == model::BUILDING_FACTION_BASE)
					continue;
				if (m_M1.first - 0.1 < unit.getX() && unit.getX() < m_M1.first + 0.1 && m_M1.second - 0.1 < unit.getY() && unit.getY() < m_M1.second + 0.1)
				{
					bFound = true;
					break;
				}
			}
			if (!bFound)
				m_mid = LaneState::TOWER_2;
		}
		if (m_mid == LaneState::TOWER_2 && m_strategy.m_self->getDistanceTo(m_M2.first, m_M2.second) <= m_strategy.m_game->getWizardVisionRange())
		{
			bool bFound = false;
			for (auto & unit : m_strategy.m_world->getBuildings())
			{
				if (unit.getFaction() == m_strategy.m_self->getFaction())
					continue;
				if (unit.getType() == model::BUILDING_FACTION_BASE)
					continue;
				if (m_M2.first - 0.1 < unit.getX() && unit.getX() < m_M2.first + 0.1 && m_M2.second - 0.1 < unit.getY() && unit.getY() < m_M2.second + 0.1)
				{
					bFound = true;
					break;
				}
			}
			if (!bFound)
				m_mid = LaneState::BASE;
		}
	}
	if (m_lane == model::LANE_BOTTOM)
	{
		if (m_bot == LaneState::TOWER_1 && m_strategy.m_self->getDistanceTo(m_B1.first, m_B1.second) <= m_strategy.m_game->getWizardVisionRange())
		{
			bool bFound = false;
			for (auto & unit : m_strategy.m_world->getBuildings())
			{
				if (unit.getFaction() == m_strategy.m_self->getFaction())
					continue;
				if (unit.getType() == model::BUILDING_FACTION_BASE)
					continue;
				if (m_B1.first - 0.1 < unit.getX() && unit.getX() < m_B1.first + 0.1 && m_B1.second - 0.1 < unit.getY() && unit.getY() < m_B1.second + 0.1)
				{
					bFound = true;
					break;
				}
			}
			if (!bFound)
				m_bot = LaneState::TOWER_2;
		}
		if (m_bot == LaneState::TOWER_2 && m_strategy.m_self->getDistanceTo(m_B2.first, m_B2.second) <= m_strategy.m_game->getWizardVisionRange())
		{
			bool bFound = false;
			for (auto & unit : m_strategy.m_world->getBuildings())
			{
				if (unit.getFaction() == m_strategy.m_self->getFaction())
					continue;
				if (unit.getType() == model::BUILDING_FACTION_BASE)
					continue;
				if (m_B2.first - 0.1 < unit.getX() && unit.getX() < m_B2.first + 0.1 && m_B2.second - 0.1 < unit.getY() && unit.getY() < m_B2.second + 0.1)
				{
					bFound = true;
					break;
				}
			}
			if (!bFound)
				m_bot = LaneState::BASE;
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