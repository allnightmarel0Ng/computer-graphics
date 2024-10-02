#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <cmath>
#include <array>

#define CONTROL_POINTS 4
#define CURVE_POINTS 1000

using point = std::pair<float, float>;

template<typename std::size_t Size>
using point_array = std::array<point, Size>;

point evaluateBezier(float t, const point_array<CONTROL_POINTS> &controlPoints) noexcept
{
    auto x = 0.0f, y = 0.0f;
    auto n = controlPoints.size() - 1;
    for (int i = 0; i <= n; ++i)
    {
        float blend = std::pow(1 - t, n - i) * std::pow(t, i) *
                      (std::tgamma(n + 1) / (std::tgamma(i + 1) * std::tgamma(n - i + 1)));
        x += controlPoints[i].first * blend;
        y += controlPoints[i].second * blend;
    }
    return std::make_pair(x, y);
}

point_array<CURVE_POINTS> computeBezierCurvePoints(const point_array<CONTROL_POINTS> &controlPoints) noexcept
{
    point_array<CURVE_POINTS> result;

    float t = 0.0f;
    for (auto &elem: result)
    {
        elem = evaluateBezier(t, controlPoints);
        t += 0.001f;
    }

    return result;
}

void drawBezierCurve(const point_array<CURVE_POINTS> &bezierCurvePoints) noexcept
{
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_STRIP);
    for (const auto &point: bezierCurvePoints)
    {
        glVertex2f(point.first, point.second);
    }
    glEnd();
}

void render(std::size_t index, const point_array<CURVE_POINTS> &bezierCurvePoints) noexcept
{
    glClear(GL_COLOR_BUFFER_BIT);

    drawBezierCurve(bezierCurvePoints);

    glColor3f(1.0f, 0.0f, 0.0f);
    auto circlePoint = bezierCurvePoints[index];
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(circlePoint.first, circlePoint.second);
    for (int i = 0; i <= 360; ++i)
    {
        float angle = i * M_PI / 180.0f;
        float size = 0.05f + (static_cast<float>(index) / 20000);
        glVertex2f(circlePoint.first + size * std::cos(angle), circlePoint.second + size * std::sin(angle));
    }
    glEnd();
}

int main()
{
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 4;
    settings.majorVersion = 3;
    settings.minorVersion = 3;

    constexpr point_array<CONTROL_POINTS> controlPoints = {
        std::make_pair(-0.8f, -0.5f),
        std::make_pair(-0.2f, 0.8f),
        std::make_pair(0.2f, -0.8f),
        std::make_pair(0.8f, 0.5f)
    };

    sf::RenderWindow window(sf::VideoMode(800, 600), "FIRST LAB", sf::Style::Default, settings);

    sf::RectangleShape plusButton(sf::Vector2f(50.f, 50.f));
    plusButton.setPosition(650, 50);
    plusButton.setFillColor(sf::Color::Green);

    sf::RectangleShape minusButton(sf::Vector2f(50.f, 50.f));
    minusButton.setPosition(650, 150);
    minusButton.setFillColor(sf::Color::Red);

    auto plusBounds = plusButton.getGlobalBounds();
    auto minusBounds = minusButton.getGlobalBounds();

    auto bezierCurvePoints = computeBezierCurvePoints(controlPoints);
    std::size_t index = 0;
    std::size_t sign = 1;
    std::size_t coefficient = 10;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            {
                auto mousePos = sf::Mouse::getPosition(window);
                if (plusBounds.contains(mousePos.x, mousePos.y) && coefficient < 7)
                {
                    coefficient++;
                }
                else if (minusBounds.contains(mousePos.x, mousePos.y) && coefficient > 0)
                {
                    coefficient--;
                }
            }
        }

        window.clear();
        render(index, bezierCurvePoints);

        window.pushGLStates();
        window.draw(plusButton);
        window.draw(minusButton);
        window.popGLStates();

        window.display();

        index += coefficient * sign;
        if (index > CURVE_POINTS - 1 || index < 1) sign *= -1;
    }

    return 0;
}