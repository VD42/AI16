#include "MyStrategy_Settings.h"


const std::function<double(MyStrategy&, const model::Tree&)> CSettings::PW_TREE = [](MyStrategy & strategy, const model::Tree & unit)
{
	double BASE = PW_CIRCULAR_UNIT(strategy, unit);
	return BASE;
};


const std::function<double(MyStrategy&, const model::Minion&)> CSettings::PW_ENEMY_CREEP = [](MyStrategy & strategy, const model::Minion & unit)
{
	double BASE = PW_CIRCULAR_UNIT(strategy, unit);
	return BASE;
};

const std::function<double(MyStrategy&, const model::Building&)> CSettings::PW_ENEMY_TOWER = [](MyStrategy & strategy, const model::Building & unit)
{
	double BASE = PW_CIRCULAR_UNIT(strategy, unit);
	return BASE;
};

const std::function<double(MyStrategy&, const model::Wizard&)> CSettings::PW_ENEMY_WIZARD = [](MyStrategy & strategy, const model::Wizard & unit)
{
	double BASE = PW_CIRCULAR_UNIT(strategy, unit);
	return BASE;
};

const std::function<double(MyStrategy&, const model::Building&)> CSettings::PW_ENEMY_BASE = [](MyStrategy & strategy, const model::Building & unit)
{
	double BASE = PW_CIRCULAR_UNIT(strategy, unit);
	return BASE;
};


const std::function<double(MyStrategy&, const model::Minion&)> CSettings::PW_FRIENDLY_CREEP = [](MyStrategy & strategy, const model::Minion & unit)
{
	double BASE = PW_CIRCULAR_UNIT(strategy, unit);
	return BASE;
};

const std::function<double(MyStrategy&, const model::Building&)> CSettings::PW_FRIENDLY_TOWER = [](MyStrategy & strategy, const model::Building & unit)
{
	double BASE = PW_CIRCULAR_UNIT(strategy, unit);
	return BASE;
};

const std::function<double(MyStrategy&, const model::Wizard&)> CSettings::PW_FRIENDLY_WIZARD = [](MyStrategy & strategy, const model::Wizard & unit)
{
	double BASE = PW_CIRCULAR_UNIT(strategy, unit);
	return BASE;
};

const std::function<double(MyStrategy&, const model::Building&)> CSettings::PW_FRIENDLY_BASE = [](MyStrategy & strategy, const model::Building & unit)
{
	double BASE = PW_CIRCULAR_UNIT(strategy, unit);
	return BASE;
};


const std::function<double(MyStrategy&, const model::Minion&)> CSettings::PW_NEUTRAL_CREEP = [](MyStrategy & strategy, const model::Minion & unit)
{
	double BASE = PW_CIRCULAR_UNIT(strategy, unit);
	return BASE;
};


const std::function<double(MyStrategy&, const model::CircularUnit&)> CSettings::PW_CIRCULAR_UNIT = [](MyStrategy & strategy, const model::CircularUnit & unit)
{
	double R1 = strategy.m_self->getRadius();
	double R2 = unit.getRadius();
	double D = std::hypot(strategy.m_self->getX() - unit.getX(), strategy.m_self->getY() - unit.getY());
	if (D - R1 - R2 < 10.0)
		return -1.0;
	if (D - R1 - R2 < 5.0)
		return -100.0;
	if (D - R1 - R2 < 2.0)
		return -10000.0;
	if (D - R1 - R2 < 1.0)
		return -1000000.0;
	double PW = 1.0 / ((D - R1 - R2) * (D - R1 - R2) + 0.0001);
	return -PW;
};
