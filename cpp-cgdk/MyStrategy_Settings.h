#pragma once

#include <functional>
#include "MyStrategy.h"


class CSettings
{
public:
	static const std::function<double(MyStrategy&, const model::Tree&)> PW_TREE;

	static const std::function<double(MyStrategy&, const model::Minion&)> PW_ENEMY_CREEP;
	static const std::function<double(MyStrategy&, const model::Building&)> PW_ENEMY_TOWER;
	static const std::function<double(MyStrategy&, const model::Wizard&)> PW_ENEMY_WIZARD;
	static const std::function<double(MyStrategy&, const model::Building&)> PW_ENEMY_BASE;

	static const std::function<double(MyStrategy&, const model::Minion&)> PW_FRIENDLY_CREEP;
	static const std::function<double(MyStrategy&, const model::Building&)> PW_FRIENDLY_TOWER;
	static const std::function<double(MyStrategy&, const model::Wizard&)> PW_FRIENDLY_WIZARD;
	static const std::function<double(MyStrategy&, const model::Building&)> PW_FRIENDLY_BASE;

	static const std::function<double(MyStrategy&, const model::Minion&)> PW_NEUTRAL_CREEP;

	static const std::function<double(MyStrategy&, const model::CircularUnit&)> PW_CIRCULAR_UNIT;
};