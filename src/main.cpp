/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/30 10:57:51 by mbatty            #+#    #+#             */
/*   Updated: 2026/02/06 17:21:57 by mbatty           ###   ########.fr       */
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
*/
class	SnakeGame
{
	public:
		enum class	Event
		{
			DEATH,
			SHRINK_SNAKE,
			GROW_SNAKE,
			NONE
		};
	public:
		SnakeGame(Vec2i size)
		{
			_size = size;
			_tiles.resize(_size.x * _size.y);
		}
		~SnakeGame() {}

		Event	update()
		{
			if (_dead)
				return (Event::DEATH);

			_snake->update();

			if (_checkDeath())
			{
				_dead = true;
				return (Event::DEATH);
			}

			if (_checkFood())
			{
				_snake->grow();
				_setTile(Tile::EMPTY, _snake->getHead().pos);
				_generateFood(Tile::GREEN_APPLE);
				return (Event::GROW_SNAKE);
			}
			if (_checkBadFood())
			{
				_snake->shrink();
				_setTile(Tile::EMPTY, _snake->getHead().pos);
				_generateFood(Tile::RED_APPLE);
				if (_snake->size() == 0)
					return (Event::DEATH);
				return (Event::SHRINK_SNAKE);
			}

			return (Event::NONE);
		}
		void	getSnakeVision(std::string &up, std::string &down, std::string &left, std::string &right)
		{
			up = _getVision(_snake->getHead().pos, Vec2i(0, -1));
			down = _getVision(_snake->getHead().pos, Vec2i(0, 1));
			left = _getVision(_snake->getHead().pos, Vec2i(-1, 0));
			right = _getVision(_snake->getHead().pos, Vec2i(1, 0));
		}
		void	reset()
		{
			_generateWalls();

			_spawnSnake();

			_generateFood(Tile::GREEN_APPLE);
			_generateFood(Tile::GREEN_APPLE);

			_generateFood(Tile::RED_APPLE);

			_dead = false;
		}

		bool	setSnakeDir(Direction dir)
		{
			return (_snake->setDirection(dir));
		}

		void	printMap()
		{
			for (int y = 0; y < _size.y; y++)
			{
				for (int x = 0; x < _size.x; x++)
				{
					if (_snake->hasPart(Vec2i(x, y)))
						std::cout << Color::Green << (char)_snake->getPart(Vec2i(x, y)).part << Color::Reset;
					else
					{
						Tile	tile = _getTile(Vec2i(x, y));
						if (tile == Tile::GREEN_APPLE)
							std::cout << Color::Cyan;
						if (tile == Tile::RED_APPLE)
							std::cout << Color::Red;
						std::cout << (char)tile;
						std::cout << Color::Reset;
					}
				}
				std::cout << std::endl;
			}
		}
		int	getSnakeSize()
		{
			return (_snake->size());
		}
	private:
		bool	_checkFood()
		{
			if (_getTileNoSnake(_snake->getHead().pos) == Tile::GREEN_APPLE)
				return (true);
			return (false);
		}
		bool	_checkBadFood()
		{
			if (_getTileNoSnake(_snake->getHead().pos) == Tile::RED_APPLE)
				return (true);
			return (false);
		}
		bool	_checkDeath()
		{
			if (_snake->size() == 0)
				return (true);
			if (!_isInBounds(_snake->getHead().pos))
				return (true);
			if (_getTileNoSnake(_snake->getHead().pos) == Tile::WALL)
				return (true);
			if (_snake->collides())
				return (true);
			return (false);
		}

		std::string	_getVision(Vec2i pos, Vec2i dir)
		{
			std::string	res;

			while (_isInBounds(pos))
			{
				res += char(_getTile(pos));
				pos = pos + dir;
			}
			return (res);
		}
		bool	_isInBounds(Vec2i pos)
		{
			return (pos.x >= 0 && pos.y >= 0 && pos.x < _size.x && pos.y < _size.y);
		}

		void	_setTile(Tile tile, Vec2i pos)
		{
			if (!_isInBounds(pos))
				return ;
			_tiles[pos.x + pos.y * _size.x] = tile;
		}
		Tile	_getTile(Vec2i pos)
		{
			if (_snake->hasPart(pos))
				return (_snake->getPart(pos).part);
			if (!_isInBounds(pos))
				return (Tile::WALL);
			return (_tiles[pos.x + pos.y * _size.x]);
		}
		Tile	_getTileNoSnake(Vec2i pos)
		{
			if (!_isInBounds(pos))
				return (Tile::WALL);
			return (_tiles[pos.x + pos.y * _size.x]);
		}

