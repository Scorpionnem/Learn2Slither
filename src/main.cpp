/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/30 10:57:51 by mbatty            #+#    #+#             */
/*   Updated: 2026/02/08 15:41:05 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Game.hpp"
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

	const std::map<std::string, std::string&>	arg_options =	{{"--export", export_path}, {"-e", export_path},
																 {"--import", import_path}, {"-i", import_path},};
	const std::map<std::string, int&>			int_options =	{{"--epochs", epochs}, {"-E", epochs},
																 {"--width", width}, {"-W", width},
																 {"--height", height}, {"-H", height}};
	const std::map<std::string, bool&>			bool_options =	{{"--help", help}, {"-h", help},
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
				return (std::cerr << arg << " needs an argument" << std::endl, 1);
			arg_options.find(arg)->second = av[i + 1];
			i++;
		}
		else if (int_options.find(arg) != int_options.end())
		{
			if (!av[i + 1])
				return (std::cerr << arg << " needs an argument" << std::endl, 1);
			int_options.find(arg)->second = std::atoi(av[i + 1]);
			i++;
		}
		else if (bool_options.find(arg) != bool_options.end())
			bool_options.find(arg)->second = true;
		else
			return (std::cerr << "Unknown option (use --help)" << std::endl, 1);
	}

	if (step && !visualize)
		return (std::cerr << "Step mode requires visualize option" << std::endl, 1);
	if (epochs <= 0 || (width < 8 && width > 100) || (height > 100 && height < 8))
		return (std::cerr << "Invalid input" << std::endl, 1);
	if (help) { print_help(); return (0); }

	Window	window;
	if (visualize)
		window.open(width * TILE_SIZE, height * TILE_SIZE, "TROP BIEN LE SNAKE IL APPREND");

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

	int	avg = 0;
	if (lengths.size())
	{
		for (int len : lengths)
			avg += len;
		avg /= lengths.size();
	}
	std::cout << "Average size: " << avg << std::endl;
	std::cout << "Max size: " << game.getMaxSize() << std::endl;
	return (0);
}
