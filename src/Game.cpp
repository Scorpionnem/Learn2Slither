/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Game.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/08 15:20:56 by mbatty            #+#    #+#             */
/*   Updated: 2026/02/08 16:08:51 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Game.hpp"

std::vector<int>	lengths;

bool	Game::simulateGame(Window &window, bool print, bool visualize, bool step, bool training, float stepTime = 0.1)
{
	bool	running = true;
	int		no_eat = 0;

	_agent._learning = training;
	_game.reset();

	Direction	action = Direction::DOWN;
	std::string	upView, downView, leftView, rightView;

	while (running)
	{
		if (print) std::cout << "\033c";
		window.pollEvents();

		if (window.getEvents().getKey(SDLK_ESCAPE))
			return (false);
		if (step && !window.getEvents().getKeyPressed(SDLK_SPACE))
		{
			_game.renderMap(window.getRendererPtr());
			window.display();
			continue ;
		}

		_game.getSnakeVision(upView, downView, leftView, rightView);
		State prevstate(upView, downView, leftView, rightView);

		if (print) std::cout << Direction::UP << ": " << upView << std::endl;
		if (print) std::cout << Direction::DOWN << ": " << downView << std::endl;
		if (print) std::cout << Direction::LEFT << ": " << leftView << std::endl;
		if (print) std::cout << Direction::RIGHT << ": " << rightView << std::endl << std::endl;

		State state(upView, downView, leftView, rightView);

		if (print) std::cout << "State: " << state.hash() << std::endl;

		action = _agent.process(upView, downView, leftView, rightView);
		if (print) std::cout << std::endl << "Agent: " << action << std::endl << std::endl;

		if (visualize) _game.renderMap(window.getRendererPtr());

		bool badDir = !_game.setSnakeDir(action);

		Vec2i	prevHead = _game.getSnakeHeadPos();

		SnakeGame::Event	event = _game.update();

		if (print) std::cout << "Event: " << event << std::endl;
		if (print) std::cout << "Size: " << _game.getSnakeSize() << std::endl;

		Vec2i	newHead = _game.getSnakeHeadPos();

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
				reward += -3; break;
			case SnakeGame::Event::NONE:
			{
				Vec2i	food = _game.getClosestFood(newHead);
				float oldDist = distance(prevHead, food);
				float newDist = distance(newHead, food);

				reward += (oldDist - newDist) * 5.0f;
			}
		}
		_agent.reward(state.hash(), reward);

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
