#include <cmath>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

// Sets up the 3D perspective
void resetPerspective(unsigned windowWidth, unsigned windowHeight);

// Converts from degrees to radians
// Radians are typically used for calculations
float degToRad(float deg);

// Converts from radians to degrees
// Degrees are easier to read and also used for setting angles in OpenGL
float radToDeg(float rad);

int main()
{
    // Create the window
    // 24 is a standard depth buffer bit depth (accuracy)
    // The depth buffer is required for not drawing things that are behind other
    // things
    sf::RenderWindow window({1280, 720}, "Blocky Basement", sf::Style::Default, sf::ContextSettings{24, 0, 0});
    window.setFramerateLimit(60);

    // Player variables
    sf::Vector3f cameraPosition = {0.f, 0.5f, 0.f};
    float cameraRotation = 0.f;

    bool movingForw = false;
    bool movingBack = false;
    bool movingLeft = false;
    bool movingRight = false;

    // Load the level
    sf::Image image;
    image.loadFromFile("level.png");
    for (unsigned i = 0; i < image.getSize().y; i++)
    {
        for (unsigned j = 0; j < image.getSize().x; j++)
        {
            // Red pixel designates the player's spawn point
            if (image.getPixel(j, i) == sf::Color::Red)
            {
                cameraPosition.x = j;
                cameraPosition.z = i;
            }
        }
    }

    // Enables the use of the depth buffer to prevent drawing of things that are
    // behind other things
    glEnable(GL_DEPTH_TEST);

    // Disables drawing of internal faces, as they're always invisible unless
    // you're clipping into something
    glEnable(GL_CULL_FACE);

    // Set the window area in which OpenGL will draw
    glViewport(0, 0, window.getSize().x, window.getSize().y);

    // Set up the 3D perspective
    resetPerspective(window.getSize().x, window.getSize().y);

    // Main loop woo!
    while (window.isOpen())
    {
        // Handle input and other events
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::W)
                    movingForw = true;
                else if (event.key.code == sf::Keyboard::S)
                    movingBack = true;
                else if (event.key.code == sf::Keyboard::A)
                    movingLeft = true;
                else if (event.key.code == sf::Keyboard::D)
                    movingRight = true;
            }
            else if (event.type == sf::Event::KeyReleased)
            {
                if (event.key.code == sf::Keyboard::W)
                    movingForw = false;
                else if (event.key.code == sf::Keyboard::S)
                    movingBack = false;
                else if (event.key.code == sf::Keyboard::A)
                    movingLeft = false;
                else if (event.key.code == sf::Keyboard::D)
                    movingRight = false;
            }
            else if (event.type == sf::Event::LostFocus)
            {
                movingForw = false;
                movingBack = false;
                movingLeft = false;
                movingRight = false;
            }
            else if (event.type == sf::Event::Resized)
            {
                glViewport(0, 0, window.getSize().x, window.getSize().y);
                resetPerspective(window.getSize().x, window.getSize().y);
            }
        }

        // Player movement and rotation
        static constexpr float MovementSpeed = 0.03f;
        static constexpr float TurnSpeed = 0.03f;
        if (movingForw)
        {
            cameraPosition.x += std::sin(cameraRotation) * MovementSpeed;
            cameraPosition.z -= std::cos(cameraRotation) * MovementSpeed;
        }
        if (movingBack)
        {
            cameraPosition.x -= std::sin(cameraRotation) * MovementSpeed;
            cameraPosition.z += std::cos(cameraRotation) * MovementSpeed;
        }
        if (movingLeft)
            cameraRotation -= TurnSpeed;
        if (movingRight)
            cameraRotation += TurnSpeed;

        // Begin drawing
        // To draw the model view matrix must be used
        glMatrixMode(GL_MODELVIEW);

        // Reset the matrix, otherwise any alterations from the last frame will
        // remain and cause issues
        glLoadIdentity();

        // Clear the screen and the depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Offset The World by the camera's rotation and position
        glRotatef(radToDeg(cameraRotation), 0.f, 1.f, 0.f);
        glTranslatef(-cameraPosition.x, -cameraPosition.y, -cameraPosition.z);

        // Draw every wall
        for (unsigned i = 0; i < image.getSize().y; i++)
        {
            for (unsigned j = 0; j < image.getSize().x; j++)
            {
                // Walls are designated by white pixels
                if (image.getPixel(j, i) != sf::Color::White)
                    continue;

                // glPushMatrix(); and glPopMatrix(); allow to place every wall
                // to it's rightful place separately - without affecting the
                // subsequent walls
                glPushMatrix();
                    // Set the wall position
                    glTranslatef(j, 0.f, i);

                    // Set the color and positions of every corner of the wall
                    glBegin(GL_QUADS);
                        // Front wall
                        glColor3f(1.f, 0.f, 0.f); glVertex3f(0.f, 0.f, 1.f);
                        glColor3f(1.f, 0.f, 0.f); glVertex3f(1.f, 0.f, 1.f);
                        glColor3f(1.f, 0.f, 0.f); glVertex3f(1.f, 1.f, 1.f);
                        glColor3f(1.f, 0.f, 0.f); glVertex3f(0.f, 1.f, 1.f);

                        // Back wall
                        glColor3f(0.f, 1.f, 0.f); glVertex3f(0.f, 0.f, 0.f);
                        glColor3f(0.f, 1.f, 0.f); glVertex3f(0.f, 1.f, 0.f);
                        glColor3f(0.f, 1.f, 0.f); glVertex3f(1.f, 1.f, 0.f);
                        glColor3f(0.f, 1.f, 0.f); glVertex3f(1.f, 0.f, 0.f);

                        // Left wall
                        glColor3f(0.f, 0.f, 1.f); glVertex3f(1.f, 0.f, 0.f);
                        glColor3f(0.f, 0.f, 1.f); glVertex3f(1.f, 1.f, 0.f);
                        glColor3f(0.f, 0.f, 1.f); glVertex3f(1.f, 1.f, 1.f);
                        glColor3f(0.f, 0.f, 1.f); glVertex3f(1.f, 0.f, 1.f);

                        // Right wall
                        glColor3f(1.f, 1.f, 0.f); glVertex3f(0.f, 0.f, 0.f);
                        glColor3f(1.f, 1.f, 0.f); glVertex3f(0.f, 0.f, 1.f);
                        glColor3f(1.f, 1.f, 0.f); glVertex3f(0.f, 1.f, 1.f);
                        glColor3f(1.f, 1.f, 0.f); glVertex3f(0.f, 1.f, 0.f);
                    glEnd();
                glPopMatrix();
            }
        }

        // Make the GPU actually render everything
        window.display();
    }

    return 0;
}

void resetPerspective(unsigned windowWidth, unsigned windowHeight)
{
    // How close and far from the camera to draw
    //
    // zFar should be as near as possible for performance reasons as well as
    // accuracy of depth calculations, but having it too close will cut off
    // distant things
    //
    // zNear can't be 0 because math, but having it too large will cut off
    // things right in front of the camera
    static constexpr float zFar = 100.f;
    static constexpr float zNear = 0.1f;

    // Camera field of view
    // Can be any value between 0 and 180 degrees
    const float fov = degToRad(90.f);

    // Magic lol
    const float ratio = (float)windowWidth / (float)windowHeight;

    const float f = 1.f / std::tan(fov / 2.f);
    const float a = f / ratio;
    const float c = (zFar + zNear) / (zNear - zFar);
    const float d = (2.f * zFar * zNear) / (zNear - zFar);
    const float matrix[] = {
        a, 0, 0, 0,
        0, f, 0, 0,
        0, 0, c, -1,
        0, 0, d, 0
    };

    // Switch to the projection matrix and load it with the calculated values
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(matrix);
}

float degToRad(float deg)
{
    return deg * M_PI / 180.f;
}

float radToDeg(float rad)
{
    return rad * 180.f / M_PI;
}
