#include "MyStrategy_Global.h"
#include "MyStrategy.h"
#include <algorithm>
#include "MyStrategy_Settings.h"


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

model::LaneType CGlobal::GetLane(const model::CircularUnit & unit)
{
	if (unit.getDistanceTo(1200.0, 1200.0) < 300.0)
		return model::_LANE_UNKNOWN_;
	if (unit.getDistanceTo(2800.0, 2800.0) < 300.0)
		return model::_LANE_UNKNOWN_;

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
		if (m_bEgoistMode)
		{
			m_lane = model::LANE_MIDDLE;
			m_bLaneChoosed = true;
			goto gt;
		}

		if (!MasterControl())
		{
			m_lane = model::LANE_MIDDLE;
			m_bLaneChoosed = true;
			goto gt;
		}

		if (!m_bEgoistMode && MasterControl())
		{
			if (m_bIsFinal)
			{
				m_strategy.m_tSkillsOrder = CSettings::GET_SKILLS_ORDER_FOR_SKILL(m_strategy, model::SKILL_FIREBALL);
				std::reverse(m_strategy.m_tSkillsOrder.begin(), m_strategy.m_tSkillsOrder.end());
				m_bLaneRush = true;
				m_lane = model::LANE_MIDDLE;
				m_bLaneChoosed = true;
				m_strategy.m_nLocalId = 1;
				std::vector<model::Message> m_tMessages = {
					model::Message(model::LANE_MIDDLE, model::SKILL_SHIELD, { 2 }),
					model::Message(model::LANE_MIDDLE, model::SKILL_FROST_BOLT, { 3 }),
					model::Message(model::LANE_MIDDLE, model::SKILL_FIREBALL, { 4 }),
					model::Message(model::LANE_MIDDLE, model::SKILL_FIREBALL, { 5 })
				};
				m_strategy.m_move->setMessages(m_tMessages);
			}
			else
			{
				m_strategy.m_tSkillsOrder = CSettings::GET_SKILLS_ORDER(m_strategy);
				std::reverse(m_strategy.m_tSkillsOrder.begin(), m_strategy.m_tSkillsOrder.end());
				m_lane = model::LANE_MIDDLE;
				m_bLaneChoosed = true;
				std::vector<model::Message> m_tMessages = {
					model::Message(model::LANE_TOP, model::_SKILL_UNKNOWN_, {}),
					model::Message(model::LANE_TOP, model::_SKILL_UNKNOWN_, {}),
					model::Message(model::LANE_BOTTOM, model::_SKILL_UNKNOWN_, {}),
					model::Message(model::LANE_BOTTOM, model::_SKILL_UNKNOWN_, {})
				};
				m_strategy.m_move->setMessages(m_tMessages);
			}
			goto gt;
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

gt:
	if (!m_bEgoistMode && m_bEnableMasterHeard)
	{
		for (auto & message : m_strategy.m_self->getMessages())
		{
			if (message.getLane() == model::_LANE_UNKNOWN_)
				continue;
			m_bMasterNotSilent = true;
			m_lane = message.getLane();
			m_bLaneChoosed = true;
			if (message.getSkillToLearn() != model::_SKILL_UNKNOWN_)
			{
				m_strategy.m_tSkillsOrder = CSettings::GET_SKILLS_ORDER_FOR_SKILL(m_strategy, message.getSkillToLearn());
				std::reverse(m_strategy.m_tSkillsOrder.begin(), m_strategy.m_tSkillsOrder.end());
			}
			if ((int)message.getRawMessage().size() > 0)
				m_strategy.m_nLocalId = message.getRawMessage()[0];
		}
	}

	if (m_bLaneChoosed)
		return;
}

std::pair<std::pair<double, double>, bool> CGlobal::GetWaypoint()
{
	const model::Building * base = nullptr;
	for (auto & building : m_strategy.m_world->getBuildings())
	{
		if (building.getType() == model::BUILDING_FACTION_BASE && building.getFaction() == m_strategy.m_self->getFaction())
		{
			base = &building;
			break;
		}
	}

	std::pair<std::pair<double, double>, bool> result = { { 0.0, 0.0 }, false };
	double distance = 40000.0;

	if (!base)
		return result;

	int nCloseToBase = 0;
	for (auto & wizard : m_strategy.m_world->getWizards())
	{
		if (wizard.getFaction() != m_strategy.m_self->getFaction())
			continue;
		if (wizard.isMe())
			continue;
		if (wizard.getDistanceTo(m_BS.first, m_BS.second) < 1350.0)
			nCloseToBase++;
	}

	if (m_lane == model::LANE_TOP)
	{
		if (m_top == LaneState::TOWER_1)
		{
			if (base->getDistanceTo(m_T1.first, m_T1.second) < distance)
			{
				result.first = { m_T1.first, m_T1.second };
				result.second = false;
				distance = base->getDistanceTo(m_T1.first, m_T1.second);
			}
		}
		if (m_top == LaneState::TOWER_2)
		{
			if (base->getDistanceTo(m_T2.first, m_T2.second) < distance)
			{
				result.first = { m_T2.first, m_T2.second };
				result.second = false;
				distance = base->getDistanceTo(m_T2.first, m_T2.second);
			}
		}
		if (m_top == LaneState::BASE)
		{
			double baseX = m_BS.first - 200.0;
			double baseY = m_BS.second - 150.0;
			bool priority = false;

			if (nCloseToBase < 2 && (m_strategy.m_world->getTickIndex() - 1) % 750 > 550)
			{
				baseX = m_BS.first - 800.0;
				priority = true;
			}

			if (base->getDistanceTo(baseX, baseY) < distance)
			{
				result.first = { baseX, baseY };
				result.second = priority;
				distance = base->getDistanceTo(baseX, baseY);
			}
		}
		for (auto & minion : m_strategy.m_world->getMinions())
		{
			if (minion.getFaction() == m_strategy.m_self->getFaction() || minion.getFaction() == model::FACTION_NEUTRAL)
				continue;
			if (GetLane(minion) != model::LANE_TOP)
				continue;
			if (base->getDistanceTo(minion.getX(), minion.getY()) < distance)
			{
				result.first = { minion.getX(), minion.getY() };
				result.second = false;
				distance = base->getDistanceTo(minion.getX(), minion.getY());
			}
		}
		for (auto & wizard : m_strategy.m_world->getWizards())
		{
			if (wizard.getFaction() == m_strategy.m_self->getFaction())
				continue;
			if (GetLane(wizard) != model::LANE_TOP)
				continue;
			if (base->getDistanceTo(wizard.getX(), wizard.getY()) < distance)
			{
				result.first = { wizard.getX(), wizard.getY() };
				result.second = false;
				distance = base->getDistanceTo(wizard.getX(), wizard.getY());
			}
		}
		if (m_strategy.m_self->getX() < m_strategy.m_self->getY() - 400.0 && result.first.first > result.first.second + 400.0)
			return { { 250.0, 250.0 }, result.second };
		else
			return { { result.first.first, result.first.second }, result.second };
	}
	else if (m_lane == model::LANE_MIDDLE)
	{
		if (m_mid == LaneState::TOWER_1)
		{
			if (base->getDistanceTo(m_M1.first, m_M1.second) < distance)
			{
				result.first = { m_M1.first, m_M1.second };
				result.second = false;
				distance = base->getDistanceTo(m_M1.first, m_M1.second);
			}
		}
		if (m_mid == LaneState::TOWER_2)
		{
			if (base->getDistanceTo(m_M2.first, m_M2.second) < distance)
			{
				result.first = { m_M2.first, m_M2.second };
				result.second = false;
				distance = base->getDistanceTo(m_M2.first, m_M2.second);
			}
		}
		if (m_mid == LaneState::BASE)
		{
			double baseX = m_BS.first - 200.0;
			double baseY = m_BS.second + 200.0;
			bool priority = false;

			if (nCloseToBase < 2 && (m_strategy.m_world->getTickIndex() - 1) % 750 > 550)
			{
				baseX = m_BS.first - 800.0;
				baseY = m_BS.second + 800.0;
				priority = true;
			}

			if (base->getDistanceTo(baseX, baseY) < distance)
			{
				result.first = { baseX, baseY };
				result.second = priority;
				distance = base->getDistanceTo(baseX, baseY);
			}
		}
		for (auto & minion : m_strategy.m_world->getMinions())
		{
			if (minion.getFaction() == m_strategy.m_self->getFaction() || minion.getFaction() == model::FACTION_NEUTRAL)
				continue;
			if (GetLane(minion) != model::LANE_MIDDLE)
				continue;
			if (base->getDistanceTo(minion.getX(), minion.getY()) < distance)
			{
				result.first = { minion.getX(), minion.getY() };
				result.second = false;
				distance = base->getDistanceTo(minion.getX(), minion.getY());
			}
		}
		for (auto & wizard : m_strategy.m_world->getWizards())
		{
			if (wizard.getFaction() == m_strategy.m_self->getFaction())
				continue;
			if (GetLane(wizard) != model::LANE_MIDDLE)
				continue;
			if (base->getDistanceTo(wizard.getX(), wizard.getY()) < distance)
			{
				result.first = { wizard.getX(), wizard.getY() };
				result.second = false;
				distance = base->getDistanceTo(wizard.getX(), wizard.getY());
			}
		}
		if (m_strategy.m_self->getX() < m_strategy.m_self->getY() - 400.0 && result.first.first > result.first.second + 400.0)
			return { { 2000.0, 2000.0 }, result.second };
		else
			return { { result.first.first, result.first.second }, result.second };
	}
	else if (m_lane == model::LANE_BOTTOM)
	{
		if (m_bot == LaneState::TOWER_1)
		{
			if (base->getDistanceTo(m_B1.first, m_B1.second) < distance)
			{
				result.first = { m_B1.first, m_B1.second };
				result.second = false;
				distance = base->getDistanceTo(m_B1.first, m_B1.second);
			}
		}
		if (m_bot == LaneState::TOWER_2)
		{
			if (base->getDistanceTo(m_B2.first, m_B2.second) < distance)
			{
				result.first = { m_B2.first, m_B2.second };
				result.second = false;
				distance = base->getDistanceTo(m_B2.first, m_B2.second);
			}
		}
		if (m_bot == LaneState::BASE)
		{
			double baseX = m_BS.first + 250.0;
			double baseY = m_BS.second + 200.0;
			bool priority = false;

			if (nCloseToBase < 2 && (m_strategy.m_world->getTickIndex() - 1) % 750 > 550)
			{
				baseY = m_BS.second + 800.0;
				priority = true;
			}

			if (base->getDistanceTo(baseX, baseY) < distance)
			{
				result.first = { baseX, baseY };
				result.second = priority;
				distance = base->getDistanceTo(baseX, baseY);
			}
		}
		for (auto & minion : m_strategy.m_world->getMinions())
		{
			if (minion.getFaction() == m_strategy.m_self->getFaction() || minion.getFaction() == model::FACTION_NEUTRAL)
				continue;
			if (GetLane(minion) != model::LANE_BOTTOM)
				continue;
			if (base->getDistanceTo(minion.getX(), minion.getY()) < distance)
			{
				result.first = { minion.getX(), minion.getY() };
				result.second = false;
				distance = base->getDistanceTo(minion.getX(), minion.getY());
			}
		}
		for (auto & wizard : m_strategy.m_world->getWizards())
		{
			if (wizard.getFaction() == m_strategy.m_self->getFaction())
				continue;
			if (GetLane(wizard) != model::LANE_BOTTOM)
				continue;
			if (base->getDistanceTo(wizard.getX(), wizard.getY()) < distance)
			{
				result.first = { wizard.getX(), wizard.getY() };
				result.second = false;
				distance = base->getDistanceTo(wizard.getX(), wizard.getY());
			}
		}
		if (m_strategy.m_self->getX() < m_strategy.m_self->getY() - 400.0 && result.first.first > result.first.second + 400.0)
			return{ { m_strategy.m_game->getMapSize() - 250.0, m_strategy.m_game->getMapSize() - 250.0 }, result.second };
		else
			return { { result.first.first, result.first.second }, result.second };
	}
	return result;
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
		if (MasterControl() && !m_bLaneRush && m_strategy.m_world->getTickIndex() < 10000 && Tower2Exists())
		{
			m_lane = model::LANE_TOP;
			m_bLaneChoosed = true;
			m_bLaneRush = true;
			std::vector<model::Message> m_tMessages = {
				model::Message(model::LANE_TOP, model::_SKILL_UNKNOWN_, std::vector<signed char>()),
				model::Message(model::LANE_TOP, model::_SKILL_UNKNOWN_, std::vector<signed char>()),
				model::Message(model::LANE_TOP, model::_SKILL_UNKNOWN_, std::vector<signed char>()),
				model::Message(model::LANE_TOP, model::_SKILL_UNKNOWN_, std::vector<signed char>())
			};
			m_strategy.m_move->setMessages(m_tMessages);
		}
	}

	if (m_mid == LaneState::TOWER_1 && EnemyTowerNotExists(m_M1))
	{
		m_mid = LaneState::TOWER_2;
	}
	if (m_mid == LaneState::TOWER_2 && EnemyTowerNotExists(m_M2))
	{
		m_mid = LaneState::BASE;
		if (MasterControl() && !m_bLaneRush && m_strategy.m_world->getTickIndex() < 10000 && Tower2Exists())
		{
			m_lane = model::LANE_MIDDLE;
			m_bLaneChoosed = true;
			m_bLaneRush = true;
			std::vector<model::Message> m_tMessages = {
				model::Message(model::LANE_MIDDLE, model::_SKILL_UNKNOWN_, std::vector<signed char>()),
				model::Message(model::LANE_MIDDLE, model::_SKILL_UNKNOWN_, std::vector<signed char>()),
				model::Message(model::LANE_MIDDLE, model::_SKILL_UNKNOWN_, std::vector<signed char>()),
				model::Message(model::LANE_MIDDLE, model::_SKILL_UNKNOWN_, std::vector<signed char>())
			};
			m_strategy.m_move->setMessages(m_tMessages);
		}
	}

	if (m_bot == LaneState::TOWER_1 && EnemyTowerNotExists(m_B1))
	{
		m_bot = LaneState::TOWER_2;
	}
	if (m_bot == LaneState::TOWER_2 && EnemyTowerNotExists(m_B2))
	{
		m_bot = LaneState::BASE;
		if (MasterControl() && !m_bLaneRush && m_strategy.m_world->getTickIndex() < 10000 && Tower2Exists())
		{
			m_lane = model::LANE_BOTTOM;
			m_bLaneChoosed = true;
			m_bLaneRush = true;
			std::vector<model::Message> m_tMessages = {
				model::Message(model::LANE_BOTTOM, model::_SKILL_UNKNOWN_, std::vector<signed char>()),
				model::Message(model::LANE_BOTTOM, model::_SKILL_UNKNOWN_, std::vector<signed char>()),
				model::Message(model::LANE_BOTTOM, model::_SKILL_UNKNOWN_, std::vector<signed char>()),
				model::Message(model::LANE_BOTTOM, model::_SKILL_UNKNOWN_, std::vector<signed char>())
			};
			m_strategy.m_move->setMessages(m_tMessages);
		}
	}

	if (m_bBonusT)
	{
		if (m_strategy.m_world->getTickIndex() - m_bBonusTStart >= 2500)
			m_bBonusT = false;

		if (m_bBonusT && (m_strategy.m_world->getTickIndex() - 1) / 2500 > (m_bBonusTStart - 1) / 2500)
		{
			if (BonusNotExists(std::make_pair(1200.0, 1200.0)))
				m_bBonusT = false;
		}
	}
	if (m_bBonusB)
	{
		if (m_strategy.m_world->getTickIndex() - m_bBonusBStart >= 2500)
			m_bBonusB = false;

		if (m_bBonusB && (m_strategy.m_world->getTickIndex() - 1) / 2500 > (m_bBonusBStart - 1) / 2500)
		{
			if (BonusNotExists(std::make_pair(2800.0, 2800.0)))
				m_bBonusB = false;
		}
	}

	if (!m_bBonusT && (m_strategy.m_world->getTickIndex() - 1) % 2500 >= (2500 - std::min(std::min((int)(m_strategy.m_self->getDistanceTo(1200.0, 1200.0) / 3.0), (int)(m_strategy.m_self->getDistanceTo(2800.0, 2800.0) / 3.0)), 2000)) && m_strategy.m_world->getTickIndex() < 17990)
	{
		m_bBonusT = true;
		m_bBonusTStart = m_strategy.m_world->getTickIndex();
	}
	if (!m_bBonusB && (m_strategy.m_world->getTickIndex() - 1) % 2500 >= (2500 - std::min(std::min((int)(m_strategy.m_self->getDistanceTo(2800.0, 2800.0) / 3.0), (int)(m_strategy.m_self->getDistanceTo(1200.0, 1200.0) / 3.0)), 2000)) && m_strategy.m_world->getTickIndex() < 17990)
	{
		m_bBonusB = true;
		m_bBonusBStart = m_strategy.m_world->getTickIndex();
	}

	for (auto & wizard : m_strategy.m_world->getWizards())
	{
		if (wizard.getFaction() == m_strategy.m_self->getFaction())
		{
			if (wizard.isMe())
			{
				m_mapFriendlyWizardsLane[wizard.getId()] = m_lane;
				continue;
			}
			if (GetLane(wizard) != model::_LANE_UNKNOWN_)
				m_mapFriendlyWizardsLane[wizard.getId()] = GetLane(wizard);
		}
		else
		{
			if (GetLane(wizard) != model::_LANE_UNKNOWN_)
				m_mapEnemyWizardsLane[wizard.getId()] = GetLane(wizard);
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
	if (m_bEgoistMode)
		return true;
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

bool CGlobal::DangerLane()
{
	int nTopWizards = 0;
	int nMidWizards = 0;
	int nBotWizards = 0;
	int nTopWizardsEnemy = 0;
	int nMidWizardsEnemy = 0;
	int nBotWizardsEnemy = 0;

	for (auto & lane : m_mapFriendlyWizardsLane)
	{
		if (lane.second == model::LANE_TOP)
			nTopWizards++;
		else if (lane.second == model::LANE_MIDDLE)
			nMidWizards++;
		else if (lane.second == model::LANE_BOTTOM)
			nBotWizards++;
	}

	for (auto & lane : m_mapEnemyWizardsLane)
	{
		if (lane.second == model::LANE_TOP)
			nTopWizardsEnemy++;
		else if (lane.second == model::LANE_MIDDLE)
			nMidWizardsEnemy++;
		else if (lane.second == model::LANE_BOTTOM)
			nBotWizardsEnemy++;
	}

	if (m_lane == model::LANE_TOP && nTopWizardsEnemy > 2)
		return true;
	if (m_lane == model::LANE_MIDDLE && nMidWizardsEnemy > 2)
		return true;
	if (m_lane == model::LANE_BOTTOM && nBotWizardsEnemy > 2)
		return true;
	return false;
}

void CGlobal::ReCheckLane(bool after_death)
{
	if (!OwnLaneControl() || m_bLaneRush)
		return;

	int nTopWizards = 0;
	int nMidWizards = 0;
	int nBotWizards = 0;
	int nTopWizardsEnemy = 0;
	int nMidWizardsEnemy = 0;
	int nBotWizardsEnemy = 0;

	for (auto & lane : m_mapFriendlyWizardsLane)
	{
		if (lane.second == model::LANE_TOP)
			nTopWizards++;
		else if (lane.second == model::LANE_MIDDLE)
			nMidWizards++;
		else if (lane.second == model::LANE_BOTTOM)
			nBotWizards++;
	}

	for (auto & lane : m_mapEnemyWizardsLane)
	{
		if (lane.second == model::LANE_TOP)
			nTopWizardsEnemy++;
		else if (lane.second == model::LANE_MIDDLE)
			nMidWizardsEnemy++;
		else if (lane.second == model::LANE_BOTTOM)
			nBotWizardsEnemy++;
	}

	printf("FW: T - %d / M - %d / B - %d\r\n", nTopWizards, nMidWizards, nBotWizards);
	printf("EW: T - %d / M - %d / B - %d\r\n", nTopWizardsEnemy, nMidWizardsEnemy, nBotWizardsEnemy);

	if (m_bEgoistMode)
	{
		if (m_strategy.m_self->getDistanceTo(m_BS.first, m_BS.second) < 1000.0)
			return;

		if (m_lane == model::LANE_TOP)
		{
			if (nMidWizardsEnemy < nTopWizardsEnemy)
			{
				m_lane = model::LANE_MIDDLE;
				m_bLaneChoosed = true;
				return;
			}
			if(after_death && nBotWizardsEnemy < nTopWizardsEnemy)
			{
				m_lane = model::LANE_BOTTOM;
				m_bLaneChoosed = true;
				return;
			}
		}
		if (m_lane == model::LANE_MIDDLE)
		{
			if (nTopWizardsEnemy < nMidWizardsEnemy)
			{
				m_lane = model::LANE_TOP;
				m_bLaneChoosed = true;
				return;
			}
			if (nBotWizardsEnemy < nMidWizardsEnemy)
			{
				m_lane = model::LANE_BOTTOM;
				m_bLaneChoosed = true;
				return;
			}
		}
		if (m_lane == model::LANE_BOTTOM)
		{
			if (nMidWizardsEnemy < nBotWizardsEnemy)
			{
				m_lane = model::LANE_MIDDLE;
				m_bLaneChoosed = true;
				return;
			}
			if (after_death && nTopWizardsEnemy < nBotWizardsEnemy)
			{
				m_lane = model::LANE_TOP;
				m_bLaneChoosed = true;
				return;
			}
		}
		m_bLaneChoosed = true;
		return;
	}

	if (MasterControl())
	{
		for (auto & wizard : m_mapFriendlyWizardsLane)
		{
			if (wizard.second == model::LANE_TOP && nTopWizardsEnemy < nTopWizards && nMidWizardsEnemy > nMidWizards)
			{
				wizard.second = model::LANE_MIDDLE;
				nTopWizards--;
				nMidWizards++;
			}
			else if (wizard.second == model::LANE_BOTTOM && nBotWizardsEnemy < nBotWizards && nMidWizardsEnemy > nMidWizards)
			{
				wizard.second = model::LANE_MIDDLE;
				nBotWizards--;
				nMidWizards++;
			}
			else if (wizard.second == model::LANE_MIDDLE && nMidWizardsEnemy < nMidWizards)
			{
				if (nTopWizardsEnemy > nTopWizards)
				{
					wizard.second = model::LANE_TOP;
					nMidWizards--;
					nTopWizards++;
				}
				else if (nBotWizardsEnemy > nBotWizards)
				{
					wizard.second = model::LANE_BOTTOM;
					nMidWizards--;
					nBotWizards++;
				}
			}
		}

		std::vector<model::Message> m_tMessages;

		std::vector<std::pair<long long, model::LaneType>> tOrder;

		for (auto & wizard : m_mapFriendlyWizardsLane)
			tOrder.push_back(std::make_pair(wizard.first, wizard.second));
		std::sort(tOrder.begin(), tOrder.end(), [](const std::pair<long long, model::LaneType> & a, const std::pair<long long, model::LaneType> & b) { return a.first < b.first; });

		for (auto & wizard : tOrder)
		{
			if (wizard.first == m_strategy.m_self->getId())
				m_lane = wizard.second;
			else
				m_tMessages.push_back(model::Message(wizard.second, model::_SKILL_UNKNOWN_, std::vector<signed char>()));
		}

		m_strategy.m_move->setMessages(m_tMessages);

		m_bLaneChoosed = true;

		return;
	}

	if (nTopWizardsEnemy + nMidWizardsEnemy + nBotWizardsEnemy == 5)
	{
		if (m_lane == model::LANE_TOP)
		{
			if (nTopWizards > nTopWizardsEnemy)
			{
				if (nMidWizards < nMidWizardsEnemy)
					m_lane = model::LANE_MIDDLE;
			}
			m_bLaneChoosed = true;
			return;
		}

		if (m_lane == model::LANE_MIDDLE)
		{
			if (nMidWizards > nMidWizardsEnemy)
			{
				if (nTopWizards < nTopWizardsEnemy)
					m_lane = model::LANE_TOP;
				else if (nBotWizards < nBotWizardsEnemy)
					m_lane = model::LANE_BOTTOM;
			}
			m_bLaneChoosed = true;
			return;
		}

		if (m_lane == model::LANE_BOTTOM)
		{
			if (nBotWizards > nBotWizardsEnemy)
			{
				if (nMidWizards < nMidWizardsEnemy)
					m_lane = model::LANE_MIDDLE;
			}
			m_bLaneChoosed = true;
			return;
		}
	}

	if (nTopWizards == 0)
	{
		if (m_lane == model::LANE_TOP)
		{
			m_bLaneChoosed = true;
			return;
		}

		if (m_lane == model::LANE_MIDDLE && nMidWizards > 1 && nMidWizards >= nMidWizardsEnemy)
		{
			m_lane = model::LANE_TOP;
			m_bLaneChoosed = true;
			return;
		}
	}

	if (nMidWizards == 0)
	{
		if (m_lane == model::LANE_MIDDLE)
		{
			m_bLaneChoosed = true;
			return;
		}

		if (m_lane == model::LANE_TOP && nTopWizards > 1 && nTopWizards >= nTopWizardsEnemy)
		{
			m_lane = model::LANE_MIDDLE;
			m_bLaneChoosed = true;
			return;
		}

		if (m_lane == model::LANE_BOTTOM && nBotWizards > 1 && nBotWizards >= nBotWizardsEnemy)
		{
			m_lane = model::LANE_MIDDLE;
			m_bLaneChoosed = true;
			return;
		}
	}

	if (nBotWizards == 0)
	{
		if (m_lane == model::LANE_BOTTOM)
		{
			m_bLaneChoosed = true;
			return;
		}

		if (m_lane == model::LANE_MIDDLE && nMidWizards > 1 && nMidWizards >= nMidWizardsEnemy)
		{
			m_lane = model::LANE_BOTTOM;
			m_bLaneChoosed = true;
			return;
		}
	}

	if (nTopWizards >= 3 && m_lane == model::LANE_TOP)
	{
		if (nMidWizards < nBotWizards)
			m_lane = model::LANE_MIDDLE;
	}
	if (nMidWizards >= 3 && m_lane == model::LANE_MIDDLE)
	{
		if (nBotWizards < nTopWizards)
			m_lane = model::LANE_BOTTOM;
		else
			m_lane = model::LANE_TOP;
	}
	if (nBotWizards >= 3 && m_lane == model::LANE_BOTTOM)
	{
		if (nMidWizards < nTopWizards)
			m_lane = model::LANE_MIDDLE;
	}

	m_bLaneChoosed = true;
}

int CGlobal::SpeedLevel(const model::Wizard & wizard)
{
	int nSpeedLevel = 0;
	for (auto & skill : wizard.getSkills())
	{
		if (skill == model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_1)
			nSpeedLevel++;
		if (skill == model::SKILL_MOVEMENT_BONUS_FACTOR_PASSIVE_2)
			nSpeedLevel++;
	}
	int nAuraMax = 0;
	for (auto & w : m_strategy.m_world->getWizards())
	{
		if (w.getFaction() != wizard.getFaction())
			continue;
		if (w.getDistanceTo(wizard) > m_strategy.m_game->getAuraSkillRange())
			continue;
		for (auto & skill : w.getSkills())
		{
			if (skill == model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_1)
				nAuraMax = 1;
			if (skill == model::SKILL_MOVEMENT_BONUS_FACTOR_AURA_2)
				nAuraMax = 2;
		}
	}
	nSpeedLevel += nAuraMax;
	return nSpeedLevel;
}

int CGlobal::RangeLevel(const model::Wizard & wizard)
{
	int nRangeLevel = 0;
	for (auto & skill : wizard.getSkills())
	{
		if (skill == model::SKILL_RANGE_BONUS_PASSIVE_1)
			nRangeLevel++;
		if (skill == model::SKILL_RANGE_BONUS_PASSIVE_2)
			nRangeLevel++;
	}
	int nAuraMax = 0;
	for (auto & w : m_strategy.m_world->getWizards())
	{
		if (w.getFaction() != wizard.getFaction())
			continue;
		if (w.getDistanceTo(wizard) > m_strategy.m_game->getAuraSkillRange())
			continue;
		for (auto & skill : w.getSkills())
		{
			if (skill == model::SKILL_RANGE_BONUS_AURA_1)
				nAuraMax = 1;
			if (skill == model::SKILL_RANGE_BONUS_AURA_2)
				nAuraMax = 2;
		}
	}
	nRangeLevel += nAuraMax;
	return nRangeLevel;
}

bool CGlobal::CanGoToBonus()
{
	if (m_bLaneRush)
		return false;

	int nWizards = 0;
	int nWizardsEnemy = 0;

	for (auto & lane : m_mapFriendlyWizardsLane)
	{
		if (lane.second == m_lane)
			nWizards++;
	}

	for (auto & lane : m_mapEnemyWizardsLane)
	{
		if (lane.second == m_lane)
			nWizardsEnemy++;
	}

	return (nWizardsEnemy <= nWizards);
}

bool CGlobal::MasterControl()
{
	if (!m_bEnableMasterControl)
		return false;
	if (!m_strategy.m_self->isMaster())
		return false;
	return true;
}

int CGlobal::LaneAdvantage()
{
	int nWizards = 0;
	int nWizardsEnemy = 0;

	for (auto & lane : m_mapFriendlyWizardsLane)
	{
		if (lane.second == m_lane)
			nWizards++;
	}

	for (auto & lane : m_mapEnemyWizardsLane)
	{
		if (lane.second == m_lane)
			nWizardsEnemy++;
	}

	return (nWizards - nWizardsEnemy);
}