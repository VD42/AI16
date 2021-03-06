#pragma once

#include <functional>
#include "MyStrategy.h"


class CSettings
{
public:
	static const std::function<double(MyStrategy&, const model::Tree&)> PW_TREE;

	static const std::function<double(MyStrategy&, const model::Minion&)> PW_ENEMY_CREEP_ORC;
	static const std::function<double(MyStrategy&, const model::Minion&)> PW_ENEMY_CREEP_FETISH;
	static const std::function<double(MyStrategy&, const model::Building&)> PW_ENEMY_TOWER;
	static const std::function<double(MyStrategy&, const model::Wizard&)> PW_ENEMY_WIZARD;
	static const std::function<double(MyStrategy&, const model::Building&)> PW_ENEMY_BASE;

	static const std::function<double(MyStrategy&, const model::Minion&)> PW_FRIENDLY_CREEP;
	static const std::function<double(MyStrategy&, const model::Minion&)> PW_FRIENDLY_CREEP_ORC;
	static const std::function<double(MyStrategy&, const model::Minion&)> PW_FRIENDLY_CREEP_FETISH;
	static const std::function<double(MyStrategy&, const model::Building&)> PW_FRIENDLY_TOWER;
	static const std::function<double(MyStrategy&, const model::Wizard&)> PW_FRIENDLY_WIZARD;
	static const std::function<double(MyStrategy&, const model::Building&)> PW_FRIENDLY_BASE;

	static const std::function<double(MyStrategy&, const model::Minion&)> PW_NEUTRAL_CREEP_ORC;
	static const std::function<double(MyStrategy&, const model::Minion&)> PW_NEUTRAL_CREEP_FETISH;

	static const std::function<double(MyStrategy&, const model::CircularUnit&)> PW_CIRCULAR_UNIT;

	static const std::function<double(MyStrategy&, const model::Projectile&)> PW_PROJECTILE;

	static bool HAVE_EMPOWER(MyStrategy & strategy, const model::Wizard & wizard);
	static bool HAVE_SHIELD(MyStrategy & strategy, const model::Wizard & wizard);
	static bool HAVE_HASTE(MyStrategy & strategy, const model::Wizard & wizard);

	static const std::vector<model::SkillType> GET_SKILLS_ORDER(MyStrategy & strategy);
	static const std::vector<model::SkillType> GET_SKILLS_ORDER_FOR_SKILL(MyStrategy & strategy, model::SkillType skill);

	static double RANGE_WINDOW_COEF(MyStrategy & strategy);
};