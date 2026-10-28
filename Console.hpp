#ifndef CONSOLE_HPP
#define CONSOLE_HPP

#include <SFML/Window/Clipboard.hpp>
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Console
{
public:
    bool loop;
    string user_text;
    vector<string> history;
    sf::Font font;

    void print(string user_text);
    void process_input(const string& input);

    struct Command_info
    {
        using CommandFunc = void (Console::*)(const vector<string>&, const string&);

        CommandFunc command;
        string description;
        string add_desc;
    };

    unordered_map<string, Command_info> cd_list;

    void cd_echo(const vector<string>& args, const string& arg);
    void cd_time(const vector<string>& args, const string& arg);
    void cd_create(const vector<string>& args, const string& arg);
    void cd_help(const vector<string>& args, const string& arg);
    void cd_clear(const vector<string>& args, const string& arg);
    void cd_debugger(const vector<string>& args, const string& arg);
    void cd_info(const vector<string>& args, const string& arg);
    void cd_exit(const vector<string>& args, const string& arg);

    Console();

private:
    vector<string> text_splitter(const string& text, size_t max_chars = 72);
    vector<string> parse_args(const string& input);
};

class INPUT_PROCESSOR
{
private:
    sf::RectangleShape box;
    sf::Text text_display;
    std::optional<sf::Sprite> render_sprite;
    sf::String buffer;

    bool is_Selected = false;

    size_t cursor_pos = 0;
    size_t sel_start = 0;
    size_t sel_end = 0;
    bool selecting = false;

    sf::View view;
    sf::RenderTexture render_texture;
    float scroll_x = 0.0f;

    sf::Clock cursor_clock;
    bool cursor_visible = true;
    const float blink_time = 0.53f;

    float box_width = 0.0f;
    float box_height = 0.0f;
    unsigned int char_size = 14;

    bool Has_selection() const
    {
        return sel_start != sel_end;
    }

    void Clear_selection()
    {
        sel_start = sel_end = cursor_pos;
    }

    void Delete_selection()
    {
        if (!Has_selection()) return;

        size_t left = std::min(sel_start, sel_end);
        size_t right = std::max(sel_start, sel_end);

        buffer.erase(left, right - left);
        cursor_pos = left;
        Clear_selection();
        Update_text();
    }

    float Get_char_offset(size_t index) const
    {
        if (index == 0) return 0.0f;

        sf::Text temp(text_display);
        temp.setString(buffer.substring(0, index));
        return temp.getLocalBounds().size.x;
    }

    size_t Get_index_from_x(float local_x) const
    {
        local_x += scroll_x;
        if (buffer.isEmpty()) return 0;

        size_t best = 0;
        float best_dist = 99999.0f;

        for (size_t i = 0; i <= buffer.getSize(); ++i)
        {
            float x = Get_char_offset(i);
            float dist = std::abs(x - local_x);
            if (dist < best_dist)
            {
                best_dist = dist;
                best = i;
            }
        }
        return best;
    }

    void Update_text()
    {
        text_display.setString(buffer);
        Ensure_cursor_visible();
    }

    void Ensure_cursor_visible()
    {
        float cursor_x = Get_char_offset(cursor_pos);
        float visible_left = scroll_x;
        float visible_right = scroll_x + box_width - 12.0f;

        if (cursor_x < visible_left)
            scroll_x = std::max(0.0f, cursor_x - 20.0f);
        else if (cursor_x > visible_right)
            scroll_x = cursor_x - box_width + 40.0f;

        float text_w = text_display.getLocalBounds().size.x;
        float max_scroll = std::max(0.0f, text_w - box_width + 20.0f);

        if (scroll_x < 0.0f) scroll_x = 0.0f;
        if (scroll_x > max_scroll) scroll_x = max_scroll;

        view.setCenter({ scroll_x + box_width / 2.0f, box_height / 2.0f });
    }

public:
    INPUT_PROCESSOR(float width, float height, unsigned int size, sf::Font& font)
        : text_display(font)
    {
        box_width = width;
        box_height = height;
        char_size = size;

        box.setSize({ width, height });
        box.setFillColor(sf::Color::Transparent);

        text_display.setCharacterSize(size);
        text_display.setFillColor(sf::Color(0, 255, 0));
        text_display.setPosition({ 8.0f, 8.0f });

        if (!render_texture.resize({ static_cast<unsigned int>(width), static_cast<unsigned int>(height) }))
        {
            // ERROR
        }

        render_texture.setSmooth(1);

        view.setSize({ width, height });
        view.setCenter({ width / 2.0f, height / 2.0f });

        render_sprite.emplace(render_texture.getTexture());
    }

