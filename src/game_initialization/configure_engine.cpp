#include "configure_engine.hpp"
#include "formula/string_utils.hpp"
#include "game_config_manager.hpp"
#include "mp_game_settings.hpp"
#include "settings.hpp"
#include "tod_manager.hpp"

#include <cassert>
#include <sstream>
#include <iostream>

namespace ng {

configure_engine::configure_engine(saved_game& state) :
	state_(state),
	parameters_(state_.mp_settings()),
	side_cfg_(state_.get_starting_pos().child_or_empty("side"))
{
	if (!state_.get_starting_pos().has_child("side")) {
		std::stringstream msg;
		msg << "Configure Engine: No sides found in scenario, aborting.";
		std::cerr << msg.str();
		std::cerr << "Full scenario config:\n";
		std::cerr << state_.to_config().debug();
		throw game::error(msg.str());
	}

	set_use_map_settings(use_map_settings_default());
	if(state_.classification().get_tagname() == "scenario") {
		for (const config& scenario :
			game_config_manager::get()->game_config().child_range(state_.classification().get_tagname())) {

			if (scenario["allow_new_game"].to_bool(true) || game_config::debug) {

				const std::string& title = (!scenario["new_game_title"].empty()) ?
					scenario["new_game_title"] : scenario["name"];

				entry_points_.push_back(&scenario);
				entry_point_titles_.push_back(title);
			}
		}
	}
}

void configure_engine::set_default_values() {
	set_use_map_settings(use_map_settings_default());
	set_game_name(game_name_default());
	set_num_turns(num_turns_default());
	set_village_gold(village_gold_default());
	set_village_support(village_support_default());
	set_xp_modifier(xp_modifier_default());
	set_mp_countdown_init_time(mp_countdown_init_time_default());
	set_mp_countdown_reservoir_time(mp_countdown_reservoir_time_default());
	set_mp_countdown_action_bonus(mp_countdown_action_bonus_default());
	set_mp_countdown(mp_countdown_default());
	set_random_start_time(random_start_time_default());
	set_fog_game(fog_game_default());
	set_shroud_game(shroud_game_default());
	set_random_faction_mode(random_faction_mode_default());
}

bool configure_engine::force_lock_settings() const {
	return state_.get_starting_pos()["force_lock_settings"].to_bool(!state_.classification().is_normal_mp_game());
}

std::string configure_engine::game_name() const { return parameters_.name; }
int configure_engine::num_turns() const { return parameters_.num_turns; }
int configure_engine::village_gold() const { return parameters_.village_gold; }
int configure_engine::village_support() const { return parameters_.village_support; }
int configure_engine::xp_modifier() const { return parameters_.xp_modifier; }
int configure_engine::mp_countdown_init_time() const { return parameters_.mp_countdown_init_time; }
int configure_engine::mp_countdown_reservoir_time() const { return parameters_.mp_countdown_reservoir_time; }
int configure_engine::mp_countdown_turn_bonus() const { return parameters_.mp_countdown_turn_bonus; }
int configure_engine::mp_countdown_action_bonus() const { return parameters_.mp_countdown_action_bonus; }
bool configure_engine::mp_countdown() const { return parameters_.mp_countdown; }
bool configure_engine::use_map_settings() const { return parameters_.use_map_settings; }
bool configure_engine::random_start_time() const { return parameters_.random_start_time; }
bool configure_engine::fog_game() const { return parameters_.fog_game; }
bool configure_engine::shroud_game() const { return parameters_.shroud_game; }
bool configure_engine::allow_observers() const { return parameters_.allow_observers; }
bool configure_engine::registered_users_only() const { return parameters_.registered_users_only; }
bool configure_engine::shuffle_sides() const { return parameters_.shuffle_sides; }
mp_game_settings::RANDOM_FACTION_MODE configure_engine::random_faction_mode() const { return parameters_.random_faction_mode; }
const config& configure_engine::options() const { return parameters_.options; }

void configure_engine::set_game_name(std::string val) { parameters_.name = val; }
void configure_engine::set_num_turns(int val) { parameters_.num_turns = val; }
void configure_engine::set_village_gold(int val) { parameters_.village_gold = val; }
void configure_engine::set_village_support(int val) { parameters_.village_support = val; }
void configure_engine::set_xp_modifier(int val) { parameters_.xp_modifier = val; }
void configure_engine::set_mp_countdown_init_time(int val) { parameters_.mp_countdown_init_time = val; }
void configure_engine::set_mp_countdown_reservoir_time(int val) { parameters_.mp_countdown_reservoir_time = val; }
void configure_engine::set_mp_countdown_turn_bonus(int val) { parameters_.mp_countdown_turn_bonus = val; }
void configure_engine::set_mp_countdown_action_bonus(int val) { parameters_.mp_countdown_action_bonus = val; }
void configure_engine::set_mp_countdown(bool val) { parameters_.mp_countdown = val; }
void configure_engine::set_use_map_settings(bool val) { parameters_.use_map_settings = val; }
void configure_engine::set_random_start_time(bool val) { parameters_.random_start_time = val; }
void configure_engine::set_fog_game(bool val) { parameters_.fog_game = val; }
void configure_engine::set_shroud_game(bool val) { parameters_.shroud_game = val; }
void configure_engine::set_allow_observers(bool val) { parameters_.allow_observers = val; }
void configure_engine::set_registered_users_only(bool val) { parameters_.registered_users_only = val; }
void configure_engine::set_oos_debug(bool val) { state_.classification().oos_debug = val; }
void configure_engine::set_shuffle_sides(bool val) { parameters_.shuffle_sides = val; }
void configure_engine::set_random_faction_mode(mp_game_settings::RANDOM_FACTION_MODE val) { parameters_.random_faction_mode = val;}
void configure_engine::set_options(const config& cfg) { parameters_.options = cfg; }

void configure_engine::set_scenario(size_t scenario_num) {
	const config& scenario = *entry_points_[scenario_num];

	parameters_.hash = scenario.hash();
	state_.set_scenario(scenario);
}

bool configure_engine::set_scenario(std::string& scenario_id) {
	for (size_t i = 0; i < entry_points_.size(); ++i) {
		if ((**(entry_points_.begin() + i))["id"] == scenario_id) {
			set_scenario(i);
			return true;
		}
	}
	return false;
}

std::string configure_engine::game_name_default() const {
	utils::string_map i18n_symbols;
	i18n_symbols["login"] = preferences::login();
	return vgettext("$login|’s game", i18n_symbols);
}
int configure_engine::num_turns_default() const {
	return use_map_settings() ?
		settings::get_turns(state_.get_starting_pos()["turns"]) :
		preferences::turns();
}
int configure_engine::village_gold_default() const {
	return use_map_settings() && !side_cfg_.empty() ?
		settings::get_village_gold(side_cfg_["village_gold"], &state_.classification()) :
		preferences::village_gold();
}
int configure_engine::village_support_default() const {
	return use_map_settings() && !side_cfg_.empty() ?
		settings::get_village_support(side_cfg_["village_support"]) :
		preferences::village_support();
}
int configure_engine::xp_modifier_default() const {
	return use_map_settings() ?
		settings::get_xp_modifier(state_.get_starting_pos()["experience_modifier"]) :
		preferences::xp_modifier();
}
int configure_engine::mp_countdown_init_time_default() const {
	return preferences::countdown_init_time();
}
int configure_engine::mp_countdown_reservoir_time_default() const {
	return preferences::countdown_reservoir_time();
}
int configure_engine::mp_countdown_turn_bonus_default() const {
	return preferences::countdown_turn_bonus();
}
int configure_engine::mp_countdown_action_bonus_default() const {
	return preferences::countdown_action_bonus();
}
bool configure_engine::mp_countdown_default() const {
	return preferences::countdown();
}
bool configure_engine::use_map_settings_default() const {
	return force_lock_settings() || preferences::use_map_settings();
}
bool configure_engine::random_start_time_default() const {
	return use_map_settings() ?
		state_.get_starting_pos()["random_start_time"].to_bool(false) :
		preferences::random_start_time();
}
bool configure_engine::fog_game_default() const {
	return use_map_settings() && !side_cfg_.empty() ?
		side_cfg_["fog"].to_bool(state_.classification().is_normal_mp_game()) :
		preferences::fog();
}
bool configure_engine::shroud_game_default() const {
	return use_map_settings() && !side_cfg_.empty() ?
		side_cfg_["shroud"].to_bool(false) :
		preferences::shroud();
}
bool configure_engine::allow_observers_default() const {
	return preferences::allow_observers();
}
bool configure_engine::registered_users_only_default() const
{
	return preferences::registered_users_only();
}
bool configure_engine::shuffle_sides_default() const {
	return preferences::shuffle_sides();
}
mp_game_settings::RANDOM_FACTION_MODE configure_engine::random_faction_mode_default() const {
	return mp_game_settings::RANDOM_FACTION_MODE::string_to_enum(preferences::random_faction_mode(), mp_game_settings::RANDOM_FACTION_MODE::DEFAULT);
}

const config& configure_engine::options_default() const {
	return preferences::options();
}

const mp_game_settings& configure_engine::get_parameters() const {
	return parameters_;
}

const std::vector<std::string>& configure_engine::entry_point_titles() const {
	return entry_point_titles_;
}

void configure_engine::write_parameters()
{
	config& scenario = this->state_.get_starting_pos();
	const mp_game_settings& params = this->state_.mp_settings();

	if (params.random_start_time) {
		if (!tod_manager::is_start_ToD(scenario["random_start_time"])) {
			scenario["random_start_time"] = true;
		}
	}
	else {
		scenario["random_start_time"] = false;
	}
	scenario["experience_modifier"] = params.xp_modifier;
	scenario["turns"] = params.num_turns;

	for (config& side : scenario.child_range("side"))
	{
		side["fog"] = params.fog_game;
		side["shroud"] = params.shroud_game;
		if (!params.use_map_settings) {
			side["village_gold"] = params.village_gold;
			side["village_support"] = params.village_support;
		}
	}
}

} //end namespace ng
