/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Agent.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/31 18:59:38 by mbatty            #+#    #+#             */
/*   Updated: 2026/02/07 23:39:15 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Direction.hpp"

#include <unordered_map>
#include <cstdint>
#include <vector>
#include <fstream>
#include <sstream>

using u16 = uint16_t;
using u8 = uint8_t;
using u32 = uint32_t;
using i16 = int16_t;
using i32 = int32_t;

using StateHash = u16;

/*
	Hash is used to store the current state of the map around the snake's head it can be compressed in a u16 to store in the q-table
*/
struct	State
{
	State() {}
	State(const std::string &upView, const std::string &downView,
								const std::string &leftView, const std::string &rightView)
	{
		set_dir(death_up, danger_up, food_up, upView);
		set_dir(death_down, danger_down, food_down, downView);
		set_dir(death_left, danger_left, food_left, leftView);
		set_dir(death_right, danger_right, food_right, rightView);
	}
	void set_dir(bool &death, bool &danger, bool &food, const std::string &view)
	{
		if (view.size() > 1)
		{
			death  = (view[1] == 'W' || view[1] == 'S');
			danger = (view[1] == 'R');
		}
		else
		{
			death = danger = false;
		}
		food = view.find('G') != std::string::npos;
	}
	enum class	Offset
	{
		FOOD_RIGHT = 0,
		FOOD_LEFT = 1,
		FOOD_DOWN = 2,
		FOOD_UP = 3,

		DANGER_RIGHT = 4,
		DANGER_LEFT = 5,
		DANGER_DOWN = 6,
		DANGER_UP = 7,

		DEATH_RIGHT = 8,
		DEATH_LEFT = 9,
		DEATH_DOWN = 10,
		DEATH_UP = 11,
	};
	bool		death_up = false;
	bool		death_down = false;
	bool		death_left = false;
	bool		death_right = false;

	bool		danger_up = false;
	bool		danger_down = false;
	bool		danger_left = false;
	bool		danger_right = false;

	bool		food_up = false;
	bool		food_down = false;
	bool		food_left = false;
	bool		food_right = false;

	StateHash	hash()
	{
		StateHash	res = 0;

		res |= (u8(food_right) << u16(Offset::FOOD_RIGHT));
		res |= (u8(food_left) << u16(Offset::FOOD_LEFT));
		res |= (u8(food_down) << u16(Offset::FOOD_DOWN));
		res |= (u8(food_up) << u16(Offset::FOOD_UP));

		res |= (u8(danger_right) << u16(Offset::DANGER_RIGHT));
		res |= (u8(danger_left) << u16(Offset::DANGER_LEFT));
		res |= (u8(danger_down) << u16(Offset::DANGER_DOWN));
		res |= (u8(danger_up) << u16(Offset::DANGER_UP));

		res |= (u8(death_right) << u16(Offset::DEATH_RIGHT));
		res |= (u8(death_left) << u16(Offset::DEATH_LEFT));
		res |= (u8(death_down) << u16(Offset::DEATH_DOWN));
		res |= (u8(death_up) << u16(Offset::DEATH_UP));

		return (res);
	}
};

struct	Action
{
	Action() {}
	Action(Direction dir)
	{
		this->dir = dir;
	}
	Direction	dir = Direction::DOWN;
	float		value = 0;
};

#include <array>

struct QTable
{
	float	learning_rate = 0.8;
	float	discount_factor = 0.95;
	float	exploration_prob = 0.2;

	void	update_q_value(StateHash state, StateHash next_state, Direction action, float reward)
	{
		float	&q_value = states[state][static_cast<int>(action)];

		q_value += learning_rate * (reward + discount_factor * get_max_qv_for_state(next_state) - q_value);
	}

	float	get_max_qv_for_state(StateHash state)
	{
		std::array<float, 4>	&actions = states[state];
		float	max = actions[0];

		for (int i = 0; i < 4; i++)
			if (actions[i] > max)
				max = actions[i] > max;
		return (max);
	}
	Direction	get_best_action_for_state(StateHash state)
	{
		std::array<float, 4>	&actions = states[state];
		float	max = actions[0];
		int		maxi = 0;

		for (int i = 0; i < 4; i++)
			if (actions[i] > max)
			{
				maxi = i;
				max = actions[i] > max;
			}
		return (static_cast<Direction>(maxi));
	}

	std::unordered_map<StateHash, std::array<float, 4>>	states;

	void	exportModel(const std::string &path)
	{
		std::ofstream	file(path);
		if (!file.is_open())
		{
			std::cerr << "Failed to open " << path << std::endl;
			return ;
		}

		std::cout << "Exporting model in " << path << std::endl;

		for (auto pair : states)
		{
			file << pair.first << " ";
			for (float qv : pair.second)
				file << qv << " ";
			file << std::endl;
		}
	}
	void	importModel(const std::string &path)
	{
		std::ifstream	file(path);
		if (!file.is_open())
		{
			std::cerr << "Failed to open " << path << std::endl;
			return ;
		}

		std::cout << "Importing model from " << path << std::endl;

		states.clear();

		std::string	line;
		while (std::getline(file, line))
		{
			std::istringstream	iss(line);

			StateHash	hash;
			float		f1;
			float		f2;
			float		f3;
			float		f4;

			if (!(iss >> hash >> f1 >> f2 >> f3 >> f4))
			{
				std::cerr << "parsing error" << std::endl;
				continue ;
			}
			states[hash][0] = f1;
			states[hash][1] = f2;
			states[hash][2] = f3;
			states[hash][3] = f4;
		}
	}
};

float	randf()
{
	return (rand() / (float)RAND_MAX);
}

/*
	On each update the view of the snake is given to the agent, it returns its action.
	THEN, after it gave its action, the game decides what the reward is, at first the agent does not know what the best reward is?

	It "learns" by trying, even if it has found a good move it will still sometimes try random move to find better (in training mode ig)
*/
class	Agent
{
	public:
		Agent() {}
		~Agent() {}

		Direction	process(const std::string &upView, const std::string &downView,
						const std::string &leftView, const std::string &rightView)
		{
			State state(upView, downView, leftView, rightView);
			Direction	action = Direction::DOWN;

			if (_learning && randf() <= _QTable.exploration_prob)
			{
				action = static_cast<Direction>(rand() % 4);
			}
			else
			{
				action = _QTable.get_best_action_for_state(state.hash());
			}

			_last_state = state.hash();
			_last_action = action;
			return (_last_action);
		}
		void	reward(StateHash next_state, float reward)
		{
			if (!_learning)
				return ;

			_QTable.update_q_value(_last_state, next_state, _last_action, reward);
		}

		bool		_learning = true;
		void	exportModel(const std::string &path)
		{
			_QTable.exportModel(path);
		}
		void	importModel(const std::string &path)
		{
			_QTable.importModel(path);
		}
	private:
		QTable		_QTable;

		Direction	_last_action;
		StateHash	_last_state;
};
