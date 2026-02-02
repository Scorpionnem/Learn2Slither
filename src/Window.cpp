/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Window.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/02 10:26:45 by mbatty            #+#    #+#             */
/*   Updated: 2026/02/02 10:46:26 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Window.hpp"
#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_sdlrenderer2.h>

void	Window::open(uint32_t width, uint32_t height, const std::string &title)
{
	_width = width;
	_height = height;
	_title = title;

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
		throw (std::runtime_error(SDL_GetError()));

	_window = SDL_CreateWindow(_title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _width, _height,
				SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (!_window)
	{
		SDL_Quit();
		throw (std::runtime_error(SDL_GetError()));
	}
	_renderer = SDL_CreateRenderer(_window, -1, 0);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	(void)io;
	ImGui::StyleColorsDark();

	ImGui_ImplSDL2_InitForSDLRenderer(_window, _renderer);
	ImGui_ImplSDLRenderer2_Init(_renderer);
}

void	Window::close()
{
	ImGui_ImplSDLRenderer2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	SDL_DestroyRenderer(_renderer);
	SDL_DestroyWindow(_window);
	SDL_Quit();
}

void	Window::pollEvents()
{
	SDL_RenderClear(_renderer);
	SDL_Event	event;

	_events.reset();
	while (SDL_PollEvent(&event))
	{
		ImGui_ImplSDL2_ProcessEvent(&event);
		switch (event.type)
		{
			case SDL_QUIT:
				_running = false;
				break ;
			case SDL_WINDOWEVENT:
				if (event.window.event == SDL_WINDOWEVENT_RESIZED)
				{
					_width = event.window.data1;
					_height = event.window.data2;
				}
				break ;
			case SDL_KEYDOWN:
				_events.setKey(event.key.keysym.sym, true);
				_events.setKeyPressed(event.key.keysym.sym);
				break ;
			case SDL_KEYUP:
				_events.setKey(event.key.keysym.sym, false);
				_events.setKeyReleased(event.key.keysym.sym);
				break ;
			case SDL_MOUSEMOTION:
				_events.setMouseDeltaX(event.motion.xrel);
				_events.setMouseDeltaY(event.motion.yrel);
				break ;
			case SDL_MOUSEBUTTONDOWN:
				_events.setMouseBtn(event.button.button, true);
				_events.setMouseBtnPressed(event.button.button);
				break ;
			case SDL_MOUSEBUTTONUP:
				_events.setMouseBtn(event.button.button, false);
				_events.setMouseBtnLifted(event.button.button);
				break ;
			case SDL_MOUSEWHEEL:
				_events.setMouseScroll(event.wheel.y);
				break ;
		}
	}
}

void	Window::display()
{
	SDL_RenderPresent(_renderer);
}
