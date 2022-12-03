#include <GL/glew.h>
#include <glm/ext.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

void prepObjects(unsigned& vbo, unsigned& vao);
unsigned prepVertShader();
unsigned prepFragShader();
unsigned prepShaderProgram();

int main()
{
    // Create the window
    sf::RenderWindow window({1280, 720}, "Blocky Basement", sf::Style::Default, sf::ContextSettings{24, 0, 0, 3, 2, sf::ContextSettings::Core});
    window.setFramerateLimit(60);

    // GLEW allows accessing Modern OpenGL functions
    glewInit();

    // Enable depth testing and face culling
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

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

    // Set the window area in which OpenGL will draw
    glViewport(0, 0, window.getSize().x, window.getSize().y);

    // Prepare shaders used for drawing
    unsigned shaderProgram = prepShaderProgram();
    glUseProgram(shaderProgram);

    // Prepare objects used for drawing
    unsigned vbo, vao;
    prepObjects(vbo, vao);
    glBindVertexArray(vao);

    // Calculate the perspective matrix
    glm::mat4 proj = glm::perspective(glm::radians(90.f), (float)window.getSize().x / (float)window.getSize().y, 0.1f, 100.f);

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
                proj = glm::perspective(glm::radians(90.f), (float)window.getSize().x / (float)window.getSize().y, 0.1f, 100.f);
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
        // Clear the screen and the depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw every wall
        for (unsigned i = 0; i < image.getSize().y; i++)
        {
            for (unsigned j = 0; j < image.getSize().x; j++)
            {
                // Walls are designated by white pixels
                if (image.getPixel(j, i) != sf::Color::White)
                    continue;

                // Transform the view matrix by the camera position, rotation
                // and the wall position
                glm::mat4 view = glm::mat4(1.f);
                view = glm::rotate(view, cameraRotation, glm::vec3(0.f, 1.f, 0.f));
                view = glm::translate(view, glm::vec3(-cameraPosition.x, -cameraPosition.y, -cameraPosition.z));
                view = glm::translate(view, glm::vec3(j, 0.f, i));

                // Pass the view and projection matrices and draw the wall
                glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
                glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
                glDrawArrays(GL_TRIANGLES, 0, 6 * 4);
            }
        }

        // Make the GPU actually render everything
        window.display();
    }

    return 0;
}

void prepObjects(unsigned& vbo, unsigned& vao)
{
    // Array containing all colors and vertices of a wall
    // Format: R G B X Y Z
    static const float WallData[] = {
        // Front wall
        1.f, 0.f, 0.f, 0.f, 0.f, 1.f,
        1.f, 0.f, 0.f, 1.f, 0.f, 1.f,
        1.f, 0.f, 0.f, 1.f, 1.f, 1.f,

        1.f, 0.f, 0.f, 0.f, 0.f, 1.f,
        1.f, 0.f, 0.f, 1.f, 1.f, 1.f,
        1.f, 0.f, 0.f, 0.f, 1.f, 1.f,

        // Back wall
        0.f, 1.f, 0.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f, 1.f, 0.f,
        0.f, 1.f, 0.f, 1.f, 1.f, 0.f,

        0.f, 1.f, 0.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 1.f, 1.f, 0.f,
        0.f, 1.f, 0.f, 1.f, 0.f, 0.f,

        // Left wall
        0.f, 0.f, 1.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 1.f, 1.f, 0.f,
        0.f, 0.f, 1.f, 1.f, 1.f, 1.f,

        0.f, 0.f, 1.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 1.f, 1.f, 1.f,
        0.f, 0.f, 1.f, 1.f, 0.f, 1.f,

        // Right wall
        1.f, 1.f, 0.f, 0.f, 0.f, 0.f,
        1.f, 1.f, 0.f, 0.f, 0.f, 1.f,
        1.f, 1.f, 0.f, 0.f, 1.f, 1.f,

        1.f, 1.f, 0.f, 0.f, 0.f, 0.f,
        1.f, 1.f, 0.f, 0.f, 1.f, 1.f,
        1.f, 1.f, 0.f, 0.f, 1.f, 0.f
    };

    // Create the vertex buffer object
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(WallData), WallData, GL_STATIC_DRAW);

    // Create the vertex array object
    glGenVertexArrays(1, &vao);

    // Specify the VBO
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // Specify the data layout
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

unsigned prepVertShader()
{
    static const char* Src =
    "#version 150 core\n"
    "\n"
    "in vec3 color;\n"
    "in vec3 pos;\n"
    "\n"
    "uniform mat4 view;\n"
    "uniform mat4 proj;\n"
    "\n"
    "out vec3 passedColor;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    gl_Position = proj * view * vec4(pos.x, pos.y, pos.z, 1.0);\n"
    "    passedColor = color;\n"
    "}\n";

    unsigned shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shader, 1, &Src, nullptr);
    glCompileShader(shader);

    return shader;
}

unsigned prepFragShader()
{
    static const char* Src =
    "#version 150 core\n"
    "\n"
    "in vec3 passedColor;\n"
    "out vec4 color;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    color = vec4(passedColor.x, passedColor.y, passedColor.z, 1.0);\n"
    "}\n";

    unsigned shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shader, 1, &Src, nullptr);
    glCompileShader(shader);

    return shader;
}

unsigned prepShaderProgram()
{
    // Prepare the vertex and fragment shaders
    unsigned vertShader = prepVertShader();
    unsigned fragShader = prepFragShader();

    // Prepare the shader program
    unsigned shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertShader);
    glAttachShader(shaderProgram, fragShader);
    glLinkProgram(shaderProgram);

    // Separate shaders are no longer used and can be deleted
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    return shaderProgram;
}
