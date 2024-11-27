#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <iostream>
#include <memory>

class Object 
{
public:
    virtual void draw(sf::RenderWindow &window, sf::Shader &shader, const sf::Vector3f &cameraPosition, const sf::Vector3f &lightPosition1, const sf::Vector3f &lightPosition2, bool enableLight1, bool enableLight2) = 0;
    virtual void rotate(float angleX, float angleY) = 0;
    virtual void setPosition(const sf::Vector3f &position) { this->_position = position; }
    virtual ~Object() = default;

protected:
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

    std::vector<sf::Vector2f> projectVertices(const std::vector<sf::Vector3f> &vertices, const sf::Vector3f &cameraPosition) 
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

class Cube: public Object {
public:
    
    Cube(float size, sf::Vector3f position, int screenWidth, int screenHeight) 
    {
        this->_position = position;
        this->_screenWidth = screenWidth;
        this->_screenHeight = screenHeight;

        _vertices = 
        {
            {-size, -size, -size},
            { size, -size, -size},
            { size,  size, -size},
            {-size,  size, -size},
            {-size, -size,  size},
            { size, -size,  size},
            { size,  size,  size},
            {-size,  size,  size}
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

    void draw(sf::RenderWindow &window, sf::Shader &shader, const sf::Vector3f &cameraPosition, const sf::Vector3f &lightPosition1, const sf::Vector3f &lightPosition2, bool enableLight1, bool enableLight2) override 
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

            sf::Vector3f normal = calculateNormal(rotatedVertices[face[0]], rotatedVertices[face[1]], rotatedVertices[face[2]]);

            shader.setUniform("fragNormal", sf::Glsl::Vec3(normal.x, normal.y, normal.z));
            shader.setUniform("fragPosition", sf::Glsl::Vec3(rotatedVertices[face[0]].x, rotatedVertices[face[0]].y, rotatedVertices[face[0]].z));
            shader.setUniform("lightPosition1", sf::Glsl::Vec3(lightPosition1.x, lightPosition1.y, lightPosition1.z));
            shader.setUniform("lightPosition2", sf::Glsl::Vec3(lightPosition2.x, lightPosition2.y, lightPosition2.z));
            shader.setUniform("cameraPosition", sf::Glsl::Vec3(cameraPosition.x, cameraPosition.y, cameraPosition.z));
            shader.setUniform("enableLight1", enableLight1);
            shader.setUniform("enableLight2", enableLight2);

            window.draw(quad, &shader);
        }
    }

    void rotate(float angleX, float angleY) override 
    {
        this->angleX = angleX;
        this->angleY = angleY;
    }

private:
    float angleX = 0.0f, angleY = 0.0f;
};

class Sphere: 
    public Object 
{

public:
    Sphere(float radius, int segments, sf::Vector3f position, int screenWidth, int screenHeight) 
    {
        this->_position = position;
        this->_screenWidth = screenWidth;
        this->_screenHeight = screenHeight;

        for (int i = 0; i <= segments; ++i) 
        {
            float lat = i * M_PI / segments - M_PI / 2;
            float sinLat = std::sin(lat);
            float cosLat = std::cos(lat);

            for (int j = 0; j <= segments; ++j) 
            {
                float lon = j * 2 * M_PI / segments;
                float sinLon = std::sin(lon);
                float cosLon = std::cos(lon);

                float x = radius * cosLat * cosLon;
                float y = radius * sinLat;
                float z = radius * cosLat * sinLon;

                _vertices.push_back({x, y, z});
            }
        }

        for (int i = 0; i < segments; ++i) 
        {
            for (int j = 0; j < segments; ++j) 
            {
                int i0 = i * (segments + 1) + j;
                int i1 = i0 + 1;
                int i2 = (i + 1) * (segments + 1) + j;
                int i3 = i2 + 1;

                _faces.push_back({i0, i1, i3, i2});
            }
        }
    }

    void draw(sf::RenderWindow &window, sf::Shader &shader, const sf::Vector3f &cameraPosition, const sf::Vector3f &lightPosition1, const sf::Vector3f &lightPosition2, bool enableLight1, bool enableLight2) override 
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

            sf::Vector3f normal = calculateNormal(rotatedVertices[face[0]], rotatedVertices[face[1]], rotatedVertices[face[2]]);

            shader.setUniform("fragNormal", sf::Glsl::Vec3(normal.x, normal.y, normal.z));
            shader.setUniform("fragPosition", sf::Glsl::Vec3(rotatedVertices[face[0]].x, rotatedVertices[face[0]].y, rotatedVertices[face[0]].z));
            shader.setUniform("lightPosition1", sf::Glsl::Vec3(lightPosition1.x, lightPosition1.y, lightPosition1.z));
            shader.setUniform("lightPosition2", sf::Glsl::Vec3(lightPosition2.x, lightPosition2.y, lightPosition2.z));
            shader.setUniform("cameraPosition", sf::Glsl::Vec3(cameraPosition.x, cameraPosition.y, cameraPosition.z));
            shader.setUniform("enableLight1", enableLight1);
            shader.setUniform("enableLight2", enableLight2);

            window.draw(quad, &shader);
        }
    }

    void rotate(float angleX, float angleY) override 
    {
        this->angleX = angleX;
        this->angleY = angleY;
    }

private:
    float angleX = 0.0f, angleY = 0.0f;
};

