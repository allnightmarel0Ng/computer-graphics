#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <iostream>
#include <cmath>
#include <vector>
#include <limits>
#include <cstdlib>

constexpr int WIDTH = 800;
constexpr int HEIGHT = 600;

struct Vec3 
{
    float x, y, z;

    Vec3(float x = 0, float y = 0, float z = 0): 
        x(x), y(y), z(z) 
    {

    }

    Vec3 operator+(const Vec3 &v) const 
    { 
        return Vec3(x + v.x, y + v.y, z + v.z); 
    }

    Vec3 operator-(const Vec3 &v) const 
    { 
        return Vec3(x - v.x, y - v.y, z - v.z); 
    }

    Vec3 operator*(float f) const 
    { 
        return Vec3(x * f, y * f, z * f); 
    }

    Vec3 operator/(float f) const 
    { 
        return Vec3(x / f, y / f, z / f); 
    }

    float dot(const Vec3 &v) const 
    { 
        return x * v.x + y * v.y + z * v.z; 
    }

    Vec3 cross(const Vec3 &v) const 
    { 
        return Vec3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); 
    }

    Vec3 normalize() const 
    { 
        return *this / sqrt(x * x + y * y + z * z); 
    }
};

struct Sphere 
{
    Vec3 center;
    float radius;
    Vec3 color;

    Sphere(const Vec3 &c, float r, const Vec3 &col): 
        center(c), radius(r), color(col) 
    {

    }
};

struct Light 
{
    Vec3 position;
    Vec3 color;

    Light(const Vec3 &p, const Vec3 &c): 
        position(p), color(c) 
    {

    }
};

struct Camera 
{
    Vec3 position;
    Vec3 direction;
    float aperture;
    float focalLength;
    int samples;

    Camera(const Vec3 &pos, const Vec3 &dir, float ap, float fl, int s): 
        position(pos), direction(dir), aperture(ap), focalLength(fl), samples(s) 
    {

    }
};

Vec3 randomInUnitDisk(float radius) 
{
    float theta = 2 * M_PI * (rand() / (float)RAND_MAX);
    float r = radius * sqrt(rand() / (float)RAND_MAX);
    return Vec3(r * cos(theta), r * sin(theta), 0);
}

Vec3 getRayDirection(const Camera &camera, float u, float v) 
{
    Vec3 rayDirection = Vec3(u, v, -1).normalize();
    Vec3 focalPoint = camera.position + rayDirection * camera.focalLength;

    Vec3 offset = randomInUnitDisk(camera.aperture);
    Vec3 newOrigin = camera.position + offset;

    return (focalPoint - newOrigin).normalize();
}

bool intersectSphere(const Vec3 &rayOrigin, const Vec3 &rayDirection, const Sphere &sphere, float &t) 
{
    Vec3 oc = rayOrigin - sphere.center;
    float a = rayDirection.dot(rayDirection);
    float b = 2.0f * oc.dot(rayDirection);
    float c = oc.dot(oc) - sphere.radius * sphere.radius;
    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0) return false;
    t = (-b - sqrt(discriminant)) / (2.0f * a);
    return t >= 0;
}

Vec3 traceRay(const Vec3 &rayOrigin, const Vec3 &rayDirection, const std::vector<Sphere> &spheres, const std::vector<Light> &lights) 
{
    float closestT = std::numeric_limits<float>::max();
    const Sphere *closestSphere = nullptr;

    for (const auto &sphere : spheres) 
    {
        float t;
        if (intersectSphere(rayOrigin, rayDirection, sphere, t) & t < closestT) 
        {
            closestT = t;
            closestSphere = &sphere;
        }
    }

    if (closestSphere) 
    {
        Vec3 hitPoint = rayOrigin + rayDirection * closestT;
        Vec3 normal = (hitPoint - closestSphere->center).normalize();
        Vec3 color = closestSphere->color;

        Vec3 finalColor(0, 0, 0);
        for (const auto &light : lights) 
        {
            Vec3 lightDir = (light.position - hitPoint).normalize();
            float diffuse = std::max(normal.dot(lightDir), 0.0f);
            finalColor = finalColor + color * diffuse;
        }

        return finalColor;
    }

    return Vec3(0, 0, 0);
}

Vec3 traceRayWithDoF(const Vec3 &rayOrigin, const Vec3 &rayDirection, const std::vector<Sphere> &spheres, const std::vector<Light> &lights, const Camera &camera) 
{
    Vec3 color(0, 0, 0);

    for (int i = 0; i < camera.samples; ++i) 
    {
        Vec3 sampleDirection = getRayDirection(camera, rayDirection.x, rayDirection.y);
        color = color + traceRay(rayOrigin, sampleDirection, spheres, lights);
    }

    return color / camera.samples;
}

void renderScene(sf::Image &image, const std::vector<Sphere> &spheres, const std::vector<Light> &lights, const Camera &camera) 
{
    for (int y = 0; y < HEIGHT; ++y) 
    {
        for (int x = 0; x < WIDTH; ++x) 
        {
            float u = (x + 0.5f) / WIDTH;
            float v = (y + 0.5f) / HEIGHT;
            Vec3 rayDirection = Vec3(u - 0.5f, v - 0.5f, -1).normalize();

            Vec3 color = traceRayWithDoF(camera.position, rayDirection, spheres, lights, camera);

            image.setPixel(x, y, sf::Color(color.x * 255, color.y * 255, color.z * 255));
        }
    }
}

int main() 
{
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Ray Tracing with DoF", sf::Style::Default, sf::ContextSettings(24));
    window.setVerticalSyncEnabled(true);

    sf::Image image;
    image.create(WIDTH, HEIGHT, sf::Color::Black);

    std::vector<Sphere> spheres = 
    {
        Sphere(Vec3(-1, 0, -5), 1, Vec3(1, 0, 0)),
        Sphere(Vec3(1, 0, -5), 1, Vec3(0, 1, 0)),
        Sphere(Vec3(0, -1, -5), 1, Vec3(0, 0, 1))
    };

    std::vector<Light> lights = 
    {
        Light(Vec3(0, 5, 0), Vec3(1, 1, 1))
    };

    Camera camera(Vec3(0, 0, 0), Vec3(0, 0, -1), 0.1f, 5.0f, 10);

    while (window.isOpen()) 
    {
        sf::Event event;
        while (window.pollEvent(event)) 
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }

            if (event.type == sf::Event::KeyPressed) 
            {
                if (event.key.code == sf::Keyboard::Q) 
                {
                    camera.aperture -= 0.01f;
                    camera.aperture = std::max(camera.aperture, 0.01f);
                }
                if (event.key.code == sf::Keyboard::E) 
                {
                    camera.aperture += 0.01f;
                }
                if (event.key.code == sf::Keyboard::R) 
                {
                    camera.focalLength -= 0.5f;
                    camera.focalLength = std::max(camera.focalLength, 1.0f);
                }
                if (event.key.code == sf::Keyboard::F) 
                {
                    camera.focalLength += 0.5f;
                }
            }
        }

        renderScene(image, spheres, lights, camera);

        sf::Texture texture;
        texture.loadFromImage(image);
        sf::Sprite sprite(texture);
        window.draw(sprite);
        window.display();
    }

    return 0;
}