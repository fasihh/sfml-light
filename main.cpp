#include <SFML/Graphics.hpp>
#include <cmath>
#include "line.hpp"
#include "intersection.hpp"

sf::RenderWindow window(sf::VideoMode(800, 600), "SFML", sf::Style::Default, sf::ContextSettings(0, 0, 8));

// find intersection of two paramteric equations: r = A + T(B-A) where B-A is the direction vector
Intersection getIntersection(Line ray, Line segment){
	float r_px = ray.a.x; // ray x
	float r_py = ray.a.y; // ray y
	float r_dx = ray.b.x-ray.a.x; // direction of ray x
	float r_dy = ray.b.y-ray.a.y; // direction of ray y

	float s_px = segment.a.x; // segment x
	float s_py = segment.a.y; // segment y
	float s_dx = segment.b.x-segment.a.x; // direction of segment x
	float s_dy = segment.b.y-segment.a.y; // direction of segment y

    // finding unit vector of both ray and segment to check if they are parallel
	float r_mag = std::sqrt(r_dx*r_dx+r_dy*r_dy);
	float s_mag = std::sqrt(s_dx*s_dx+s_dy*s_dy);
    // in case they are, return a false status
	if(r_dx/r_mag==s_dx/s_mag && r_dy/r_mag==s_dy/s_mag)
		return { false };

    // SOLVE FOR T1 & T2
	// r_px+r_dx*T1 = s_px+s_dx*T2 && r_py+r_dy*T1 = s_py+s_dy*T2
	// ==> T1 = (s_px+s_dx*T2-r_px)/r_dx = (s_py+s_dy*T2-r_py)/r_dy
	// ==> s_px*r_dy + s_dx*T2*r_dy - r_px*r_dy = s_py*r_dx + s_dy*T2*r_dx - r_py*r_dx
	// ==> T2 = (r_dx*(s_py-r_py) + r_dy*(r_px-s_px))/(s_dx*r_dy - s_dy*r_dx)
	float T2 = (r_dx*(s_py-r_py) + r_dy*(r_px-s_px))/(s_dx*r_dy - s_dy*r_dx);
	float T1 = (s_px+s_dx*T2-r_px)/r_dx;

    // conditions for intersection
	if(T1<0) return { false };
	if(T2<0 || T2>1) return { false };

    // if all conditions satisfy, return an intersection with true status, point and the value T
	return {
        true,
		r_px+r_dx*T1,
		r_py+r_dy*T1,
		T1
	};
}

// utility function to find all unique points
void findUniquePoints(const std::vector<Line> segments, std::vector<Point>& uniquePoints) {
    for (Line segment : segments) {
        Point startPoint = segment.a;
        Point endPoint = segment.b;

        bool startPointUnique = true;
        for (Point point : uniquePoints) {
            if (point == startPoint) {
                startPointUnique = false;
                break;
            }
        }
        if (startPointUnique)
            uniquePoints.push_back(startPoint);

        bool endPointUnique = true;
        for (Point point : uniquePoints) {
            if (point == endPoint) {
                endPointUnique = false;
                break;
            }
        }
        if (endPointUnique)
            uniquePoints.push_back(endPoint);
    }
}

