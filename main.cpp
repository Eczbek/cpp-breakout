#include <chrono> // std::chrono::milliseconds
#include <iostream> // std::cout
#include <random> // std::mt19937, std::random_device, std::uniform_int_distribution
#include <string> // std::string
#include <thread> // std::this_thread::sleep_for
#include <vector> // std::vector
#include <xieite/console/Canvas.hpp>
#include <xieite/console/RawLock.hpp>
#include <xieite/console/codes.hpp>
#include <xieite/console/readBuffer.hpp>
#include <xieite/graphics/Color.hpp>
#include <xieite/graphics/colors.hpp>

struct Position {
	int x;
	int y;

	Position(const int x, const int y) noexcept
	: x(x), y(y) {}
};

int main() {
	const std::vector<xieite::graphics::Color> bricks {
		xieite::graphics::colors::lime,
		xieite::graphics::colors::yellow,
		xieite::graphics::colors::orange,
		xieite::graphics::colors::red
	};

	const int worldWidth = 20;
	const int worldHeight = 20;
	std::vector<std::vector<int>> world(worldWidth);
	for (int x = 0; x < worldWidth; ++x)
		for (int y = 0; y < worldHeight; ++y)
			world[x].push_back((y >= worldHeight - bricks.size() && x % 2 != y % 2)
				? (bricks.size() - worldHeight + y + 1)
				: 0);

	int paddleSize = 3;
	int initialPaddlePosition = worldWidth / 2 - paddleSize / 2;
	Position paddle(initialPaddlePosition, 1);
	Position ball(initialPaddlePosition + paddleSize / 2, 3);
	std::mt19937 rng(std::random_device {}());
	Position ballDirection(std::uniform_int_distribution<int>(0, 1)(rng) * 2 - 1, 1);

	std::cout
		<< xieite::console::saveScreen
		<< xieite::console::hideCursor;

	bool running = true;
	while (running) {
		xieite::console::Canvas canvas(worldWidth, worldHeight, xieite::graphics::colors::darkGray);
		for (int y = worldHeight; y--;)
			for (int x = 0; x < worldWidth; ++x)
				if (world[x][y])
					canvas.draw(x, y, bricks[world[x][y] - 1]);
		for (int x = paddle.x; x < paddle.x + paddleSize; ++x)
			canvas.draw(x, paddle.y, xieite::graphics::colors::azure);
		canvas.draw(ball.x, ball.y, xieite::graphics::colors::red);

		std::cout
			<< xieite::console::eraseScreen
			<< canvas.string(0, 0);
		std::cout.flush();
		{
			xieite::console::RawLock rawLock(false);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		const std::string input = xieite::console::readBuffer();
		if (input.size())
			switch (input.back()) {
			case 'd':
				if (paddle.x < worldWidth - paddleSize)
					++paddle.x;
				break;
			case 'a':
				if (paddle.x)
					--paddle.x;
				break;
			case 'q':
				running = false;
			}
		
		if (ball.x + ballDirection.x < 0 || ball.x + ballDirection.x >= worldWidth - 1)
			ballDirection.x = -ballDirection.x;
		if (ball.y == worldHeight - 1)
			ballDirection.y = -1;
		else if (ball.y == paddle.y + 1 && ball.x + ballDirection.x >= paddle.x && ball.y + ballDirection.x <= paddle.x + paddleSize)
			ballDirection.y = 1;
		else if (!ball.y)
			break;
		else if (world[ball.x + ballDirection.x][ball.y + 1]) {
			ballDirection.y = -1;
			--world[ball.x + ballDirection.x][ball.y + 1];
		}
		ball.x += ballDirection.x;
		ball.y += ballDirection.y;
	}

	std::cout
		<< xieite::console::showCursor
		<< xieite::console::restoreScreen;
}