class Pyramid: 
    public Object 
{

public:
    Pyramid(float size, sf::Vector3f position, int screenWidth, int screenHeight) 
    {
        this->_position = position;
        this->_screenWidth = screenWidth;
        this->_screenHeight = screenHeight;

        _vertices = 
        {
            {-size, -size, -size},
            { size, -size, -size},
            { size, -size,  size},
            {-size, -size,  size},
            {0.0f,  size, 0.0f}
        };

        _faces = 
        {
            {0, 1, 4},
            {1, 2, 4},
            {2, 3, 4},
            {3, 0, 4},
            {0, 1, 2, 3}
        };
    }

    void draw(sf::RenderWindow &window, sf::Shader &shader, const sf::Vector3f &cameraPosition, const sf::Vector3f &lightPosition1, const sf::Vector3f &lightPosition2, bool enableLight1, bool enableLight2) override 
    {
        std::vector<sf::Vector3f> rotatedVertices = rotateVertices(angleX, angleY);
        std::vector<sf::Vector2f> projectedVertices = projectVertices(rotatedVertices, cameraPosition);

        for (const auto &face: _faces) 
        {
            if (face.size() == 3) 
            {
                sf::VertexArray triangle(sf::Triangles, 3);
                for (int i = 0; i < 3; ++i) 
                {
                    triangle[i].position = projectedVertices[face[i]];
                }

                sf::Vector3f normal = calculateNormal(rotatedVertices[face[0]], rotatedVertices[face[1]], rotatedVertices[face[2]]);

                shader.setUniform("fragNormal", sf::Glsl::Vec3(normal.x, normal.y, normal.z));
                shader.setUniform("fragPosition", sf::Glsl::Vec3(rotatedVertices[face[0]].x, rotatedVertices[face[0]].y, rotatedVertices[face[0]].z));
                shader.setUniform("lightPosition1", sf::Glsl::Vec3(lightPosition1.x, lightPosition1.y, lightPosition1.z));
                shader.setUniform("lightPosition2", sf::Glsl::Vec3(lightPosition2.x, lightPosition2.y, lightPosition2.z));
                shader.setUniform("cameraPosition", sf::Glsl::Vec3(cameraPosition.x, cameraPosition.y, cameraPosition.z));
                shader.setUniform("enableLight1", enableLight1);
                shader.setUniform("enableLight2", enableLight2);

                window.draw(triangle, &shader);
            } 
            else if (face.size() == 4) 
            {
                sf::VertexArray quad(sf::Quads, 4);
                for (int i = 0; i < 4; ++i) 
                {
                    quad[i].position = projectedVertices[face[i]];
                }

                sf::Vector3f normal = calculateNormal(rotatedVertices[face[0]], rotatedVertices[face[1]], rotatedVertices[face[2]]);

                shader.setUniform("fragNormal", sf::Glsl::Vec3(normal.x, normal.y, normal.z));
                shader.setUniform("fragPosition", sf::Glsl::Vec3(rotatedVertices[face[0]].x, rotatedVertices[face[0]].y, rotatedVertices[face[0]].z));
                shader.setUniform("lightPosition1", sf::Glsl::Vec3(lightPosition1.x, lightPosition1.y, lightPosition1.z));
                shader.setUniform("lightPosition2", sf::Glsl::Vec3(lightPosition2.x, lightPosition2.y, lightPosition2.z));
                shader.setUniform("cameraPosition", sf::Glsl::Vec3(cameraPosition.x, cameraPosition.y, cameraPosition.z));
                shader.setUniform("enableLight1", enableLight1);
                shader.setUniform("enableLight2", enableLight2);

                window.draw(quad, &shader);
            }
        }
    }

    void rotate(float angleX, float angleY) override 
    {
        this->angleX = angleX;
        this->angleY = angleY;
    }

private:
    float angleX = 0.0f, angleY = 0.0f;
};

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "FOURTH LAB");
    window.setFramerateLimit(60);

    sf::Shader shader;
    if (!shader.loadFromFile("../shaders/shader.frag", sf::Shader::Fragment)) 
    {
        std::cerr << "Failed to load shader" << std::endl;
        return -1;
    }

    sf::Vector3f cameraPosition = {0.0f, 0.0f, -5.0f};
    sf::Vector3f lightPosition1 = {2.0f, 2.0f, -2.0f};
    sf::Vector3f lightPosition2 = {-2.0f, -2.0f, -2.0f};
    float cameraSpeed = 1.0f;

    bool enableLight1 = true;
    bool enableLight2 = true;

    std::unique_ptr<Object> cube = std::make_unique<Cube>(1.0f, sf::Vector3f{0.0f, 0.0f, 0.0f}, window.getSize().x, window.getSize().y);
    std::unique_ptr<Object> sphere = std::make_unique<Sphere>(1.0f, 20, sf::Vector3f{3.0f, 0.0f, 0.0f}, window.getSize().x, window.getSize().y);
    std::unique_ptr<Object> pyramid = std::make_unique<Pyramid>(1.0f, sf::Vector3f{-3.0f, 0.0f, 0.0f}, window.getSize().x, window.getSize().y);

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

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) 
        {
            cameraPosition.z += cameraSpeed;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) 
        {
            cameraPosition.z -= cameraSpeed;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) 
        {
            cameraPosition.x -= cameraSpeed;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) 
        {
            cameraPosition.x += cameraSpeed;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) 
        {
            enableLight1 = !enableLight1;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) 
        {
            enableLight2 = !enableLight2;
        }

        window.clear();

        cube->draw(window, shader, cameraPosition, lightPosition1, lightPosition2, enableLight1, enableLight2);
        sphere->draw(window, shader, cameraPosition, lightPosition1, lightPosition2, enableLight1, enableLight2);
        pyramid->draw(window, shader, cameraPosition, lightPosition1, lightPosition2, enableLight1, enableLight2);

        window.display();
    }

    return 0;
}