    bool contains(sf::Vector2f point) const
    {
        return box.getGlobalBounds().contains(point);
    }

    void set_position(sf::Vector2f pos)
    {
        box.setPosition(pos);
        if (render_sprite)
            render_sprite->setPosition(pos);
    }

    void input_handler(const sf::Event& event, const sf::RenderWindow& window)
    {
        if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>())
        {
            if (mb->button == sf::Mouse::Button::Left)
            {
                sf::Vector2f mouse(static_cast<float>(mb->position.x),
                    static_cast<float>(mb->position.y));

                if (box.getGlobalBounds().contains(mouse))
                {
                    is_Selected = true;
                    float local_x = mouse.x - box.getPosition().x;
                    cursor_pos = Get_index_from_x(local_x);
                    sel_start = sel_end = cursor_pos;
                    selecting = true;
                    cursor_clock.restart();
                    cursor_visible = true;
                }
                else
                {
                    is_Selected = false;
                    selecting = false;
                }
            }
        }

        if (const auto* mb = event.getIf<sf::Event::MouseButtonReleased>())
        {
            if (mb->button == sf::Mouse::Button::Left)
                selecting = false;
        }

        if (const auto* mm = event.getIf<sf::Event::MouseMoved>())
        {
            if (selecting && is_Selected)
            {
                float local_x = static_cast<float>(mm->position.x) - box.getPosition().x;
                cursor_pos = Get_index_from_x(local_x);
                sel_end = cursor_pos;
                Ensure_cursor_visible();
            }
        }

        if (!is_Selected) return;

        if (const auto* key = event.getIf<sf::Event::KeyPressed>())
        {
            const bool ctrl = key->control;
            const bool shift = key->shift;

            if (ctrl && key->code == sf::Keyboard::Key::C)
            {
                if (Has_selection())
                {
                    size_t left = std::min(sel_start, sel_end);
                    size_t right = std::max(sel_start, sel_end);
                    sf::Clipboard::setString(buffer.substring(left, right - left));
                    sf::String clip = sf::Clipboard::getString();
                    for (size_t i = 0; i < clip.getSize(); ++i)
                    {
                        if (clip[i] == '\n' || clip[i] == '\r')
                            clip[i] = ' ';
                    }
                    buffer.insert(cursor_pos, clip);
                }
                return;
            }

            if (ctrl && key->code == sf::Keyboard::Key::V)
            {
                Delete_selection();
                sf::String clip = sf::Clipboard::getString();
                buffer.insert(cursor_pos, clip);
                cursor_pos += clip.getSize();
                Clear_selection();
                Update_text();
                return;
            }

            if (ctrl && key->code == sf::Keyboard::Key::X)
            {
                if (Has_selection())
                {
                    size_t left = std::min(sel_start, sel_end);
                    size_t right = std::max(sel_start, sel_end);
                    sf::Clipboard::setString(buffer.substring(left, right - left));
                    Delete_selection();
                }
                return;
            }

            if (ctrl && key->code == sf::Keyboard::Key::A)
            {
                sel_start = 0;
                sel_end = buffer.getSize();
                cursor_pos = buffer.getSize();
                cursor_clock.restart();
                cursor_visible = true;
                return;
            }

            if (key->code == sf::Keyboard::Key::Left)
            {
                if (cursor_pos > 0) --cursor_pos;
                if (shift) sel_end = cursor_pos;
                else Clear_selection();
                cursor_clock.restart();
                cursor_visible = true;
                Ensure_cursor_visible();
                return;
            }

            if (key->code == sf::Keyboard::Key::Right)
            {
                if (cursor_pos < buffer.getSize()) ++cursor_pos;
                if (shift) sel_end = cursor_pos;
                else Clear_selection();
                cursor_clock.restart();
                cursor_visible = true;
                Ensure_cursor_visible();
                return;
            }

            if (key->code == sf::Keyboard::Key::Home)
            {
                cursor_pos = 0;
                if (shift) sel_end = cursor_pos;
                else Clear_selection();
                Ensure_cursor_visible();
                return;
            }

            if (key->code == sf::Keyboard::Key::End)
            {
                cursor_pos = buffer.getSize();
                if (shift) sel_end = cursor_pos;
                else Clear_selection();
                Ensure_cursor_visible();
                return;
            }

            if (key->code == sf::Keyboard::Key::Delete)
            {
                if (Has_selection()) Delete_selection();
                else if (cursor_pos < buffer.getSize())
                {
                    buffer.erase(cursor_pos, 1);
                    Update_text();
                }
                return;
            }

            if (key->code == sf::Keyboard::Key::Backspace)
            {
                if (Has_selection()) Delete_selection();
                else if (cursor_pos > 0)
                {
                    buffer.erase(cursor_pos - 1, 1);
                    --cursor_pos;
                    Clear_selection();
                    Update_text();
                }
                cursor_clock.restart();
                cursor_visible = true;
                return;
            }
        }

