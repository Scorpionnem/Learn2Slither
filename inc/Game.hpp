/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Game.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/08 15:17:59 by mbatty            #+#    #+#             */
/*   Updated: 2026/02/08 15:38:30 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

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

extern std::vector<int>	lengths;

class	Game
{
	public:
		Game(Vec2i size) : _game(size) {}

		bool	simulateGame(Window &window, bool print, bool visualize, bool step, bool training, float stepTime);
		void	exportModel(const std::string &path)
		{
			_agent.exportModel(path);
		}
		void	importModel(const std::string &path)
		{
			_agent.importModel(path);
		}
		int	getMaxSize() {return (_maxSizeTotal);}
	private:
		int			_maxSizeTotal = 0;

		Agent		_agent;
		SnakeGame	_game;
};

