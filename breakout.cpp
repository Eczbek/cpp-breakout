
#include <chrono>
#include <fcntl.h>
#include <iostream>
#include <random>
#include <string>
#include <termios.h>
#include <thread>
#include <unistd.h>
#include <vector>


struct Position {
	int x;
	int y;
};

int main() {
	constexpr int width = 20;
	constexpr int height = 20;

	constexpr int bricks = 4;
	std::vector<std::vector<int>> world;
	for (int x = 0; x < width; ++x) {
		std::vector<int> column;
		for (int y = 0; y < height; ++y)
			column.push_back(y >= height - bricks ? bricks - (height - y - 1) : 0);
		world.push_back(column);
	}

	std::mt19937 rng(std::random_device {}());

	int size = 3;
	int startX = std::uniform_int_distribution<>(0, width - size)(rng);
	Position paddle = { startX, 1 };
	Position ball = { startX + size / 2, 3 };
	Position direction = { std::uniform_int_distribution<>(0, 1)(rng) * 2 - 1, 1 };

	int score = 0;
	bool running = true;

	termios cooked;
	tcgetattr(STDIN_FILENO, &cooked);
	termios raw = cooked;
	cfmakeraw(&raw);
	tcsetattr(STDIN_FILENO, TCSANOW, &raw);
	fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) | O_NONBLOCK);
	std::cout << "\033[?25l";

	while (running) {
		std::string display;
		for (int y = height - 1; y >= 0; --y) {
			for (int x = 0; x < width; ++x)
				display += world[x][y] ? std::to_string(world[x][y]) + ' ' : ". ";
			display += "\r\n";
		}
		const std::size_t max = display.length() - 1;
		for (int x = 0; x < size; ++x)
			display[max - (paddle.y + 1) * width * 2 + (paddle.x + x) * 2 - paddle.y * 2 - 1] = '#';
		display[max - (ball.y + 1) * width * 2 + ball.x * 2 - ball.y * 2 - 1] = 'O';

		std::cout << "\033[2J\033[HScore: " << score << "\r\n" << display;
		std::cout.flush();

		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		char input = 0;
		while (read(STDIN_FILENO, &input, 1) == 1);

		switch (input) {
			case 'd':
				if (paddle.x < width - size)
					++paddle.x;
				break;
			case 'a':
				if (paddle.x > 0)
					--paddle.x;
				break;
			case 'q':
				running = false;
		}

		if (!ball.x || ball.x == width - 1)
			direction.x = -direction.x;
		if (ball.y == height - 1)
			direction.y = -1;
		else if (ball.y == 2 && (direction.x == 1 && ball.x >= paddle.x - 1 && ball.x < paddle.x + size || direction.x == -1 && ball.x >= paddle.x && ball.x <= paddle.x + size))
				direction.y = 1;
		else if (!ball.y)
			break;
		else if (world[ball.x][ball.y + 1]) {
			direction.y = -1;
			score += world[ball.x][ball.y + 1];
			--world[ball.x][ball.y + 1];
		} else if (world[ball.x + direction.x][ball.y + 1]) {
			direction.y = -1;
			score += world[ball.x + direction.x][ball.y + 1];
			--world[ball.x + direction.x][ball.y + 1];
		}
		ball.x += direction.x;
		ball.y += direction.y;
	}

	tcsetattr(STDIN_FILENO, TCSANOW, &cooked);
	fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) & ~O_NONBLOCK);
	std::cout << "\033[?25h";
}
