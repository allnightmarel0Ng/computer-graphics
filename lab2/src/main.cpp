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

        faces = 
        {
            {0, 1, 2, 3},
            {1, 5, 6, 2},
            {5, 4, 7, 6},
            {4, 0, 3, 7},
            {0, 1, 5, 4},
            {3, 2, 6, 7}
        };
    }

    void draw(sf::RenderWindow &window, float angleX, float angleY, sf::Vector3f lightDirection1, sf::Vector3f lightDirection2) 
    {
        std::vector<sf::Vector3f> rotatedVertices = rotateVertices(angleX, angleY);
        std::vector<sf::Vector2f> projectedVertices = projectVertices(rotatedVertices);

        sf::Vector3f cameraDirection = {0.0f, 0.0f, -1.0f};

        for (const auto &face: faces) 
        {
            sf::Vector3f normal = calculateNormal(rotatedVertices[face[0]], rotatedVertices[face[1]], rotatedVertices[face[2]]);

            if (dotProduct(normal, cameraDirection) > 0) 
            {
                continue;
            }

            sf::VertexArray quad(sf::Quads, 4);
            for (int i = 0; i < 4; ++i) 
            {
                quad[i].position = projectedVertices[face[i]];
            }

            float intensity1 = dotProduct(normal, lightDirection1);
            intensity1 = std::max(0.2f, intensity1);

            float intensity2 = dotProduct(normal, lightDirection2);
            intensity2 = std::max(0.2f, intensity2);

            float totalIntensity = std::min(1.0f, intensity1 + intensity2);

            sf::Color color(255 * totalIntensity, 255 * totalIntensity, 255 * totalIntensity);
            for (int i = 0; i < 4; ++i) 
            {
                quad[i].color = color;
            }

            window.draw(quad);
        }
    }

private:
    float _size;
    sf::Vector3f _position;
    int _screenWidth, _screenHeight;
    std::vector<sf::Vector3f> _vertices;
    std::vector<std::vector<int>> faces;

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

    std::vector<sf::Vector2f> projectVertices(const std::vector<sf::Vector3f> &vertices) 
    {
        std::vector<sf::Vector2f> projectedVertices;
        float fov = 256.0f;
        float aspectRatio = static_cast<float>(_screenWidth) / _screenHeight;

        for (const auto &vertex: vertices) 
        {
            float x = vertex.x;
            float y = vertex.y;
            float z = vertex.z;

            float zInv = 1.0f / (z + _position.z + fov);
            float xProj = (x + _position.x) * zInv * fov * aspectRatio + _screenWidth / 2;
            float yProj = (y + _position.y) * zInv * fov + _screenHeight / 2;

            projectedVertices.push_back({xProj, yProj});
        }

        return projectedVertices;
    }

    sf::Vector3f calculateNormal(const sf::Vector3f &v1, const sf::Vector3f &v2, const sf::Vector3f &v3) 
    {
        sf::Vector3f edge1 = {v2.x - v1.x, v2.y - v1.y, v2.z - v1.z};
        sf::Vector3f edge2 = {v3.x - v1.x, v3.y - v1.y, v3.z - v1.z};

        sf::Vector3f normal = 
        {
            edge1.y * edge2.z - edge1.z * edge2.y,
            edge1.z * edge2.x - edge1.x * edge2.z,
            edge1.x * edge2.y - edge1.y * edge2.x
        };

        float length = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
        normal.x /= length;
        normal.y /= length;
        normal.z /= length;

        return normal;
    }

    float dotProduct(const sf::Vector3f &v1, const sf::Vector3f &v2) 
    {
        return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
    }
};

int main() 
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "SECOND LAB");
    Cube cube(100.0f, {0.0f, 0.0f, 5.0f}, window.getSize().x, window.getSize().y);

    float angleX = 0.3f;
    float angleY = 0.5f;
    sf::Vector3f lightDirection1 = {0.0f, -1.0f, 0.0f};
    sf::Vector3f lightDirection2 = {-1.0f, 1.0f, 1.0f};

    while (window.isOpen()) 
    {
        sf::Event event;
        while (window.pollEvent(event)) 
        {
            if (event.type == sf::Event::Closed) 
            {
                window.close();
            }
        }

        window.clear();
        cube.draw(window, angleX, angleY, lightDirection1, lightDirection2);
        window.display();
    }

    return 0;
}