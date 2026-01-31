/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Map.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/31 11:58:23 by mbatty            #+#    #+#             */
/*   Updated: 2026/01/31 11:58:44 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Vec2.hpp"
#include "Tile.hpp"

#include <vector>

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

