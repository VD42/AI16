#include "MyStrategy.h"

#define PI 3.14159265358979323846
#define _USE_MATH_DEFINES

#include <cmath>
#include <cstdlib>
#include <time.h>
#include <limits>

using namespace model;

const model::Wizard MyStrategy::CState::tmp::s;
const model::World MyStrategy::CState::tmp::w;
const model::Game MyStrategy::CState::tmp::g;
model::Move MyStrategy::CState::tmp::m;

MyStrategy::MyStrategy() : m_state(CState::tmp::s, CState::tmp::w, CState::tmp::g, CState::tmp::m)
{
	srand((unsigned int)time(nullptr));
	game = nullptr;
}

void MyStrategy::Init(const model::Game & g)
{
	if (game)
		return;
	game = &g;

	double fMapSize = game->getMapSize();

	LaneType type = (LaneType)(rand() % 3);

	type = LANE_MIDDLE; // middle rush

	if (type == LANE_TOP)
	{
		m_tWaypoints = {
			{ 100.0, fMapSize - 100.0 },
			{ 100.0, fMapSize - 400.0 },
			{ 200.0, fMapSize - 800.0 },
			{ 200.0, fMapSize * 0.75 },
			{ 200.0, fMapSize * 0.5 },
			{ 200.0, fMapSize * 0.25},
			{ 200.0, 200.0 },
			{ fMapSize * 0.25, 200.0 },
			{ fMapSize * 0.5, 200.0 },
			{ fMapSize * 0.75, 200.0 },
			{ fMapSize - 200.0, 200.0 }
		};
	}
	else if (type == LANE_MIDDLE)
	{
		std::pair<double, double> second;
		if (rand() % 2)
			second = { 600.0, fMapSize - 200.0 };
		else
			second = { 200.0, fMapSize - 600.0 };
		/*m_tWaypoints = {
			{ 100.0, fMapSize - 100.0 },
			second,
			{ 800.0, fMapSize - 800.0 },
			{ fMapSize - 600.0, 600.0 }
		};*/
		m_tWaypoints = {
			{ 100.0, fMapSize - 100.0 },
			second,
			{ 800.0, fMapSize - 600.0 },
			{ 1000.0, fMapSize - 800.0 },
			{ 1200.0, fMapSize - 1000.0 },
			{ 1400.0, fMapSize - 1200.0 },
			{ 1600.0, fMapSize - 1400.0 },
			{ 1800.0, fMapSize - 1600.0 },
			{ 2000.0, fMapSize - 1800.0 },
			{ 2200.0, fMapSize - 2000.0 },
			{ 2400.0, fMapSize - 2600.0 },
			{ 2600.0, fMapSize - 2800.0 },
			{ 2800.0, fMapSize - 3000.0 },
			{ 3000.0, fMapSize - 3200.0 },
			{ fMapSize - 600.0, 600.0 }
		};
	}
	else if (type == LANE_BOTTOM)
	{
		m_tWaypoints = {
			{ 100.0, fMapSize - 100.0 },
			{ 400.0, fMapSize - 100.0 },
			{ 800.0, fMapSize - 200.0 },
			{ fMapSize * 0.25, fMapSize - 200.0 },
			{ fMapSize * 0.5, fMapSize - 200.0 },
			{ fMapSize * 0.75, fMapSize - 200.0 },
			{ fMapSize - 200.0, fMapSize - 200.0 },
			{ fMapSize - 200.0, fMapSize * 0.75 },
			{ fMapSize - 200.0, fMapSize * 0.5 },
			{ fMapSize - 200.0, fMapSize * 0.25 },
			{ fMapSize - 200.0, 200.0 }
		};
	}
}

void MyStrategy::move(const model::Wizard & self, const model::World & world, const model::Game & game, model::Move & move)
{
	Init(game);
	m_state = CState(self, world, game, move);
	Tick();
}

void MyStrategy::CheckBlock()
{
	static int nBlockCounter = 0;
	static std::pair<double, double> lastWaypoint;
	if (lastWaypoint == GetNextWaypoint())
		nBlockCounter++;
	else
		nBlockCounter = 0;
	lastWaypoint = GetNextWaypoint();
	if (nBlockCounter > 300)
		GoTo(GetPreviousWaypoint());
}

