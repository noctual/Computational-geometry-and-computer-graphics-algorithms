#include <iostream>
#include <deque>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb_image.h"

#define N 100

using namespace std;

GLFWwindow* g_window;

GLuint g_shaderProgram;
GLint g_uMV;
GLint g_uMVP;
// Textures
GLint mapLocation;
GLuint texID;
// Last cursor position.
GLfloat lastX = 0;
GLfloat lastY = 0;
// Rotation matrix.
glm::mat4 new_rot = glm::mat4(1.0f);
// Window size
int g_width = 800;
int g_height = 600;
// Vertices
const unsigned int cnt_vertices = N * N * 3;
GLfloat* vertices;

class Model
{
public:
    GLuint vbo;
    GLuint ibo;
    GLuint vao;
    GLsizei indexCount;
};

Model g_model;

GLuint createShader(const GLchar* code, GLenum type)
{
    GLuint result = glCreateShader(type);

    glShaderSource(result, 1, &code, NULL);
    glCompileShader(result);

    GLint compiled;
    glGetShaderiv(result, GL_COMPILE_STATUS, &compiled);

    if (!compiled)
    {
        GLint infoLen = 0;
        glGetShaderiv(result, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 0)
        {
            char *infoLog = new char[infoLen];
            glGetShaderInfoLog(result, infoLen, NULL, infoLog);
            cout << "Shader compilation error" << endl << infoLog << endl;
            delete[] infoLog;
        }
        glDeleteShader(result);
        return 0;
    }

    return result;
}

GLuint createProgram(GLuint vsh, GLuint fsh)
{
    GLuint result = glCreateProgram();

    glAttachShader(result, vsh);
    glAttachShader(result, fsh);

    glLinkProgram(result);

    GLint linked;
    glGetProgramiv(result, GL_LINK_STATUS, &linked);

    if (!linked)
    {
        GLint infoLen = 0;
        glGetProgramiv(result, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 0)
        {
            char* infoLog = (char*)alloca(infoLen);
            glGetProgramInfoLog(result, infoLen, NULL, infoLog);
            cout << "Shader program linking error" << endl << infoLog << endl;
        }
        glDeleteProgram(result);
        return 0;
    }

    return result;
}

bool createShaderProgram()
{
    g_shaderProgram = 0;

    const GLchar vsh[] =
        "#version 330\n"
        ""
        "layout(location = 0) in vec3 a_position;"
        ""
        "uniform mat4 u_mvp;"
        "uniform mat4 u_mv;"
        ""
        "out float height;"
        ""
        "void main()"
        "{"
        "   vec4 pos = vec4(a_position, 1.0);"
        "   gl_Position = u_mvp * pos;"
        "   height = a_position[1];"
        "}"
        ;

    const GLchar fsh[] =
        "#version 330\n"
        ""
        "uniform sampler1D Texture;"
        "in float height;"
        ""
        "layout(location = 0) out vec4 o_color;"
        ""
        "void main()"
        "{"
        "   o_color = vec4(texture(Texture, height).rgb, 1.0);"
        "}"
        ;

    GLuint vertexShader, fragmentShader;

    vertexShader = createShader(vsh, GL_VERTEX_SHADER);
    fragmentShader = createShader(fsh, GL_FRAGMENT_SHADER);

    g_shaderProgram = createProgram(vertexShader, fragmentShader);

    g_uMV = glGetUniformLocation(g_shaderProgram, "u_mv");
    g_uMVP = glGetUniformLocation(g_shaderProgram, "u_mvp");

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    GLsizei texW, texH, nrChannels;
    GLvoid* image;
    
    if (!(image = stbi_load("texture.png", &texW, &texH, &nrChannels, 0))) {
        return false;
    }

    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_1D, texID);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, texW, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    stbi_image_free(image);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    mapLocation = glGetUniformLocation(g_shaderProgram, "Texture");

    return g_shaderProgram != 0;
}

bool createModel()
{
    vertices = new GLfloat[cnt_vertices];
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            vertices[i * N * 3 + j * 3] = (i - N / 2) / 15.0;
            vertices[i * N * 3 + j * 3 + 1] = 0.0;
            vertices[i * N * 3 + j * 3 + 2] = (j - N / 2) / 15.0;
        }
    }

    const unsigned int cnt_indices = (N - 1) * (N - 1) * 6;
    GLuint* indices = new GLuint[cnt_indices];
    for (int i = 0; i < N - 1; i++) {
        for (int j = 0; j < N - 1; j++) {
            indices[i * (N - 1) * 6 + j * 6] = i * N + j;
            indices[i * (N - 1) * 6 + j * 6 + 1] = i * N + j + 1;
            indices[i * (N - 1) * 6 + j * 6 + 2] = i * N + j + 1 + N;
            indices[i * (N - 1) * 6 + j * 6 + 3] = i * N + j + 1 + N;
            indices[i * (N - 1) * 6 + j * 6 + 4] = i * N + j + N;
            indices[i * (N - 1) * 6 + j * 6 + 5] = i * N + j;
        }
    }

    glGenVertexArrays(1, &g_model.vao);
    glBindVertexArray(g_model.vao);

    glGenBuffers(1, &g_model.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, g_model.vbo);
    glBufferData(GL_ARRAY_BUFFER, cnt_vertices * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &g_model.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_model.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cnt_indices * sizeof(GLuint), indices, GL_STATIC_DRAW);

    g_model.indexCount = cnt_indices;

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (const GLvoid*)0);

    delete[] indices;

    return g_model.vbo != 0 && g_model.ibo != 0 && g_model.vao != 0;
}

