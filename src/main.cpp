
#include <stdio.h>
#include <stdlib.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
using namespace glm;

// Resolution configs
int WIDTH = 800;
int HEIGHT= 600;

void key_callback (GLFWwindow *window, int key, int scancode, int action, int mode) {
    // When the user preses escale, the window should then close
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
        GLint polyMode;
        glGetIntegerv(GL_POLYGON_MODE, &polyMode);
        if (polyMode == GL_LINE)
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        else if (polyMode == GL_FILL)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
}

int main (int argc, char **argv) {
    // Initialize GLFW
    if ( !glfwInit() ) {
        fprintf (stderr, "Failed to initialize GLFW!\n");
        return EXIT_FAILURE;
    }

    // State the opengl version as 3.3 and make sure that the window can't be resized
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // Create a GLFW window and make it the current context
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "My Render Window", NULL, NULL);
    if (window == NULL) {
        fprintf (stderr, "Failed to create a GLFW window!\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);

    // Set our key callback
    glfwSetKeyCallback(window, key_callback);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if ( glewInit() != GLEW_OK ) {
        fprintf (stderr, "Failed to initialize GLEW!\n");
        return EXIT_FAILURE;
    }

    // State the viewport
    glViewport(0, 0, WIDTH, HEIGHT);

    // Now we're going to write a shader. Typically this would be put in its own file but, uh, fuck it. C string.
    char *vertShaderString = "#version 330 core\n\nlayout(location = 0) in vec3 position;\n\nvoid main() { gl_Position = vec4(position.x, position.y, position.z, 1.0); }";

    // Now we create the shader object
    GLuint vertShader;
    vertShader = glCreateShader(GL_VERTEX_SHADER);

    // Now attach the source code and compile
    glShaderSource(vertShader, 1, &vertShaderString, NULL);
    glCompileShader(vertShader);

    // It's probably important to uh, error check that...
    GLint success;
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(vertShader, 512, NULL, infoLog);
        fprintf (stderr, "Could not compile vertex shader! Log: %s\n", infoLog);
        return EXIT_FAILURE;
    }

    // We've made it this far. Yay! Time to make the fragment shader.
    char *fragShaderString = "#version 330 core\n\nout vec4 color;\n\nvoid main() { color = vec4(1.0f, 0.5f, 0.2f, 1.0f); }";

    // All the same shit except frag
    GLuint fragShader;
    fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fragShaderString, NULL);
    glCompileShader(fragShader);
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(fragShader, 512, NULL, infoLog);
        fprintf (stderr, "Could not compile fragment shader! Log: %s\n", infoLog);
        return EXIT_FAILURE;
    }

    // So we have our shaders, but now we need to link them together in what's called a shader program
    GLuint shaderProgram;
    shaderProgram = glCreateProgram();

    // Easy! Now we attach our shaders and link them.
    glAttachShader(shaderProgram, vertShader);
    glAttachShader(shaderProgram, fragShader);
    glLinkProgram(shaderProgram);

    // Again, uh, error checking...
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        fprintf (stderr, "Could not generate shader program! Log: %s\n", infoLog);
        return EXIT_FAILURE;
    }

    // Now we tell GL to use the shader program. After this, every render call will use this program (and thus, the shaders)
    glUseProgram(shaderProgram);

    // BEE-TEE-DUBZ, dont need these anymore. They're copied into the program object.
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    // Make some vertex data for GL to use. Let's make a quad!
    GLfloat verts[] = {
         0.5f,  0.5f, 0.0f, // Top right
         0.5f, -0.5f, 0.0f, // Bottom right
        -0.5f, -0.5f, 0.0f, // Bottom left
        -0.5f,  0.5f, 0.0f  // Top left
    };

    // Indices tell GL which verts connect to what
    GLuint indices[] = {
        0, 1, 3,    // Tri 1
        1, 2, 3     // Tri 2
    };
    GLuint VBO, VAO, EBO;
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenVertexArrays(1, &VAO);

    // Bind the vertex array
    glBindVertexArray(VAO);

        // Copy our ver array in a buffer for gl to use
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

        // Copy our index array in an element buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // Then set our vertex attribute pointers
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*) 0);
        glEnableVertexAttribArray(0);

    // Finally, unbind
    glBindVertexArray(0);

    // Run the primary loop
    while (!glfwWindowShouldClose(window)) {
        // Check and call our events stack
        glfwPollEvents();

        // Rendering
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);   // Set the clear color
        glClear(GL_COLOR_BUFFER_BIT);           // Clear the buffer

        // DRAW our damn triangle
        glUseProgram(shaderProgram);            // Use the shader program...
        glBindVertexArray(VAO);                 // Bind our VAO...
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);       // Draw arrays...
        glBindVertexArray(0);                   // Unbind that shit so it doesn't get misconfigured you dont want that

        // Swap the buffers
        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return EXIT_SUCCESS;
}
