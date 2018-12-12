//1. Create Game Window..OK
//2. Draw Board..OK
//3. Draw Stones..OK, Antialiasing..OK
//4. Mouse Click..OK
//5. Apply Texture..OK
//6. Capture Stone..OK
//7. AI(OK), Implement pipeline(OK)
//8. Remove Update Delay..OK
//DONE!
//Thanks for watching!!!

#include <SFML/Graphics.hpp>
using namespace std;
using namespace sf;

#include <Windows.h>
#include <assert.h>
STARTUPINFO sti = { 0 };
SECURITY_ATTRIBUTES sats = { 0 };
PROCESS_INFORMATION pi = { 0 };
HANDLE pipin_w, pipin_r, pipout_w, pipout_r;
BYTE buffer[2048];
DWORD writ, excode, read, avaiable;

void ConnectToEngine(char* path)
{
	pipin_w = pipin_r = pipout_w = pipout_r = NULL;
	sats.nLength = sizeof(sats);
	sats.bInheritHandle = TRUE;
	sats.lpSecurityDescriptor = NULL;

	CreatePipe(&pipout_r, &pipout_w, &sats, 0);
	CreatePipe(&pipin_r, &pipin_w, &sats, 0);

	sti.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	sti.wShowWindow = SW_HIDE;
	sti.hStdInput = pipin_r;
	sti.hStdOutput = pipout_w;
	sti.hStdError = pipout_w;

	assert(CreateProcess(NULL, path, NULL, NULL, TRUE, 0, NULL, NULL, &sti, &pi));
}

string getNextMove(string position)
{
	string str;
	position = "play black " + position + "\ngenmove white\n";
	printf("%s\n", position.c_str());

	WriteFile(pipin_w, position.c_str(), position.length(), &writ, NULL);
	Sleep(500);

	PeekNamedPipe(pipout_r, buffer, sizeof(buffer), &read, &avaiable, NULL);
A:
	do
	{
		ZeroMemory(buffer, sizeof(buffer));
		if (!ReadFile(pipout_r, buffer, sizeof(buffer), &read, NULL) || !read) break;
		buffer[read] = 0;
		str += (char*)buffer;
	} while (read >= sizeof(buffer));
	printf("%s\n", str.c_str());
	int n = str.rfind("=");
	assert(n != string::npos);
	string r = str.substr(n + 1);
	if (r.length() <= 4) goto A;
	return str.substr(n + 1);
}

void CloseConnection()
{
	WriteFile(pipin_w, "quit\n", 5, &writ, NULL);
	if (pipin_w != NULL) CloseHandle(pipin_w);
	if (pipin_r != NULL) CloseHandle(pipin_r);
	if (pipout_w != NULL)CloseHandle(pipout_w);
	if (pipout_r != NULL)CloseHandle(pipout_r);
	if (pi.hProcess != NULL) CloseHandle(pi.hProcess);
	if (pi.hThread != NULL)CloseHandle(pi.hThread);
}


int cell_size = 40;
int board[19][19] = { 0 };
#define BLACK (1)
#define WHITE (2)

bool visit[19][19];

bool live_check(int color, int x, int y)
{
	if (visit[y][x]) return false;  // to remove cycle
	visit[y][x] = true;

	if (board[y][x] == 0)
	{
		return true; // empty space means the dragon is alive
	}
	if (board[y][x] != color)return false;  // captured by enemy stone.

	//recursive search
	bool r = x > 0 && live_check(color, x - 1, y);
	r |= x < 19 - 1 && live_check(color, x + 1, y);
	r |= x > 0 && live_check(color, x, y - 1);
	r |= y < 19 - 1 && live_check(color, x, y + 1);
	return r;
}

int main(void)
{
	ConnectToEngine("gnugo.exe --mode gtp");

	ContextSettings s;
	s.antialiasingLevel = 8;
	RenderWindow window(VideoMode(cell_size * 19, cell_size * 19), "Baduk Game", Style::Default, s);
	Texture t1; t1.loadFromFile("black_stone.bmp");
	Texture t2; t2.loadFromFile("white_stone.bmp");
	t1.setSmooth(true); t2.setSmooth(true);
	Sprite bs(t1), ws(t2);
	bs.setScale(1.0*cell_size / bs.getLocalBounds().width, 1.0*cell_size / bs.getLocalBounds().height);
	ws.setScale(1.0*cell_size / ws.getLocalBounds().width, 1.0*cell_size / bs.getLocalBounds().height);

	auto remove_dead_stones = [&](int color)
	{
		int capture[19][19] = { 0 };
		for (int y = 0; y < 19; y++)for (int x = 0; x < 19; x++)
		{
			if (board[y][x] != color) continue;
			for (int i = 0; i < 19; i++)for (int j = 0; j < 19; j++) visit[i][j] = false;
			if (live_check(color, x, y) == false)capture[y][x] = 1;
		}
		for (int y = 0; y < 19; y++)for (int x = 0; x < 19; x++)
		{
			if (capture[y][x]) board[y][x] = 0;
		}

	};

	auto update = [&]()
	{
		window.clear(Color(255, 207, 97));
		auto draw_board = [&]()
		{
			float half_cell = 1.0 * cell_size / 2;
			for (int y = 0; y < 19; y++)
			{
				Vertex hline[] = {
					Vertex(Vector2f(half_cell, half_cell + y * cell_size)),
					Vertex(Vector2f(cell_size * 19 - half_cell, half_cell + y * cell_size))
				};
				hline[0].color = Color::Black;
				hline[1].color = Color::Black;
				window.draw(hline, 2, Lines);
			}
			for (int x = 0; x < 19; x++)
			{
				Vertex vline[] = {
					Vertex(Vector2f(half_cell + x * cell_size, half_cell)),
					Vertex(Vector2f(half_cell + x * cell_size, cell_size * 19 - half_cell))
				};
				vline[0].color = Color::Black;
				vline[1].color = Color::Black;
				window.draw(vline, 2, Lines);
			}
			// Draw Start Points
			float start_point_r = half_cell / 5;
			CircleShape circle(start_point_r);
			circle.setFillColor(Color::Black);
			for (int y = 0; y < 3; y++)for (int x = 0; x < 3; x++)
			{
				circle.setPosition(half_cell + (3 + 6 * x)*cell_size - start_point_r,
					half_cell + (3 + 6 * y)*cell_size - start_point_r);
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
	};

	while (window.isOpen())
	{
		Event e;
		while (window.pollEvent(e))
		{
			if (e.type == Event::Closed) window.close();
			if (e.type == Event::MouseButtonPressed)
			{
				int ix = e.mouseButton.x / cell_size;
				int iy = e.mouseButton.y / cell_size;

				//Put Black Stone with left click
				if (e.mouseButton.button == Mouse::Left)
				{
					board[iy][ix] = BLACK;
					remove_dead_stones(WHITE);
					update();

					//AI
					char move[10] = { 0 };
					ix += 'A';
					if (ix >= 'I') ix += 1;
					sprintf(move, "%c%d", ix, iy + 1);
					string ret = getNextMove(move);
					assert(ret.length() >= 4);
					sscanf(ret.c_str(), " %c%d\r\n\r\n", &ix, &iy);
					if (ix >= 'J') ix--;
					ix -= 'A';
					board[iy - 1][ix] = WHITE;  // AI will play white stones!
					remove_dead_stones(BLACK);
					update();
				}
			}
		}
		update();
	}
	return 0;
}