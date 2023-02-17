#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <sstream>
#include "world.hpp"

const double DOUBLE_EPSILON = 1e-5;

bool is_equal_double (const double a, const double b);

/*
    Для компиляции:
        g++ -I C:\msys64\mingw64\include main.cpp player.cpp player_states.cpp -lsfml-window -lsfml-system -lsfml-graphics

    Графика взята отсюда:
        https://rvros.itch.io/animated-pixel-hero
*/


int main() 
{
    /*sf::Font font;
    font.loadFromFile("arial.ttf");
    sf::Text text("time", font);
    text.setCharacterSize(30);
    text.setStyle(sf::Text::Bold);
    text.setFillColor(sf::Color::Red); */

    sf::ContextSettings settings;
    settings.antialiasingLevel = 8.0;
    sf::RenderWindow window(sf::VideoMode(1200, 900), "Player states", sf::Style::Close, settings);
    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(60);

    double time = 0;
    double dt = 1.0 / 60;
    int last_time = 0;

    World world;
    world.addBlock({-500, 770, 20000, 400});
    world.addBlock({-400, 100, 700, 300});
    world.addBlock({600, 500, 300, 120});
    world.addBlock({800, 0, 400, 200});
    world.addBlock({-100, -700, 400, 100});
    world.addBlock({700, -700, 400, 100});
    world.addBlock({1500, -700, 400, 100});
    world.addBlock({1100, -300, 400, 100});
    world.addBlock({1100, 400, 400, 400});
    world.addBlock({1900, -100, 200, 800});
    world.addBlock({3000, 500, 1000, 200});

    while (window.isOpen()) 
    {
        //text.setPosition(400, 400);

        sf::Event event;
        while(window.pollEvent(event)) 
        {
            if(event.type == sf::Event::Closed) 
                window.close();

            world.handleEvents(event);
        }
        window.clear(sf::Color::Black);
        world.update(dt);
        world.draw(window);
        //window.draw(text);
        window.display();

        time += dt;  /*
        if ((int)time > last_time)
        {
            std::ostringstream playerScoreString;
            playerScoreString << (int)time;
            text.setString("Time:" + playerScoreString.str());
            cout << (int)time << endl;
            last_time = time;
        }  */
        
    }

    return 0;
}