        if (const auto* text = event.getIf<sf::Event::TextEntered>())
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RControl))
                return;

            if (text->unicode >= 32 && text->unicode != 127)
            {
                Delete_selection();
                buffer.insert(cursor_pos, sf::String(static_cast<char32_t>(text->unicode)));
                ++cursor_pos;
                Clear_selection();
                Update_text();
            }

            cursor_clock.restart();
            cursor_visible = true;
        }

        if (const auto* wheel = event.getIf<sf::Event::MouseWheelScrolled>())
        {
            if (!is_Selected) return;

            scroll_x -= wheel->delta * 40.0f;
            float text_w = text_display.getLocalBounds().size.x;
            float max_scroll = std::max(0.0f, text_w - box_width + 20.0f);
            if (scroll_x < 0.0f) scroll_x = 0.0f;
            if (scroll_x > max_scroll) scroll_x = max_scroll;
            view.setCenter({ scroll_x + box_width / 2.0f, box_height / 2.0f });
        }
    }

    void clear()
    {
        buffer.clear();
        cursor_pos = 0;
        Clear_selection();
        scroll_x = 0.0f;
        Update_text();
    }

    sf::String getText() const { return buffer; }

    void draw(sf::RenderWindow& window)
    {
        if (is_Selected && cursor_clock.getElapsedTime().asSeconds() > blink_time)
        {
            cursor_visible = !cursor_visible;
            cursor_clock.restart();
        }

        const sf::Vector2f boxPos = box.getPosition();
        const sf::Vector2u winSize = window.getSize();

        const float sx = boxPos.x / static_cast<float>(winSize.x);
        const float sy = boxPos.y / static_cast<float>(winSize.y);
        const float sw = box_width / static_cast<float>(winSize.x);
        const float sh = box_height / static_cast<float>(winSize.y);

        sf::View current = window.getView();
        const sf::FloatRect oldScissor = current.getScissor();

        current.setScissor(sf::FloatRect({ sx, sy }, { sw, sh }));
        window.setView(current);

        const float baseX = boxPos.x + 8.0f - scroll_x;
        const float baseY = boxPos.y + 8.0f;

        if (Has_selection() && is_Selected)
        {
            size_t left = std::min(sel_start, sel_end);
            size_t right = std::max(sel_start, sel_end);
            float x1 = Get_char_offset(left);
            float x2 = Get_char_offset(right);

            sf::RectangleShape sel({ x2 - x1, static_cast<float>(char_size) + 4.0f });
            sel.setPosition({ baseX + x1, baseY });
            sel.setFillColor(sf::Color(0, 128, 0));
            window.draw(sel);
        }

        text_display.setPosition({ baseX, baseY });
        window.draw(text_display);

        if (Has_selection() && is_Selected)
        {
            size_t left = std::min(sel_start, sel_end);
            size_t right = std::max(sel_start, sel_end);
            if (right > left)
            {
                sf::Text selected_text(text_display);
                selected_text.setString(buffer.substring(left, right - left));
                selected_text.setFillColor(sf::Color::White);
                selected_text.setPosition({ baseX + Get_char_offset(left), baseY });
                window.draw(selected_text);
            }
        }

        if (is_Selected && cursor_visible)
        {
            float cx = Get_char_offset(cursor_pos);
            sf::RectangleShape caret({ 1.0f, static_cast<float>(char_size) + 2.0f });
            caret.setPosition({ baseX + cx, baseY });
            caret.setFillColor(sf::Color(0, 255, 0));
            window.draw(caret);
        }

        current.setScissor(oldScissor);
        window.setView(current);
    }
};

