/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/30 10:57:51 by mbatty            #+#    #+#             */
/*   Updated: 2026/02/08 14:15:26 by mbatty           ###   ########.fr       */
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
#include "SnakeGame.hpp"

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using i16 = int16_t;
using i32 = int32_t;

std::vector<int>	lengths;

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
			Direction	action = Direction::DOWN;
			
			std::string	upView, downView, leftView, rightView;
			_game.getSnakeVision(upView, downView, leftView, rightView);
			action = _agent.process(upView, downView, leftView, rightView);

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

				if (print) std::cout << "\033c";

				_game.getSnakeVision(upView, downView, leftView, rightView);
				State prevstate(upView, downView, leftView, rightView);

				if (print) std::cout << Direction::UP << " " << upView << std::endl;
				if (print) std::cout << Direction::DOWN << " " << downView << std::endl;
				if (print) std::cout << Direction::LEFT << " " << leftView << std::endl;
				if (print) std::cout << Direction::RIGHT << " " << rightView << std::endl << std::endl;

				float	lastReward = _agent.getLastReward();
				action = _agent.process(upView, downView, leftView, rightView);
				if (print) _agent.print_values();
				if (print) std::cout << std::endl << "Agent: " << action << std::endl;

				if (visualize) _game.renderMap(window.getRendererPtr());

				bool badDir = !_game.setSnakeDir(action);

				Vec2i	prevHead = _game.getSnakeHeadPos();

				SnakeGame::Event	event = _game.update();

				Vec2i	newHead = _game.getSnakeHeadPos();

				if (_game.getSnakeSize() != 0)
					_game.getSnakeVision(upView, downView, leftView, rightView);
				State state(upView, downView, leftView, rightView);

				float	reward = 0;

				if (badDir && event != SnakeGame::Event::DEATH)
					reward += -50;

				switch (event)
				{
					case SnakeGame::Event::DEATH:
						reward += -100; running = false; break;
					case SnakeGame::Event::GROW_SNAKE:
						reward += 100; no_eat = 0; break;
					case SnakeGame::Event::SHRINK_SNAKE:
						reward += -10; break;
					case SnakeGame::Event::NONE:
					{
						Vec2i	food = _game.getClosestFood(newHead);
						float oldDist = distance(prevHead, food);
						float newDist = distance(newHead, food);

						reward += (oldDist - newDist) * 5.0f;
					}
				}
				_agent.reward(state.hash(), reward);

				if (print) std::cout << "Q-Value: " << lastReward << std::endl;
				if (print) std::cout << "Reward: " << reward << std::endl << std::endl;

				if (print) std::cout << "Event: " << event << std::endl;
				if (print) std::cout << "Size: " << _game.getSnakeSize() << std::endl;
				if (print) std::cout << "Biggest " << _maxSizeTotal << std::endl;
				if (print) std::cout << "State " << state.hash() << std::endl;


				if (!training) usleep(stepTime * 1000000);

				if (!training && no_eat++ > 64)
					break ;

				window.display();
				if (print) std::cout << std::endl;
			}
			if (_game.getSnakeSize() > _maxSizeTotal)
				_maxSizeTotal = _game.getSnakeSize();
			lengths.push_back(_game.getSnakeSize());
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

	if (epochs <= 0 || (width < 8 && width > 100) || (height > 100 && height < 8))
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
	{
		if (!game.simulateGame(window, print, visualize, step, train, 0.1 * visualize))
			break ;
		if (train) std::cout << "\r" << i;
	}
	if (train)
		game.exportModel(export_path);

	if (lengths.size())
	{
		int	avg = 0;
		for (int len : lengths)
			avg += len;
		avg /= lengths.size();
		std::cout << "Average length " << avg << std::endl;
	}
	return (0);
}
