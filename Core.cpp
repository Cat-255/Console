#include "Console.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#ifdef byte
#undef byte
#endif
#endif

#include <SFML/Graphics.hpp>
#include <optional>
#include <string>
#include <iostream>

using namespace std;

int main()
{
#ifdef _WIN32
    if (HWND hwnd = GetConsoleWindow())
        ShowWindow(hwnd, SW_HIDE);
#endif

    Console app;
    app.loop = 1;

    try
    {
        sf::RenderWindow window(sf::VideoMode({ 600u, 500u }), "Console ALPHA 1.0", sf::Style::Titlebar | sf::Style::Close);
        window.setFramerateLimit(60);

        app.print("Hi! This is console version 1.0. Type \"/help\" for help");

        sf::Image icon;
        if (icon.loadFromFile("Icon.png")) window.setIcon(icon);
        else
        {
        #ifdef _WIN32
            MessageBeep(MB_ICONERROR);
        #endif
        }

        sf::Font font;
        if (!font.openFromFile("Fonts/Courier_new.ttf"))
        {
        #ifdef _WIN32
            MessageBeep(MB_ICONERROR);
        #endif
            return -1;
        }

        font.setSmooth(true);

        {
            sf::Text preload(font);
            preload.setCharacterSize(14);
            preload.setString("Aa");
            preload.setCharacterSize(15);
            preload.setString("Aa");
            preload.setCharacterSize(16);
            preload.setString("Aa");
        }

        font.setSmooth(true);

        INPUT_PROCESSOR input_widget(575.0f, 28.0f, 15, font);
        input_widget.set_position({ 8.0f, 463.0f });

        OUTPUT_PROCESSOR output_widget(584.f, 420.f, 14, font);
        output_widget.set_position({ 8.f, 12.f });
        output_widget.set_history(app.history);
        output_widget.scroll_to_bottom();

        auto cursor_text = sf::Cursor::createFromSystem(sf::Cursor::Type::Text);
        auto cursor_arrow = sf::Cursor::createFromSystem(sf::Cursor::Type::Arrow);

        while (app.loop && window.isOpen())
        {
            while (const std::optional event = window.pollEvent())
            {
                if (event->is<sf::Event::Closed>())
                {
                    window.close();
                    continue;
                }
                output_widget.input_handler(*event);
                input_widget.input_handler(*event, window);

                if (const auto* key = event->getIf<sf::Event::KeyPressed>())
                {
                    if (key->code == sf::Keyboard::Key::Enter)
                    {
                        string current_input = input_widget.getText().toAnsiString();
                        if (!current_input.empty())
                        {
                            if (current_input[0] == '/')
                                app.process_input(current_input);
                            else
                                app.print(current_input);

                            input_widget.clear();
                            output_widget.scroll_to_bottom();
                        }
                    }
                }
            }

            window.clear(sf::Color::Black);

            output_widget.draw(window);

            const sf::Color border_color_dark(75, 75, 75);
            const sf::Color border_color_light(145, 145, 145);

            auto drawBorder = [&](sf::Vector2f size, sf::Vector2f pos, sf::Color color)
                {
                    sf::RectangleShape r(size);
                    r.setPosition(pos);
                    r.setFillColor(color);
                    window.draw(r);
                };

            drawBorder({ 584.f, 1.f }, { 8.f, 8.f }, border_color_dark);
            drawBorder({ 584.f, 1.f }, { 8.f, 436.f }, border_color_light);
            drawBorder({ 1.f, 428.f }, { 8.f, 8.f }, border_color_dark);
            drawBorder({ 1.f, 428.f }, { 591.f, 8.f }, border_color_light);

            drawBorder({ 584.f, 1.f }, { 8.f, 468.f }, border_color_dark);
            drawBorder({ 584.f, 1.f }, { 8.f, 491.f }, border_color_light);
            drawBorder({ 1.f, 23.f }, { 8.f, 468.f }, border_color_dark);
            drawBorder({ 1.f, 23.f }, { 591.f, 468.f }, border_color_light);

            input_widget.draw(window);

            sf::Vector2f mouse = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));
            if (input_widget.contains(mouse))
            {
                if (cursor_text) window.setMouseCursor(*cursor_text);
            }
            else
            {
                if (cursor_arrow) window.setMouseCursor(*cursor_arrow);
            }

            window.display();
        }
    }
    catch (const exception& error)
    {
    #ifdef _WIN32
        MessageBeep(MB_ICONERROR);
    #endif
        cerr << "ERROR: " << error.what() << endl;
        return -1;
    }

    return 0;
}
