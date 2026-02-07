/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/30 10:57:51 by mbatty            #+#    #+#             */
/*   Updated: 2026/02/07 22:44:22 by mbatty           ###   ########.fr       */
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
#include "Window.hpp"

#define TILE_SIZE 64

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

		void	renderMap(SDL_Renderer *renderer)
		{
			for (int y = 0; y < _size.y; y++)
			{
				for (int x = 0; x < _size.x; x++)
				{
					SDL_Rect rectangle = {x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
					Tile	tile;

					if (_snake->hasPart(Vec2i(x, y)))
						tile = _snake->getPart(Vec2i(x, y)).part;
					else
						tile = _getTile(Vec2i(x, y));

					switch (tile)
					{
						case Tile::EMPTY:
							SDL_SetRenderDrawColor(renderer, 0,  0, 0, 1); break;
						case Tile::SNAKE_BODY:
							SDL_SetRenderDrawColor(renderer, 0,  169, 64, 1); break;
						case Tile::SNAKE_HEAD:
							SDL_SetRenderDrawColor(renderer, 0,  150, 64, 1); break;
						case Tile::WALL:
							SDL_SetRenderDrawColor(renderer, 128,  128, 128, 1); break;
						case Tile::GREEN_APPLE:
							SDL_SetRenderDrawColor(renderer, 64,  167, 0, 1); break;
						case Tile::RED_APPLE:
							SDL_SetRenderDrawColor(renderer, 167,  0, 0, 1); break;
					}
					SDL_RenderFillRect(renderer, &rectangle);
				}
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

		bool	simulateGame(Window &window, bool print, bool visualize, bool step, bool training, float stepTime = 0.1)
		{
			_game.reset();

			bool	running = true;
			int	no_eat = 0;
			_agent._learning = training;
			while (running)
			{
				window.pollEvents();
				if (window.getEvents().getKey(SDLK_ESCAPE))
					return (false);
				if (step && !window.getEvents().getKeyPressed(SDLK_SPACE))
				{
					_game.renderMap(window.getRendererPtr());
					window.display();
					continue ;
				}

				std::string	upView, downView, leftView, rightView;
				_game.getSnakeVision(upView, downView, leftView, rightView);

				if (print) std::cout << Direction::UP << " " << upView << std::endl;
				if (print) std::cout << Direction::DOWN << " " << downView << std::endl;
				if (print) std::cout << Direction::LEFT << " " << leftView << std::endl;
				if (print) std::cout << Direction::RIGHT << " " << rightView << std::endl;
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

				if (visualize) _game.renderMap(window.getRendererPtr());

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

				if (!training) usleep(stepTime * 1000000);

				if (!training && no_eat++ > 64)
					break ;

				window.display();
				if (print) std::cout << std::endl;
			}
			if (_game.getSnakeSize() > _maxSizeTotal)
				_maxSizeTotal = _game.getSnakeSize();
			return (true);
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
#include <map>

void	print_help()
{
	std::cout << "usage: ./Learn2Slither [-h, -i, -e, -E, -t, -W, -H, -v, -p]" << std::endl << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "-h --help\tshow help message and exit" << std::endl;
	std::cout << "-i --import\tpath of file to import agent's model (defaults to \"models/out.txt\")" << std::endl;
	std::cout << "-e --export\tpath of file to export agent's model (only used with --train)" << std::endl;
	std::cout << "-E --epochs\tnumber of epochs to train/test the model" << std::endl;
	std::cout << "-t --train\tactivates agent's training mode" << std::endl;
	std::cout << "-W --width\tchange map's width" << std::endl;
	std::cout << "-H --height\tchange map's height" << std::endl;
	std::cout << "-v --visualize\topen window to show the board" << std::endl;
	std::cout << "-p --print\tprint model's vision and decisions" << std::endl;
	std::cout << "-s --step\tactivate step by step mode (requires --visualize)" << std::endl;
}

int	main(int ac, char **av)
{
	srand(std::time(NULL));

	std::string	import_path;
	std::string	export_path = "models/out.txt";
	int			epochs = 1;
	int			width = 11;
	int			height = 11;
	bool		help = false;
	bool		train = false;
	bool		print = false;
	bool		visualize = false;
	bool		step = false;

	std::map<std::string, std::string&>	arg_options =	{{"--export", export_path}, {"-e", export_path},
														 {"--import", import_path}, {"-i", import_path},};

	std::map<std::string, int&>	int_options =			{{"--epochs", epochs}, {"-E", epochs},
														 {"--width", width}, {"-W", width},
														 {"--height", height}, {"-H", height}};

	std::map<std::string, bool&>		bool_options =	{{"--help", help}, {"-h", help},
														 {"--visualize", visualize}, {"-v", visualize},
														 {"--print", print}, {"-p", print},
														 {"--step", step}, {"-s", step},
														 {"--train", train}, {"-t", train}};

	av++;
	int	i = -1;
	while (av[++i])
	{
		std::string	arg = av[i];
		if (arg_options.find(arg) != arg_options.end())
		{
			if (!av[i + 1])
			{
				std::cerr << arg << " needs an argument" << std::endl;
				return (1);
			}
			arg_options.find(arg)->second = av[i + 1];
			i++;
		}
		else if (int_options.find(arg) != int_options.end())
		{
			if (!av[i + 1])
			{
				std::cerr << arg << " needs an argument" << std::endl;
				return (1);
			}
			int_options.find(arg)->second = std::atoi(av[i + 1]);
			i++;
		}
		else if (bool_options.find(arg) != bool_options.end())
			bool_options.find(arg)->second = true;
		else
		{
			std::cerr << "Unknown option (use --help)" << std::endl;
			return (1);
		}
	}

	if (step && !visualize)
	{
		std::cerr << "Step mode requires visualize option" << std::endl;
		return (1);
	}

	if (epochs <= 0 || (width <= 10 && width > 100) || (height > 100 && height <= 10))
	{
		std::cerr << "Invalid input" << std::endl;
		return (1);
	}

	if (help) { print_help(); return (0); }

	Window	window;

	if (visualize)
	{
		window.open(width * TILE_SIZE, height * TILE_SIZE, "TROP BIEN LE SNAKE IL APPREND");
	}

	Game	game(Vec2i(width, height));

	if (!import_path.empty())
		game.importModel(import_path);

	for (int i = 0; i < epochs; i++)
		if (!game.simulateGame(window, print, visualize, step, train, 0.1 * visualize))
			break ;
	if (train)
		game.exportModel(export_path);
	return (0);
}
