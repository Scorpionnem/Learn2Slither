/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/30 10:57:51 by mbatty            #+#    #+#             */
/*   Updated: 2026/01/31 12:07:59 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <vector>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <cstdint>
#include "Vec2.hpp"
#include <bitset>
#include "Snake.hpp"

using u8 = uint8_t;
using u16 = uint16_t;
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
	Direction	dir = Direction::DOWN;
};

#define SET_CURRENT_STATE_DIR(state, dir)												\
			state.death_##dir = dir##View.find("WHS") != dir##View.npos;	\
			state.danger_##dir = dir##View.find("R") != dir##View.npos;	\
			state.food_##dir = dir##View.find("G") != dir##View.npos;	\

struct QTable
{
	std::unordered_map<StateHash, std::vector<Action>>	states;
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
			State state = _makeState(upView, downView, leftView, rightView);

			std::cout << std::bitset<16>(state.hash()).to_string() << std::endl;

			return (Action());
		}
	private:
		State	_makeState(const std::string &upView, const std::string &downView,
									const std::string &leftView, const std::string &rightView)
		{
			State	s;

			SET_CURRENT_STATE_DIR(s, up);
			SET_CURRENT_STATE_DIR(s, down);
			SET_CURRENT_STATE_DIR(s, left);
			SET_CURRENT_STATE_DIR(s, right);
			return (s);
		}
		QTable		_QTable;
};

class	Game
{
	public:
		Game(Vec2i size)
		{
			_size = size + 1;

			_tiles.resize(_size.x * _size.y);
			_map.resize(_size.x * _size.y);

			_generateWalls();
			_spawnSnake();

			printMap();

			Agent	agent;

			agent.process("R", "", "", "");
		}

		bool	isInBounds(Vec2i pos)
		{
			return (pos.x >= 0 || pos.y >= 0 || pos.x < _size.x || pos.y < _size.y);
		}
		void	setTile(Tile tile, Vec2i pos)
		{
			if (!isInBounds(pos))
				return ;
			_tiles[pos.x + pos.y * _size.x] = tile;
		}
		Tile	getTile(Vec2i pos)
		{
			if (!isInBounds(pos))
				return (Tile::WALL);
			return (_tiles[pos.x + pos.y * _size.x]);
		}

		void	printMap()
		{
			for (int x = 0; x < _size.x; x++)
			{
				for (int y = 0; y < _size.y; y++)
				{
					std::cout << (char)getTile(Vec2i(x, y));
				}
				std::cout << std::endl;
			}
		}
	private:
		void	_generateWalls()
		{
			for (int x = 0; x < _size.x; x++)
			{
				for (int y = 0; y < _size.y; y++)
				{
					if (y == 0 || x == 0 || x == _size.x - 1 || y == _size.y - 1)
						setTile(Tile::WALL, Vec2i(x, y));
					else
						setTile(Tile::EMPTY, Vec2i(x, y));
				}
			}
		}
		void	_spawnSnake()
		{
			_snake = std::make_unique<Snake>(Direction::RIGHT, Vec2i(0), 1);
		}

		std::unique_ptr<Snake>	_snake;
		std::vector<Tile>		_tiles;

		std::vector<Tile>		_map;
		Vec2i					_size;
};

/*
	Board size : 10 * 10
	2 Green apples at random pos
	1 Red apple at random pos
	Snake of length 3 starts at random pos (Placed straight)
	Game over if:
		Snake hits a wall
		Snake hits its tail
		Snake length goes to 0
	+1 Length if snake eats GREEN apple (A new green apple spawns on the board)
	-1 Length if snake eats RED apple (A new red apple spawns on the board)


	I need to inform myself about Q-Learning but rn by idea is this:

		Have a hash map of situation -> moves done -> results
			When it finds a situation it already knows it checks the moves that it already did in the past and takes the one with the best result
				I have no idea if that fits the subject but idk I thought abt it.
			If it cant find a situation:
				Try to find a similar situation (Would need to find some algo for that)
				Take random choice to build up its situation table.
*/
int	main(void)
{
	Game	game(Vec2i(10, 10));
	return (1);
}
