/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SnakeGame.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/08 12:43:08 by mbatty            #+#    #+#             */
/*   Updated: 2026/02/08 14:15:35 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <vector>
#include "Tile.hpp"
#include "Vec2.hpp"
#include "Snake.hpp"
#include <memory>
#include "Window.hpp"

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
				if (_snake->size() == 0)
					return (Event::DEATH);
				_setTile(Tile::EMPTY, _snake->getHead().pos);
				_generateFood(Tile::RED_APPLE);
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
							SDL_SetRenderDrawColor(renderer, 0,  0, 0, 255); break;
						case Tile::SNAKE_BODY:
							SDL_SetRenderDrawColor(renderer, 0,  169, 64, 255); break;
						case Tile::SNAKE_HEAD:
							SDL_SetRenderDrawColor(renderer, 0,  150, 64, 255); break;
						case Tile::WALL:
							SDL_SetRenderDrawColor(renderer, 128,  128, 128, 255); break;
						case Tile::GREEN_APPLE:
							SDL_SetRenderDrawColor(renderer, 64,  167, 0, 255); break;
						case Tile::RED_APPLE:
							SDL_SetRenderDrawColor(renderer, 167,  0, 0, 255); break;
					}
					SDL_RenderFillRect(renderer, &rectangle);
				}
			}
		}
		int	getSnakeSize()
		{
			return (_snake->size());
		}
		Vec2i	getSnakeHeadPos()
		{
			if (_snake->size() <= 0)
				return (Vec2i(0));
			return (_snake->getHead().pos);
		}
		Vec2i	getClosestFood(Vec2i pos)
		{
			float	smallestDist = 1000000;
			Vec2i	res = Vec2i(0);
			
			for (int x = 0; x < _size.x; x++)
			{
				for (int y = 0; y < _size.y; y++)
				{
					if (_getTile(Vec2i(x, y)) == Tile::GREEN_APPLE)
					{
						if (distance(pos, Vec2i(x, y)) < smallestDist)
						{
							res = Vec2i(x, y);
							smallestDist = distance(pos, Vec2i(x, y));
						}
					}
				}
			}
			return (res);
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
			Vec2i		spawnPos;
			Direction	dir;
			int			length = 2;
			while (1)
			{
				spawnPos.x = rand() % _size.x;
				spawnPos.y = rand() % _size.y;
				dir = static_cast<Direction>(rand() % 4);
				if (_canSpawnSnake(dir, spawnPos, length))
					break ;
			}
			_snake = std::make_unique<Snake>(dir, spawnPos, length);
		}
		bool	_canSpawnSnake(Direction dir, Vec2i headPos, int length)
		{
			if (_getTileNoSnake(headPos) != Tile::EMPTY)
				return (false);

			Vec2i	body_dir = backward(dir);

			for (int i = 0; i < length; i++)
			{
				Vec2i pos(
					headPos.x + body_dir.x * (i + 1),
					headPos.y + body_dir.y * (i + 1)
				);

				if (_getTileNoSnake(pos) != Tile::EMPTY)
					return (false);
			}
			return (true);
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
