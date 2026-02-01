/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Agent.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/31 18:59:38 by mbatty            #+#    #+#             */
/*   Updated: 2026/02/01 19:18:44 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Direction.hpp"

#include <unordered_map>
#include <cstdint>
#include <vector>

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

	/*
		Hashes the state in a u16
               DU DD DL DR FU FD FL FR DIR
		000000 0  0  0  0  0  0  0  0  00
	*/
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

struct QTable
{
	std::unordered_map<StateHash, std::pair<float, Action>>	states;
};

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

		Action	process(const std::string &upView, const std::string &downView,
						const std::string &leftView, const std::string &rightView)
		{
			State state(upView, downView, leftView, rightView);

			_lastStateProcessed = state.hash();

			auto	find = _QTable.states.find(_lastStateProcessed);
			if ((_training && rand() % 10 < 3) || find == _QTable.states.end())
			{
				Action	action = Action(Direction(rand() % 4));

				_lastActionTaken = action;

				lastVal = -42;
				return (action);
			}

			lastVal = find->second.first;
			_lastActionTaken = find->second.second;
			return (find->second.second);
		}
		void	reward(float reward)
		{
			auto	find = _QTable.states.find(_lastStateProcessed);

			if (find == _QTable.states.end())
			{
				// std::cout << "wow thats new " <<  reward << std::endl;

				_QTable.states[_lastStateProcessed].second = _lastActionTaken;
				_QTable.states[_lastStateProcessed].first = reward;
				_QTable.states[_lastStateProcessed].second.value = reward;
				return ;
			}
			if (find->second.first < reward)
			{
				// std::cout << "wow thats better " <<  reward << " " << find->second.first << std::endl;

				find->second.second = _lastActionTaken;
				find->second.first = reward;
				find->second.second.value = reward;
			}
		}
		float	getStateVal()
		{
			return (lastVal);
		}
		void	setTraining(bool state) {_training = state;}
	private:
		QTable		_QTable;
		float	lastVal = -42;
		bool	_training = true;

		Action		_lastActionTaken;
		StateHash	_lastStateProcessed;
};
