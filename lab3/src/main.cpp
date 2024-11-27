#include <array>
#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>

class Cube 
{
public:
    Cube(float size, sf::Vector3f position, int screenWidth, int screenHeight): 
        _size(size), _position(position), _screenWidth(screenWidth), _screenHeight(screenHeight) 
    {
        _vertices = 
        {
            {-_size, -_size, -_size},
            { _size, -_size, -_size},
            { _size,  _size, -_size},
            {-_size,  _size, -_size},
            {-_size, -_size,  _size},
            { _size, -_size,  _size},
            { _size,  _size,  _size},
            {-_size,  _size,  _size}
        };

        _faces = 
        {
            {0, 1, 2, 3},
            {1, 5, 6, 2},
            {5, 4, 7, 6},
            {4, 0, 3, 7},
            {0, 1, 5, 4},
            {3, 2, 6, 7}
        };
    }

    void draw(sf::RenderWindow &window, float angleX, float angleY, sf::Vector3f cameraPosition) 
    {
        std::vector<sf::Vector3f> rotatedVertices = rotateVertices(angleX, angleY);
        std::vector<sf::Vector2f> projectedVertices = projectVertices(rotatedVertices, cameraPosition);

        for (const auto &face: _faces) 
        {
            sf::VertexArray quad(sf::Quads, 4);
            for (int i = 0; i < 4; ++i) 
            {
                quad[i].position = projectedVertices[face[i]];
            }

            window.draw(quad);
        }
    }

private:
    float _size;
    sf::Vector3f _position;
    int _screenWidth, _screenHeight;
    std::vector<sf::Vector3f> _vertices;
    std::vector<std::vector<int>> _faces;

    std::vector<sf::Vector3f> rotateVertices(float angleX, float angleY) 
    {
        std::vector<sf::Vector3f> rotatedVertices;
        float cosX = std::cos(angleX);
        float sinX = std::sin(angleX);
        float cosY = std::cos(angleY);
        float sinY = std::sin(angleY);

        for (const auto &vertex: _vertices) 
        {
            float x = vertex.x;
            float y = vertex.y;
            float z = vertex.z;

            float y1 = y * cosX - z * sinX;
            float z1 = y * sinX + z * cosX;

            float x1 = x * cosY + z1 * sinY;
            float z2 = -x * sinY + z1 * cosY;

            rotatedVertices.push_back({x1, y1, z2});
        }

        return rotatedVertices;
    }

    std::vector<sf::Vector2f> projectVertices(const std::vector<sf::Vector3f> &vertices, sf::Vector3f cameraPosition) 
    {
        std::vector<sf::Vector2f> projectedVertices;
        float fov = 256.0f;
        float aspectRatio = static_cast<float>(_screenWidth) / _screenHeight;

        for (const auto &vertex: vertices) 
        {
            float x = vertex.x + _position.x - cameraPosition.x;
            float y = vertex.y + _position.y - cameraPosition.y;
            float z = vertex.z + _position.z - cameraPosition.z;

            float zInv = 1.0f / (z + fov);
            float xProj = x * zInv * fov * aspectRatio + _screenWidth / 2;
            float yProj = y * zInv * fov + _screenHeight / 2;

            projectedVertices.push_back({xProj, yProj});
        }

        return projectedVertices;
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "THIRD_LAB");

    std::vector<Cube> cubes = 
    {
        Cube(20.0f, {0.0f, 0.0f, 150.0f}, window.getSize().x, window.getSize().y),
        Cube(20.0f, {60.0f, 0.0f, 100.0f}, window.getSize().x, window.getSize().y),
        Cube(20.0f, {-40.0f, 0.0f, 80.0f}, window.getSize().x, window.getSize().y)
    };

    float angleX = 0.0f;
    float angleY = 0.0f;

    sf::Vector3f cameraStart = {0.0f, 0.0f, 0.0f};
    std::array<sf::Vector3f, 3> cameraEnd = 
    {
        sf::Vector3f{-20.0f, 0.0f, 300.0f},
        sf::Vector3f{30.0f, 0.0f, 300.0f},
        sf::Vector3f{80.0f, 0.0f, 300.0f}
    };

    float cameraSpeed = 0.001f;
    float cameraPosition = 0.0f;

    std::size_t direction = 1;

    while (window.isOpen()) 
    {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) 
            {
                cameraSpeed += 0.001f;
                if (cameraSpeed > 0.01f) 
                {
                    cameraSpeed = 0.01f;
                }
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) 
            {
                cameraSpeed -= 0.001f;
                if (cameraSpeed < 0.0f) 
                {
                    cameraSpeed = 0.0f;
                }
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) 
            {
                direction = 0;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) 
            {
                direction = 1;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3)) 
            {
                direction = 2;
            }
        }

        window.clear();

        cameraPosition += cameraSpeed;
        if (cameraPosition > 1.0f) 
        {
            cameraSpeed = -cameraSpeed;
        } else if (cameraPosition < 0.0f) 
        {
            cameraSpeed = -cameraSpeed;
        }

        sf::Vector3f cameraPos = 
        {
            cameraStart.x + (cameraEnd[direction].x - cameraStart.x) * cameraPosition,
            cameraStart.y + (cameraEnd[direction].y - cameraStart.y) * cameraPosition,
            cameraStart.z + (cameraEnd[direction].z - cameraStart.z) * cameraPosition
        };

        for (auto &cube: cubes) 
        {
            cube.draw(window, angleX, angleY, cameraPos);
        }

        window.display();
    }

    return 0;
}