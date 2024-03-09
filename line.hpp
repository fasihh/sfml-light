#pragma once
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <iostream>

struct Point {
    float x, y;

    Point(float x = 0.f, float y = 0.f) : x(x), y(y) {}
    sf::Vector2f vector() { return sf::Vector2f{ x, y }; }
    sf::Vertex vertex(sf::Color color = sf::Color::White) { return sf::Vertex{ this->vector(), color }; }

    // test
    void display() {
        std::cout << "{ " << x << ", " << y << " }" << std::endl;
    }

    bool operator == (const Point& obj) const {
        return this->x == obj.x && this->y == obj.y;
    }
};

struct Line {
    Point a, b;

    Line(Point a, Point b) : a(a), b(b) {}

    sf::VertexArray vertArr(sf::Color color = sf::Color::White) {
        sf::VertexArray arr(sf::Lines, 2);
        arr[0] = a.vertex(color), arr[1] = b.vertex(color);
        return arr;
    }

    void draw(sf::RenderWindow& window, sf::Color color = sf::Color::White) {
        window.draw(this->vertArr(color));
    }
};