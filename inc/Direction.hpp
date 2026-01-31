/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Direction.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/31 11:56:30 by mbatty            #+#    #+#             */
/*   Updated: 2026/01/31 11:56:53 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>

enum class	Direction
{
	UP = 0,
	DOWN = 1,
	LEFT = 2,
	RIGHT = 3,
};
inline std::ostream& operator<<(std::ostream& os, const Direction& p)
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
inline constexpr bool is_opposite(Direction a, Direction b)
{
	return ((static_cast<int>(a) ^ static_cast<int>(b)) == 1);
}
