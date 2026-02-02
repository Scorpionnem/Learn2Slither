/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/30 10:57:51 by mbatty            #+#    #+#             */
/*   Updated: 2026/02/02 11:05:21 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <vector>
#include <unistd.h>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <algorithm>
#include <cstdint>
#include "Vec2.hpp"
#include <bitset>
#include "Snake.hpp"
#include "Agent.hpp"

#include "Window.hpp"
#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_sdlrenderer2.h>

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
			reset();
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
				return (Event::GROW_SNAKE);
			}
			if (_checkBadFood())
			{
				_snake->shrink();
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
			_tiles.resize(_size.x * _size.y);

			_generateWalls();

			_spawnSnake();

			// _generateFood();
			// _generateFood();

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
						std::cout << (char)_getTile(Vec2i(x, y));
				}
				std::cout << std::endl;
			}
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
			return (false);
		}
		bool	_checkDeath()
		{
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

		void	_generateFood()
		{
			_setTile(Tile::GREEN_APPLE, Vec2i(rand() % (_size.x - 2) + 1, rand() % (_size.y - 2) + 1));
		}
		void	_spawnSnake()
		{
			_snake = std::make_unique<Snake>(Direction::RIGHT, _size / 2, 2);
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

#define TILE_SIZE 64
class	Game
{
	public:
		Game(Vec2i size) : _game(size)
		{
			_size = size;
		}

		void	start()
		{
			_window.open(_size.x * TILE_SIZE, _size.y * TILE_SIZE, "Learn2Slither");

			while (_running)
			{
				_window.pollEvents();

				ImGui_ImplSDLRenderer2_NewFrame();
				ImGui_ImplSDL2_NewFrame();
				ImGui::NewFrame();
				///////////////////////

				const Window::Events	&events = _window.getEvents();

				if (events.getKey(SDLK_ESCAPE))
					_running = false;

				ImGui::Begin("Training");
				
				ImGui::InputInt("Sessions", &_trainingSessions);
				_trainingSessions = std::clamp(_trainingSessions, 1, 10000000);
				ImGui::Checkbox("Render", &_renderTraining);

				if (ImGui::Button("Start"))
					trainAgent(_trainingSessions);
				ImGui::End();

				///////////////////
				ImGui::Render();
				ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), _window.getRendererPtr());
				_window.display();
			}
		}
		void	trainAgent(int sessions)
		{
			for (int i = 0; i < sessions; i++)
			{
				simulateGame(_renderTraining, true);
				std::cout << '\r' << i + 1 << "/" << sessions;
			}
			std::cout << std::endl;
		}
		void	simulateGame(bool print, bool training, float stepTime = 0.1)
		{
			_game.reset();

			bool	running = true;
			_agent.setTraining(training);
			while (running)
			{
				if (print) std::cout << Color::Cyan << "VVVVVV" << Color::Reset << std::endl;

				std::string	upView, downView, leftView, rightView;
				_game.getSnakeVision(upView, downView, leftView, rightView);

				Action	action = _agent.process(upView, downView, leftView, rightView);

				if (print) std::cout << "Agent: " << action.dir << " decision had a value of " << action.value << std::endl;
				_game.setSnakeDir(action.dir);

				SnakeGame::Event	event = _game.update();
				
				if (print) std::cout << "Event: " << event << std::endl;
				if (print) _game.printMap();

				switch (event)
				{
					case SnakeGame::Event::DEATH:
						_agent.reward(-100); running = false; break;
					case SnakeGame::Event::GROW_SNAKE:
						_agent.reward(100); break;
					case SnakeGame::Event::SHRINK_SNAKE:
						_agent.reward(-10); break;
					case SnakeGame::Event::NONE:
						_agent.reward(-0.1); break;
				}

				if (print) std::cout << Color::Cyan << "^^^^^^^^" << Color::Reset << std::endl << std::endl;
				if (print) usleep(stepTime * 1000000);
			}
		}
	private:
		int			_trainingSessions = 1;
		bool		_renderTraining = true;
		Window		_window;
		bool		_running = true;
		Agent		_agent;
		SnakeGame	_game;
		Vec2i		_size;
};

#include <ctime>
int	main(void)
{
	srand(std::time(NULL));
	
	Game	game(Vec2i(11, 11));

	game.start();
	return (1);
}
