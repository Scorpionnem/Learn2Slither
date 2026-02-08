/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Snake.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/31 11:55:12 by mbatty            #+#    #+#             */
/*   Updated: 2026/02/08 15:21:44 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Direction.hpp"
#include "Vec2.hpp"
#include "Tile.hpp"

#include <vector>

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

inline Vec2i	backward(Direction dir)
{
	switch (dir)
	{
		case Direction::UP:    return (Vec2i(0, 1));
		case Direction::DOWN:  return (Vec2i(0, -1));
		case Direction::LEFT:  return (Vec2i(1, 0));
		case Direction::RIGHT: return (Vec2i(-1, 0));
	}
	return Vec2i(0);
}

class	Snake
{
	public:
		Snake(Direction dir, Vec2i headPos, int length)
		{
			_parts.push_back(SnakePart(Tile::SNAKE_HEAD, dir, headPos));

			Vec2i	body_dir = backward(dir);

			for (int i = 0; i < length; i++)
			{
				Vec2i pos(
					headPos.x + body_dir.x * (i + 1),
					headPos.y + body_dir.y * (i + 1)
				);

				_parts.push_back(SnakePart(Tile::SNAKE_BODY, dir, pos));
			}
		}
		void	update() // Advances snake by 1
		{
			SnakePart	&head = getHead();
			Direction	prevDir = head.dir;

			lastTailDir = _parts.back().dir;
			lastTailPos = _parts.back().pos;
			for (SnakePart &part : _parts)
			{
				// std::cout << part.dir << " " << part.pos.x << " " << part.pos.y << std::endl;
				part.update(prevDir);
			}
		}
		void	grow()
		{
			_parts.push_back(SnakePart(Tile::SNAKE_BODY, lastTailDir, lastTailPos));
		}
		int	size()
		{
			return (_parts.size());
		}
		void	shrink()
		{
			_parts.pop_back();
		}

		bool	hasPart(Vec2i pos)
		{
			for (SnakePart &part : _parts)
				if (part.pos == pos)
					return (true);
			return (false);
		}
		const SnakePart	&getPart(Vec2i pos)
		{
			for (SnakePart &part : _parts)
				if (part.pos == pos)
					return (part);
			throw std::runtime_error("Snake has no part here!");
		}
		bool	collides()
		{
			for (SnakePart &part : _parts)
			{
				for (SnakePart &part_check : _parts)
					if (part.pos == part_check.pos && &part != &part_check)
						return (true);
			}
			return (false);
		}

		bool	setDirection(Direction dir)
		{
			SnakePart	&head = getHead();
			Direction	cur = head.dir;

			if (!is_opposite(cur, dir))
			{
				head.dir = dir;
				return (true);
			}
			return (false);
		}

		SnakePart	&getHead() {return (_parts.front());}
	private:
		std::vector<SnakePart>	_parts;
		Direction				lastTailDir;
		Vec2i					lastTailPos;
};
