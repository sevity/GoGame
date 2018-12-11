#include <SFML/Graphics.hpp>
#include "Connector.hpp"


using namespace std;
using namespace sf;

const int cell_size = 50;
const int cell_cnt = 19;
int board[19][19] = { 0 };
#define BLACK (1)
#define WHITE (2)

bool visit[19][19];

bool live_check(int color, int x, int y)
{
	if (visit[y][x]) return false;  // to remove cycle
	visit[y][x] = true;

	if (board[y][x] == 0) {
		return true; //empty space means the dragon is alive
	}
	if (board[y][x] != color) return false; //captured by enermy stone.

	bool r = x > 0 && live_check(color, x - 1, y);
	r |= x < 19 - 1 && live_check(color, x + 1, y);
	r |= y > 0 && live_check(color, x, y - 1);
	r |= y < 19 - 1 && live_check(color, x, y + 1);
	return r;
};

int main(void)
{
	ConnectToEngine("gnugo.exe --mode gtp");


	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;
	RenderWindow window(VideoMode(cell_size*cell_cnt, cell_size*cell_cnt), "Go Game", Style::Default, settings);
	Texture t2; t2.loadFromFile("white_stone.bmp");
	Texture t1; t1.loadFromFile("black_stone.bmp");
	t1.setSmooth(true); t2.setSmooth(true);
	Sprite bs(t1), ws(t2);
	bs.setScale(1.0*cell_size / bs.getLocalBounds().width, 1.0*cell_size / bs.getLocalBounds().height);
	ws.setScale(1.0*cell_size / ws.getLocalBounds().width, 1.0*cell_size / ws.getLocalBounds().height);

	Vertex line[] =
	{
		Vertex(Vector2f(10,10)),
		Vertex(Vector2f(150,150))
	};
	line[0].color = Color::White;
	line[1].color = Color::White;
	int xx = 0, yy = 0;

	auto remove_dead_stones = [&](int color)
	{
		int capture[19][19] = { 0 };
		for (int y = 0; y < 19; y++)for (int x = 0; x < 19; x++)
		{
			if (board[y][x] != color) continue;
			for (int i = 0; i < 19; i++)for (int j = 0; j < 19; j++) visit[i][j] = false;
			if (live_check(color, x, y) == false) capture[y][x] = 1;
		}
		for (int y = 0; y < 19; y++)for (int x = 0; x < 19; x++)
		{
			if (capture[y][x])
				board[y][x] = 0;
		}
	};
	while (window.isOpen())
	{
		Event e;
		while (window.pollEvent(e))
		{
			if (e.type == Event::Closed)
			{
				CloseConnection();
				window.close();
			}
			if (e.type == Event::MouseButtonPressed)
			{
				printf("mouse button pressed!\n");
				if (e.mouseButton.button == Mouse::Left)
				{
					int ix = e.mouseButton.x / cell_size;
					int iy = e.mouseButton.y / cell_size;
					board[iy][ix] = BLACK;
					remove_dead_stones(WHITE);

					//AI
					char move[10] = { 0 };
					ix += 'A';
					if (ix >= 'I') ix += 1;
					sprintf(move, "%c%d", ix, iy + 1);
					string ret = getNextMove(move);
					printf("::%s\n", ret.c_str());
					assert(ret.length() >= 4);
					sscanf(ret.c_str(), " %c%d\r\n\r\n", &ix, &iy);
					if (ix >= 'J') ix--;
					ix -= 'A';
					board[iy - 1][ix] = WHITE;
					remove_dead_stones(BLACK);
				}
				if (e.mouseButton.button == Mouse::Right)
				{
					int ix = e.mouseButton.x / cell_size;
					int iy = e.mouseButton.y / cell_size;
					board[iy][ix] = WHITE;
					remove_dead_stones(BLACK);
				}

			}
		}
		window.clear(Color(255, 207, 97));

		auto draw_board = [&]()
		{
			float half_cell = 1.0 * cell_size / 2;
			for (int y = 0; y < cell_cnt; y++)
			{
				Vertex hline[] = { Vertex(Vector2f(half_cell, half_cell + y * cell_size)), Vertex(Vector2f(cell_size*cell_cnt - half_cell, half_cell + y * cell_size)) };
				hline[0].color = Color::Black;
				hline[1].color = Color::Black;
				window.draw(hline, 2, Lines);
			}
			for (int x = 0; x < cell_cnt; x++)
			{
				Vertex vline[] = { Vertex(Vector2f(half_cell + x * cell_size, half_cell)), Vertex(Vector2f(half_cell + x * cell_size, cell_size*cell_cnt - half_cell)) };
				vline[0].color = Color::Black;
				vline[1].color = Color::Black;
				window.draw(vline, 2, Lines);
			}
			float star_point_r = half_cell / 5;
			CircleShape circle(star_point_r);
			circle.setFillColor(Color::Black);
			for (int y = 0; y < 3; y++)for (int x = 0; x < 3; x++)
			{
				circle.setPosition(half_cell + (3 + 6 * x)*cell_size - star_point_r, half_cell + (3 + 6 * y)*cell_size - star_point_r);
				window.draw(circle);
			}
		};
		draw_board();

		auto draw_stones = [&]()
		{
			for (int y = 0; y < 19; y++)for (int x = 0; x < 19; x++)
			{
				if (board[y][x] == BLACK)
				{
					bs.setPosition(x*cell_size, y*cell_size);
					window.draw(bs);
				}
				if (board[y][x] == WHITE)
				{
					ws.setPosition(x*cell_size, y*cell_size);
					window.draw(ws);
				}
			}
		};
		draw_stones();
		window.display();

	}

	return 0;
}