bool init()
{
    // Set initial color of color buffer to white.
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    glEnable(GL_DEPTH_TEST);

    return createShaderProgram() && createModel();
}

void reshape(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    g_width = width, g_height = height;
}

void mousePosWrapper(GLFWwindow* window, double xpos, double ypos)
{
    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = ypos - lastY;

    lastX = xpos;
    lastY = ypos;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        new_rot = glm::rotate(new_rot, glm::radians(xoffset / g_width * 90.0f), glm::vec3(0, 1, 0));
        new_rot = glm::rotate(new_rot, glm::radians(yoffset / g_height * 90.0f), glm::vec3(1, 0, 0));
    }
}

void draw()
{
    // Clear color buffer.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(g_shaderProgram);
    glBindVertexArray(g_model.vao);

    glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);

    glm::mat4 View = glm::lookAt(
        glm::vec3(12, 12, 12),
        glm::vec3(0, 0, 0),
        glm::vec3(0, 1, 0)
    );

    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::rotate(Model, glm::radians(-15.0f), glm::vec3(1, 1, 0));
    Model = new_rot * Model;

    glUniformMatrix4fv(g_uMV, 1, GL_FALSE, glm::value_ptr(View * Model));
    glUniformMatrix4fv(g_uMVP, 1, GL_FALSE, glm::value_ptr(Projection * View * Model));

    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_1D, texID);
    glUniform1i(mapLocation, 0);

    glDrawElements(GL_TRIANGLES, g_model.indexCount, GL_UNSIGNED_INT, NULL);
}

void cleanup()
{
    if (g_shaderProgram != 0)
        glDeleteProgram(g_shaderProgram);
    if (g_model.vbo != 0)
        glDeleteBuffers(1, &g_model.vbo);
    if (g_model.ibo != 0)
        glDeleteBuffers(1, &g_model.ibo);
    if (g_model.vao != 0)
        glDeleteVertexArrays(1, &g_model.vao);
    if (texID != 0)
        glDeleteTextures(1, &texID);
    delete[] vertices;
}

bool initOpenGL()
{
    // Initialize GLFW functions.
    if (!glfwInit())
    {
        cout << "Failed to initialize GLFW" << endl;
        return false;
    }

    // Request OpenGL 3.3 without obsoleted functions.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window.
    g_window = glfwCreateWindow(g_width, g_height, "OpenGL Test", NULL, NULL);
    if (g_window == NULL)
    {
        cout << "Failed to open GLFW window" << endl;
        glfwTerminate();
        return false;
    }

    // Initialize OpenGL context with.
    glfwMakeContextCurrent(g_window);

    // Set internal GLEW variable to activate OpenGL core profile.
    glewExperimental = true;

    // Initialize GLEW functions.
    if (glewInit() != GLEW_OK)
    {
        cout << "Failed to initialize GLEW" << endl;
        return false;
    }

    // Ensure we can capture the escape key being pressed.
    glfwSetInputMode(g_window, GLFW_STICKY_KEYS, GL_TRUE);

    // Set callback for framebuffer resizing event.
    glfwSetFramebufferSizeCallback(g_window, reshape);

    // Set callback for changes cursor position.
    glfwSetCursorPosCallback(g_window, mousePosWrapper);

    return true;
}

void tearDownOpenGL()
{
    // Terminate GLFW.
    glfwTerminate();
}

void update(deque<vector<float>>& deq) {
    
    vector<float> random;
    for (int i = 0; i < N; i++)
        random.push_back(rand()%12 / 10.0);
    deq.push_back(random);
    while (deq.size() > N) {
        deq.pop_front();
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            vertices[i * N * 3 + j * 3 + 1] = (deq.size() > i ? deq[i][j] : 0.0);
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, g_model.vbo);
    glBufferData(GL_ARRAY_BUFFER, cnt_vertices * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
}

int main()
{
    // Initialize OpenGL
    if (!initOpenGL())
        return -1;

    // Initialize graphical resources.
    bool isOk = init();

    if (isOk)
    {
        deque<vector<float>> deq;

        // Main loop until window closed or escape pressed.
        while (glfwGetKey(g_window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(g_window) == 0)
        {
            // Update spectr
            update(deq);
            // Draw scene.
            draw();

            // Swap buffers.
            glfwSwapBuffers(g_window);
            // Poll window events.
            glfwPollEvents();
        }
    }

    // Cleanup graphical resources.
    cleanup();

    // Tear down OpenGL.
    tearDownOpenGL();

    return isOk ? 0 : -1;
}
