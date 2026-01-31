/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/30 10:57:51 by mbatty            #+#    #+#             */
/*   Updated: 2026/01/31 19:27:08 by mbatty           ###   ########.fr       */
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
#include "Agent.hpp"

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using i16 = int16_t;
using i32 = int32_t;

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

			Agent	agent;

			printMap();
			_snake->update();
			printMap();
			_snake->update();
			printMap();
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

		void	printMap()
		{
			std::cout << std::endl;

			for (int y = 0; y < _size.y; y++)
			{
				for (int x = 0; x < _size.x; x++)
				{
					if (_snake->hasPart(Vec2i(x, y)))
						std::cout << (char)_snake->getPart(Vec2i(x, y)).part;
					else
						std::cout << (char)getTile(Vec2i(x, y));
				}
				std::cout << std::endl;
			}

			std::cout << std::endl;
			std::cout << getView(_snake->getHead().pos, Vec2i(0, -1)) << " " << Direction::UP << std::endl; // UP
			std::cout << getView(_snake->getHead().pos, Vec2i(0, 1)) << " " << Direction::DOWN << std::endl; // DOWN
			std::cout << getView(_snake->getHead().pos, Vec2i(-1, 0)) << " " << Direction::LEFT << std::endl; // LEFT
			std::cout << getView(_snake->getHead().pos, Vec2i(1, 0)) << " " << Direction::RIGHT << std::endl; // RIGHT
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
int	main(void)
{
	Game	game(Vec2i(10, 10));
	return (1);
}