void MyStrategy::Tick()
{
	if (m_state.world.getTickIndex() < 333)
		return;

	m_state.move.setStrafeSpeed((rand() % 2) ? m_state.game.getWizardStrafeSpeed() : -m_state.game.getWizardStrafeSpeed());

	if (m_state.self.getLife() < m_state.self.getMaxLife() * 0.25)
	{
		GoTo(GetPreviousWaypoint());
		return;
	}

	const LivingUnit * pNearestTarget = GetNearestTarget();
	if (pNearestTarget)
	{
		double fDistance = m_state.self.getDistanceTo(*pNearestTarget);
		if (fDistance <= m_state.self.getCastRange())
		{
			double fAngle = m_state.self.getAngleTo(*pNearestTarget);
			m_state.move.setTurn(fAngle);
			if (std::abs(fAngle) < m_state.game.getStaffSector() / 2.0)
			{
				m_state.move.setAction(ACTION_MAGIC_MISSILE);
				m_state.move.setCastAngle(fAngle);
				m_state.move.setMinCastDistance(fDistance - pNearestTarget->getRadius() + m_state.game.getMagicMissileRadius());
			}
			return;
		}
	}

	GoTo(GetNextWaypoint());

	//CheckBlock();
}

const LivingUnit * MyStrategy::GetNearestTarget()
{
	const LivingUnit * pNearestTarget = nullptr;
	double fNearestTargetDistance = std::numeric_limits<double>::max();

	for (auto & target : m_state.world.getBuildings())
	{
		if (target.getFaction() == FACTION_NEUTRAL || target.getFaction() == m_state.self.getFaction())
			continue;
		double fDistance = m_state.self.getDistanceTo(target);
		if (fDistance < fNearestTargetDistance)
		{
			pNearestTarget = &target;
			fNearestTargetDistance = fDistance;
		}
	}

	for (auto & target : m_state.world.getWizards())
	{
		if (target.getFaction() == FACTION_NEUTRAL || target.getFaction() == m_state.self.getFaction())
			continue;
		double fDistance = m_state.self.getDistanceTo(target);
		if (fDistance < fNearestTargetDistance)
		{
			pNearestTarget = &target;
			fNearestTargetDistance = fDistance;
		}
	}

	for (auto & target : m_state.world.getMinions())
	{
		if (target.getFaction() == FACTION_NEUTRAL || target.getFaction() == m_state.self.getFaction())
			continue;
		double fDistance = m_state.self.getDistanceTo(target);
		if (fDistance < fNearestTargetDistance)
		{
			pNearestTarget = &target;
			fNearestTargetDistance = fDistance;
		}
	}

	return pNearestTarget;
}

void MyStrategy::GoTo(std::pair<double, double> point)
{
	double fAngle = m_state.self.getAngleTo(point.first, point.second);
	m_state.move.setTurn(fAngle);
	if (std::abs(fAngle) < m_state.game.getStaffSector() / 4.0)
		m_state.move.setSpeed(m_state.game.getWizardForwardSpeed());
}

double GetDistanceTo(std::pair<double, double> p1, std::pair<double, double> p2)
{
	double xRange = p1.first - p2.first;
	double yRange = p1.second - p2.second;
	return sqrt(xRange * xRange + yRange * yRange);
}

std::pair<double, double> MyStrategy::GetNextWaypoint()
{
	int nLastWaypointIndex = (int)m_tWaypoints.size() - 1;
	auto lastWaypoint = m_tWaypoints[nLastWaypointIndex];
	for (int i = 0; i < nLastWaypointIndex; i++)
	{
		auto waypoint = m_tWaypoints[i];
		if (m_state.self.getDistanceTo(waypoint.first, waypoint.second) <= 100.0)
			return m_tWaypoints[i + 1];
		if (GetDistanceTo(lastWaypoint, waypoint) < m_state.self.getDistanceTo(lastWaypoint.first, lastWaypoint.second))
			return waypoint;
	}
	return lastWaypoint;
}

std::pair<double, double> MyStrategy::GetPreviousWaypoint()
{
	auto firstWaypoint = m_tWaypoints[0];
	for (int i = (int)m_tWaypoints.size() - 1; i > 0; i--)
	{
		auto waypoint = m_tWaypoints[i];
		if (m_state.self.getDistanceTo(waypoint.first, waypoint.second) <= 100.0)
			return m_tWaypoints[i - 1];
		if (GetDistanceTo(firstWaypoint, waypoint) < m_state.self.getDistanceTo(firstWaypoint.first, firstWaypoint.second))
			return waypoint;
	}
	return firstWaypoint;
}