int main() {
    window.setFramerateLimit(60);
    // window size in vector2f
    sf::Vector2f windowSize = static_cast<sf::Vector2f>(window.getSize());

    // loadings shaders
    sf::Shader shadowShader;
    shadowShader.loadFromFile("shadow.vert", "shadow.frag");

    // defining the u_resolution variable in the shadow.frag file
    shadowShader.setUniform("u_resolution", windowSize);

    // creating a new texture
    sf::RenderTexture castTexture;
    castTexture.create(window.getSize().x, window.getSize().y);
    castTexture.setSmooth(true);
    
    // creating objects
    std::vector<Line> segments{
        // borders
        { {0.f, 0.f}, {windowSize.x, 0.f} },
        { {windowSize.x, 0.f}, {windowSize.x, windowSize.y} },
        { {windowSize.x, windowSize.y}, {0.f, windowSize.y} },
        { {0.f, windowSize.y}, {0.f, 0.f} },

        // square
        { {100.f, 200.f}, {100.f, 300.f} },
        { {100.f, 300.f}, {200.f, 300.f} },
        { {200.f, 300.f}, {200.f, 200.f} },
        { {200.f, 200.f}, {100.f, 200.f} },

        // big square
        { {300.f, 500.f}, {500.f, 500.f} },
        { {500.f, 500.f}, {500.f, 300.f} },
        { {500.f, 300.f}, {300.f, 300.f} },
        { {300.f, 300.f}, {300.f, 500.f} },

        // triangle
        { {400.f, 100.f}, {500.f, 200.f} },
        { {500.f, 200.f}, {300.f, 300.f} },
        { {300.f, 300.f}, {400.f, 100.f} },

        // pentagon
        { {100.f, 400.f}, {150.f, 450.f} },
        { {150.f, 450.f}, {200.f, 425.f} },
        { {200.f, 425.f}, {175.f, 375.f} },
        { {175.f, 375.f}, {125.f, 375.f} },
        { {125.f, 375.f}, {100.f, 400.f} }
    };

    // finding all unique points from those objects
    std::vector<Point> uniquePoints;
    findUniquePoints(segments, uniquePoints);
    
    while (window.isOpen()) {
        sf::Event event;
        std::vector<float> angles;
        std::vector<Intersection> intersections;
        // cursor position
        sf::Vector2f mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // setting the u_mouse variable with mousePosition
        shadowShader.setUniform("u_mouse", mousePos);

        // finding the angle of each point with slight change
        // all the rays will only hit the vertices of objects
        // this creates very bad polygons and to fix it, another ray at a slightly changed angle
        // is projected so that it goes past that vertex
        for (Point point : uniquePoints) {
            float angle = std::atan2(point.y-mousePos.y,point.x-mousePos.x);
            angles.push_back(angle-0.0001f);
            angles.push_back(angle);
            angles.push_back(angle+0.0001f);
        }

        // all angles are used to find the direction of the ray
        for (float angle : angles) {
            float dx = std::cos(angle);
            float dy = std::sin(angle);

            // starting point is the mouse, ending point is the mouse+direction
            Line ray{ { mousePos.x, mousePos.y }, { mousePos.x+dx, mousePos.y+dy } };

            // finding the closest intersection point
            Intersection closest{ false };
            for (Line segment : segments) {
                Intersection intersect = getIntersection(ray, segment);
                intersect.angle = angle; // store angle too for later use
                if (!intersect.status) continue; // if doesn't intersect then pass
                // if no previous intersection OR current intersection closer to previous closest
                if (!closest.status || intersect.T1 < closest.T1) 
                    closest = intersect;
            }
            // store each intersection
            intersections.push_back(closest);
        }
        // sort intersections based on angles
        // this is done to create a proper triangle fan object
        std::sort(intersections.begin(), intersections.end(), [&](const Intersection& a, const Intersection& b) { return a.angle < b.angle; });
        intersections.emplace_back(intersections[0]); // this completes the triangle fan from all directions

        window.clear();
        castTexture.clear();

        // storing vertices for the triangle fan
        std::vector<sf::Vertex> vertices;
        // initial point for the triangle fan
        vertices.emplace_back(sf::Vertex(mousePos));

        // storing every intersection point in the vertices
        for (Intersection intersect : intersections)
            vertices.emplace_back(intersect.point.vertex());

        // drawing texture as a triangle fan using the vertices
        castTexture.draw(vertices.data(), vertices.size(), sf::TriangleFan);
        castTexture.display();

        // drawing the texture
        window.draw(sf::Sprite(castTexture.getTexture()), &shadowShader);

        // drawing game objects
        for (Line segment : segments)
            segment.draw(window);

        window.display();
    }

    return 0;
}
