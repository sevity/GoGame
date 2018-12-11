#include <SFML/Graphics.hpp>
using namespace std;
using namespace sf;

int cell_size = 40;
int cell_cnt = 19;

int main(void)
{
    RenderWindow window(VideoMode(cell_size*cell_cnt, cell_size*cell_cnt), "Go Game");
    Vertex line[] = 
    {
        Vertex(Vector2f(10,10)),
        Vertex(Vector2f(150,150))
    };
    line[0].color = Color::White;
    line[1].color = Color::White;
    while(window.isOpen())
    {
        Event e;
        while(window.pollEvent(e))
        {
            if(e.type == Event::Closed) window.close();
        }
        window.clear(Color::Yellow);

        auto draw_board = [&]()
        {
            float half_cell = 1.0 * cell_size / 2;
            for(int y=0;y<cell_cnt;y++)
            {
                Vertex hline[] = {Vertex(Vector2f(half_cell, half_cell+y*cell_size)), Vertex(Vector2f(cell_size*cell_cnt-half_cell, half_cell+y*cell_size))};
                hline[0].color = Color::Black;
                hline[1].color = Color::Black;
                window.draw(hline, 2, Lines);
            }
            for(int x=0;x<cell_cnt;x++)
            {
                Vertex vline[] = {Vertex(Vector2f(half_cell+x*cell_size, half_cell)), Vertex(Vector2f(half_cell+x*cell_size, cell_size*cell_cnt-half_cell))};
                vline[0].color = Color::Black;
                vline[1].color = Color::Black;
                window.draw(vline,2,Lines);
            }
            float star_point_r = half_cell/5;
            CircleShape circle(star_point_r);
            circle.setFillColor(Color::Black);
            for(int y=0;y<3;y++)for(int x=0;x<3;x++)
            {
                circle.setPosition(half_cell+(3+6*x)*cell_size-star_point_r, half_cell+(3+6*y)*cell_size-star_point_r);
                window.draw(circle);
            }
        };
        draw_board();
        window.display();

    }

    return 0;
}
