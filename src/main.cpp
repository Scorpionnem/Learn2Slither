/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/30 10:57:51 by mbatty            #+#    #+#             */
/*   Updated: 2026/01/31 22:09:51 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <vector>
#include <unistd.h>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <cstdint>
#include "Vec2.hpp"
#include <bitset>
#include "Snake.hpp"
#include "Agent.hpp"

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using i16 = int16_t;
using i32 = int32_t;

namespace Color
{
	constexpr const char	*Red = "\e[0;31m";
	constexpr const char	*Green = "\e[0;32m";
	constexpr const char	*Blue = "\e[0;34m";
	constexpr const char	*Yellow = "\e[0;33m";
	constexpr const char	*Black = "\e[0;30m";
	constexpr const char	*Purple = "\e[0;35m";
	constexpr const char	*Cyan = "\e[0;36m";
	constexpr const char	*White = "\e[0;37m";

	constexpr const char	*Reset = "\e[0m";
};

class	Game
{
	public:
		Game(Vec2i size)
		{
			_size = size + 1;

			_tiles.resize(_size.x * _size.y);
			_map.resize(_size.x * _size.y);

			resetState();

			Agent	agent;

			int	it = 0;

			while (1)
			{
				std::string	upView = getView(_snake->getHead().pos, Vec2i(0, -1));
				std::string	downView = getView(_snake->getHead().pos, Vec2i(0, 1));
				std::string	leftView = getView(_snake->getHead().pos, Vec2i(-1, 0));
				std::string	rightView = getView(_snake->getHead().pos, Vec2i(1, 0));

				Action	action = agent.process(upView, downView, leftView, rightView, it < 1000000);

				if (it > 1000000)
				{
					printMap();

					std::cout << upView << std::endl;
					std::cout << downView << std::endl;
					std::cout << leftView << std::endl;
					std::cout << rightView << std::endl;

					std::cout << action.dir << std::endl;
				}

				_snake->setDirection(action.dir);
				_snake->update();

				if (checkDeath())
				{
					if (it > 1000000)
					{
						std::cout << "DEAD" << std::endl;
						std::cout << agent.getStateVal() << std::endl;
						sleep(1);
					}
					agent.reward(-1000);
					resetState();
				}
				else if (is_opposite(action.dir, _snake->getHead().dir))
					agent.reward(-100);
				else if (checkFood())
				{
					setTile(Tile::EMPTY, _snake->getHead().pos);
					_generateFood();
					_snake->growSnake();
					agent.reward(100);
					// Good reward
				}
				else if (checkBadFood())
				{
					agent.reward(-1);
				}
				else
					agent.reward(-0.1);

				if (it++ > 1000000)
					usleep(50000);
			}
		}

		bool	checkFood()
		{
			if (getTileNoSnake(_snake->getHead().pos) == Tile::GREEN_APPLE)
				return (true);
			return (false);
		}
		bool	checkBadFood()
		{
			return (false);
		}

		bool	checkDeath()
		{
			if (!isInBounds(_snake->getHead().pos))
				return (true);
			if (getTileNoSnake(_snake->getHead().pos) == Tile::WALL)
				return (true);
			if (_snake->collides())
				return (true);
			return (false);
		}

		void	resetState()
		{
			_generateWalls();
			_spawnSnake();
		}

		std::string	getView(Vec2i pos, Vec2i dir)
		{
			std::string	res;

			while (isInBounds(pos))
			{
				res += char(getTile(pos));
				pos = pos + dir;
			}
			return (res);
		}

		bool	isInBounds(Vec2i pos)
		{
			return (pos.x >= 0 && pos.y >= 0 && pos.x < _size.x && pos.y < _size.y);
		}
		void	setTile(Tile tile, Vec2i pos)
		{
			if (!isInBounds(pos))
				return ;
			_tiles[pos.x + pos.y * _size.x] = tile;
		}
		Tile	getTile(Vec2i pos)
		{
			if (_snake->hasPart(pos))
				return (_snake->getPart(pos).part);
			if (!isInBounds(pos))
				return (Tile::WALL);
			return (_tiles[pos.x + pos.y * _size.x]);
		}
		Tile	getTileNoSnake(Vec2i pos)
		{
			if (!isInBounds(pos))
				return (Tile::WALL);
			return (_tiles[pos.x + pos.y * _size.x]);
		}

		void	printMap()
		{
			std::cout << std::endl;

			for (int y = 0; y < _size.y; y++)
			{
				for (int x = 0; x < _size.x; x++)
				{
					if (_snake->hasPart(Vec2i(x, y)))
						std::cout << Color::Green << (char)_snake->getPart(Vec2i(x, y)).part << Color::Reset;
					else
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
			_generateFood();
			_generateFood();
			_generateFood();
		}
		void	_generateFood()
		{
			setTile(Tile::GREEN_APPLE, Vec2i(rand() % (_size.x - 2) + 1, rand() % (_size.y - 2) + 1));
		}
		void	_spawnSnake()
		{
			_snake = std::make_unique<Snake>(Direction::RIGHT, _size / 2, 4);
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
#include <ctime>
int	main(void)
{
	srand(std::time(NULL));
	Game	game(Vec2i(10, 10));
	return (1);
}
