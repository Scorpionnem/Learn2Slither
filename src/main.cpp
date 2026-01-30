/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/30 10:57:51 by mbatty            #+#    #+#             */
/*   Updated: 2026/01/30 16:43:42 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <vector>
#include <iostream>
#include <memory>
#include "Vec2.hpp"

enum class	Tile
{
	WALL = 'W',
	SNAKE_HEAD = 'H',
	SNAKE_BODY = 'S',
	GREEN_APPLE = 'G',
	RED_APPLE = 'R',
	EMPTY = '0',
};

enum class	Direction
{
	UP = 0,
	DOWN = 1,
	LEFT = 2,
	RIGHT = 3,
};
std::ostream& operator<<(std::ostream& os, const Direction& p)
{
	switch (p)
	{
		case Direction::UP:
			return (os << "UP");
		case Direction::DOWN:
			return (os << "DOWN");
		case Direction::LEFT:
			return (os << "LEFT");
		case Direction::RIGHT:
			return (os << "RIGHT");
	}
	return (os);
}
constexpr bool is_opposite(Direction a, Direction b)
{
	return ((static_cast<int>(a) ^ static_cast<int>(b)) == 1);
}

struct	SnakePart
{
	SnakePart(Tile part, Direction dir, Vec2i pos)
	{
		this->part = part;
		this->dir = dir;
		this->pos = pos;
	}
	Tile			part;
	Direction		dir;
	Vec2i			pos;

	void	update(Direction &prevDir)
	{
		if (dir == Direction::UP)
			pos.y -= 1;
		if (dir == Direction::DOWN)
			pos.y += 1;
		if (dir == Direction::LEFT)
			pos.x -= 1;
		if (dir == Direction::RIGHT)
			pos.x += 1;

		std::swap(dir, prevDir);
	}
};

class	Snake
{
	public:
		Snake(Direction dir, Vec2i headPos, int length)
		{
			_parts.push_back(SnakePart(Tile::SNAKE_HEAD, dir, headPos));
			for (int i = 0; i < length; i++)
			{
				// Push parts behind head
			}
		}
		void	update() // Advances snake by 1
		{
			SnakePart	&head = getHead();
			Direction	prevDir = head.dir;

			for (SnakePart &part : _parts)
			{
				// std::cout << part.dir << " " << part.pos.x << " " << part.pos.y << std::endl;
				part.update(prevDir);
			}
		}

		void	setDirection(Direction dir)
		{
			SnakePart	&head = getHead();
			Direction	cur = head.dir;

			if (!is_opposite(cur, dir))
				head.dir = dir;
		}

		SnakePart	&getHead() {return (_parts.front());}
	private:
		std::vector<SnakePart>	_parts;
};

/*
 *
 *	Stores all of the game's tile infos.
 *	It is used to store only the basic tiles (food, walls, ...)
 *	
 *	The snake's collisions are checked by checking the tile thats at the same position as its head.
 *
 *	TODO:
 *		Add a method to get the snake's vision.
 *		Add apple spawn methods
 *
 *
 * */
class	Map
{
	public:
			Map(Vec2i size)
			{
				_size = size;
				_tiles.resize(_size.x * _size.y);
			}
	private:
			Vec2i			_size;
			std::vector<Tile>	_tiles;
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

			while (1)
			{
				_snake->setDirection(static_cast<Direction>(rand() % 3));
				_snake->update();
			}
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