class OUTPUT_PROCESSOR
{
private:
    sf::RectangleShape box;
    sf::Text text_drawer;
    const std::vector<std::string>* history = nullptr;

    float box_width = 0.f;
    float box_height = 0.f;
    float line_height = 16.f;
    int scroll_lines = 0;
    bool is_Selected = false;

public:
    OUTPUT_PROCESSOR(float width, float height, unsigned int char_size, sf::Font& font)
        : text_drawer(font)
    {
        box_width = width;
        box_height = height;

        box.setSize({ width, height });
        box.setFillColor(sf::Color::Transparent);

        text_drawer.setCharacterSize(char_size);
        text_drawer.setFillColor(sf::Color(0, 255, 0));
    }

    void set_history(const std::vector<std::string>& h) { history = &h; }

    void set_position(sf::Vector2f pos) { box.setPosition(pos); }

    bool contains(sf::Vector2f p) const
    {
        return box.getGlobalBounds().contains(p);
    }

    void input_handler(const sf::Event& event)
    {
        if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>())
        {
            if (mb->button == sf::Mouse::Button::Left)
            {
                sf::Vector2f mouse(
                    static_cast<float>(mb->position.x),
                    static_cast<float>(mb->position.y));
                is_Selected = box.getGlobalBounds().contains(mouse);
            }
        }

        if (!is_Selected || !history) return;

        if (const auto* wheel = event.getIf<sf::Event::MouseWheelScrolled>())
        {
            scroll_lines -= static_cast<int>(wheel->delta);

            const int max_visible = static_cast<int>(box_height / line_height);
            const int max_scroll = std::max(0, static_cast<int>(history->size()) - max_visible);

            if (scroll_lines < 0) scroll_lines = 0;
            if (scroll_lines > max_scroll) scroll_lines = max_scroll;
        }
    }

    void scroll_to_bottom()
    {
        if (!history) return;
        const int max_visible = static_cast<int>(box_height / line_height);
        scroll_lines = std::max(0, static_cast<int>(history->size()) - max_visible);
    }

    void draw(sf::RenderWindow& window)
    {
        if (!history) return;

        const sf::Vector2f boxPos = box.getPosition();
        const sf::Vector2u winSize = window.getSize();

        const float sx = boxPos.x / static_cast<float>(winSize.x);
        const float sy = boxPos.y / static_cast<float>(winSize.y);
        const float sw = box_width / static_cast<float>(winSize.x);
        const float sh = box_height / static_cast<float>(winSize.y);

        sf::View current = window.getView();
        const sf::FloatRect oldScissor = current.getScissor();

        current.setScissor(sf::FloatRect({ sx, sy }, { sw, sh }));
        window.setView(current);

        const int max_visible = static_cast<int>(box_height / line_height);
        const int total = static_cast<int>(history->size());

        int start = scroll_lines;
        if (start > total) start = total;

        float y = boxPos.y + 4.0f;

        for (int i = start; i < total; ++i)
        {
            if (y > boxPos.y + box_height) break;

            text_drawer.setString((*history)[i]);
            text_drawer.setPosition({ boxPos.x + 6.0f, y });
            window.draw(text_drawer);

            y += line_height;
        }

        current.setScissor(oldScissor);
        window.setView(current);
    }
};

#endif