		void	_generateFood(Tile type)
		{
			while (1)
			{
				Vec2i	pos = Vec2i(rand() % (_size.x - 2) + 1, rand() % (_size.y - 2) + 1);
				if (_getTile(pos) == Tile::EMPTY)
				{
					_setTile(type, pos);
					break ;
				}
			}
		}
		void	_generateBadFood()
		{
			_setTile(Tile::RED_APPLE, Vec2i(rand() % (_size.x - 2) + 1, rand() % (_size.y - 2) + 1));
		}
		void	_spawnSnake()
		{
			Vec2i	spawnPos = _size / 2;
			_snake = std::make_unique<Snake>(Direction::RIGHT, spawnPos, 2);
		}
		void	_generateWalls()
		{
			for (int x = 0; x < _size.x; x++)
			{
				for (int y = 0; y < _size.y; y++)
				{
					if (y == 0 || x == 0 || x == _size.x - 1 || y == _size.y - 1)
						_setTile(Tile::WALL, Vec2i(x, y));
					else
						_setTile(Tile::EMPTY, Vec2i(x, y));
				}
			}
		}

		std::unique_ptr<Snake>	_snake;
		std::vector<Tile>		_tiles;
		Vec2i					_size;

		bool					_dead = false;
};

inline std::ostream& operator<<(std::ostream& os, const SnakeGame::Event& p)
{
	switch (p)
	{
		case SnakeGame::Event::DEATH:
			return (os << "DEATH");
		case SnakeGame::Event::NONE:
			return (os << "NONE");
		case SnakeGame::Event::SHRINK_SNAKE:
			return (os << "SHRINK_SNAKE");
		case SnakeGame::Event::GROW_SNAKE:
			return (os << "GROW_SNAKE");
	}
	return (os);
}

class	Game
{
	public:
		Game(Vec2i size) : _game(size) {}

		void	simulateGame(bool print, bool training, float stepTime = 0.1)
		{
			_game.reset();

			bool	running = true;
			int	no_eat = 0;
			while (running)
			{
				if (print) _agent._learning = false;
				if (print) std::cout << Color::Cyan << "VVVVVV" << Color::Reset << std::endl;

				std::string	upView, downView, leftView, rightView;
				_game.getSnakeVision(upView, downView, leftView, rightView);

				Direction	action = _agent.process(upView, downView, leftView, rightView);

				if (print) std::cout << "Agent: " << action << std::endl;
				_game.setSnakeDir(action);

				SnakeGame::Event	event = _game.update();

				_game.getSnakeVision(upView, downView, leftView, rightView);
				State state(upView, downView, leftView, rightView);

				if (print) std::cout << "Event: " << event << std::endl;
				if (print) std::cout << "Size: " << _game.getSnakeSize() << std::endl;
				if (print) std::cout << "Biggest " << _maxSizeTotal << std::endl;
				if (print) std::cout << "State " << state.hash() << std::endl;
				if (print) _game.printMap();

				switch (event)
				{
					case SnakeGame::Event::DEATH:
						_agent.reward(0, -100); running = false; break;
					case SnakeGame::Event::GROW_SNAKE:
						_agent.reward(state.hash(), 100); no_eat = 0; break;
					case SnakeGame::Event::SHRINK_SNAKE:
						_agent.reward(state.hash(), -10); break;
					case SnakeGame::Event::NONE:
						_agent.reward(state.hash(), -1); break;
				}

				if (print) std::cout << Color::Cyan << "^^^^^^^^" << Color::Reset << std::endl << std::endl;
				if (print) usleep(stepTime * 1000000);

				if (!training && no_eat++ > 64)
					break ;
			}
			if (_game.getSnakeSize() > _maxSizeTotal)
				_maxSizeTotal = _game.getSnakeSize();
		}
		void	exportModel(const std::string &path)
		{
			_agent.exportModel(path);
		}
		void	importModel(const std::string &path)
		{
			_agent.importModel(path);
		}
	private:
		int			_maxSizeTotal = 0;

		Agent		_agent;
		SnakeGame	_game;
};

#include <ctime>
int	main(void)
{
	srand(std::time(NULL));
	Game	game(Vec2i(11, 11));

	game.importModel("model.txt");
	int	trainCycles = 50000;
	for (int i = 0; i < trainCycles; i++)
	{
		std::cout << '\r' << i << "/" << trainCycles;
		game.simulateGame(false, true);
	}
	game.exportModel("models/50k11x11.txt");
	while (1)
		game.simulateGame(true, false, 0.1);
	return (1);
}
