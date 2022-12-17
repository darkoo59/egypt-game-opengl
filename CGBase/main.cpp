#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <chrono>
#include <thread>
#include <cstdlib>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include "shader.hpp"
#include "model.hpp"
#include "renderable.hpp"


const int WindowWidth = 1280;
const int WindowHeight = 720;
const std::string WindowTitle = "Egypt world";
const float TargetFPS = 60.0f;
const float TargetFrameTime = 1.0f / TargetFPS;
const int steps = 100;
const float moonAngle = 3.1415926 * 2.f / steps;

void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

static void
KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    bool IsDown = action == GLFW_PRESS || action == GLFW_REPEAT;
    switch (key) {
    case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
    }
}

static void
ErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error: " << description << std::endl;
}

void processInput(GLFWwindow* context, float& x, float& y, float& z)
{
    if (glfwGetKey(context, GLFW_KEY_A) == GLFW_PRESS)
    {
        x += 0.25;
        if (x > 13)
            x = 13;
    }
    if (glfwGetKey(context, GLFW_KEY_D) == GLFW_PRESS)
    {
        x -= 0.25;
        if (x < -13)
            x = -13;
    }
    if (glfwGetKey(context, GLFW_KEY_W) == GLFW_PRESS)
    {
        y += 0.25;
        if (y > 13)
            y = 13;
    }
    if (glfwGetKey(context, GLFW_KEY_S) == GLFW_PRESS)
    {
        y -= 0.25;
        if (y < -4)
            y = -4;
    }
}

float x = 0, y = 0, z = 0;

int main() {
    GLFWwindow* Window = 0;
    if (!glfwInit()) {
        std::cerr << "Failed to init glfw" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwSetErrorCallback(ErrorCallback);

    Window = glfwCreateWindow(WindowWidth, WindowHeight, WindowTitle.c_str(), 0, 0);
    if (!Window) {
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(Window);
    glfwSetKeyCallback(Window, KeyCallback);
    glfwSetFramebufferSizeCallback(Window, FramebufferSizeCallback);

    GLenum GlewError = glewInit();
    if (GlewError != GLEW_OK) {
        std::cerr << "Failed to init glew: " << glewGetErrorString(GlewError) << std::endl;
        glfwTerminate();
        return -1;
    }

    Shader Basic("shaders/basic.vert", "shaders/basic.frag");
    Shader RugShader("shaders/rug.vert", "shaders/basic.frag");

    //Ucitavanje modela tepiha
    Model Rug("models/Rug.obj");
    if (!Rug.Load())
    {
        std::cout << "Failed to load rug model!\n";
        glfwTerminate();
        return -1;
    }

    //Ucitavanje modela figure
    Model Figure("models/anubis.obj");
    if (!Figure.Load())
    {
        std::cout << "Failed to load anubis model!\n";
        glfwTerminate();
        return -1;
    }

    float cubeVertices[] = //Temena kocke koriscene za mesec
    {
        -0.2, -0.2, -0.2,       0.0, 0.0, 0.0,
        +0.2, -0.2, -0.2,       0.0, 0.0, 0.0,
        -0.2, -0.2, +0.2,       0.0, 0.0, 0.0,
        +0.2, -0.2, +0.2,       0.0, 0.0, 0.0,

        -0.2, +0.2, -0.2,       0.0, 0.0, 0.0,
        +0.2, +0.2, -0.2,       0.0, 0.0, 0.0,
        -0.2, +0.2, +0.2,       0.0, 0.0, 0.0,
        +0.2, +0.2, +0.2,       0.0, 0.0, 0.0,
    };

    float pyramidVertices[] =  //Temena najblize piramide
    {
        -1.0, 0.0, -1.0,        0.0, 0.0, 0.0,      //0
        - 1.0, 0.0,  1.0,       0.0, 0.0, 0.0,      //1
         1.0, 0.0, -1.0,        0.0, 0.0, 0.0,      //2
         1.0, 0.0,  1.0,        0.0, 0.0, 0.0,      //3
         -1.0, 3.0,  0.0,        0.0, 0.0, 0.0,      //4
    };

    float secondpyramidVertices[] = //Temena srednje piramide
    {
        -1.0, 0.0, -1.0,        0.0, 0.0, 0.0,      //0
        -1.0, 0.0,  1.0,       0.0, 0.0, 0.0,      //1
         1.0, 0.0, -1.0,        0.0, 0.0, 0.0,      //2
         1.0, 0.0,  1.0,        0.0, 0.0, 0.0,      //3
         1.0, 3.0,  0.0,        0.0, 0.0, 0.0,      //4
    };

    float thirdpyramidVertices[] =  //Temena najdalje piramide
    {
        -1.0, 0.0, -1.0,        0.0, 0.0, 0.0,      //0
        -1.0, 0.0,  1.0,       0.0, 0.0, 0.0,      //1
         1.0, 0.0, -1.0,        0.0, 0.0, 0.0,      //2
         1.0, 0.0,  1.0,        0.0, 0.0, 0.0,      //3
         0.0, 3.0,  0.0,        0.0, 0.0, 0.0,      //4
    };

    unsigned int pyramidIndices[] = {
        0, 1, 2,    //good
        1, 3, 2,    //good

        0, 4, 1,    //GOOD

        1, 4, 3,    //good

        2, 3, 4,   //good 

        0, 2, 4
    };
    

    unsigned int cubeIndices[] = {
        0, 1, 3,
        0, 2, 3,

        4, 6, 7,
        4, 5, 7,

        3, 6, 2,
        3, 6, 7,

        0, 4, 1,
        1, 4, 5,

        0, 6, 2,
        0, 4, 6,

        1, 3, 7,
        1, 7, 5
    };

    //Renderable objekti kocke i piramida
    Renderable cube(cubeVertices, sizeof(cubeVertices), cubeIndices, sizeof(cubeIndices));
    Renderable pyramid(pyramidVertices, sizeof(pyramidVertices), pyramidIndices, sizeof(pyramidIndices));
    Renderable secondPyramid(secondpyramidVertices, sizeof(secondpyramidVertices), pyramidIndices, sizeof(pyramidIndices));
    Renderable thirdPyramid(thirdpyramidVertices, sizeof(thirdpyramidVertices), pyramidIndices, sizeof(pyramidIndices));

    //Podesavanje kamere
    glm::mat4 m(1.0f);
    glm::mat4 v = glm::lookAt(glm::vec3(0.0, -0.5, -2.5), glm::vec3(0.0, 0.4, 0.0), glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 p = glm::perspective(glm::radians(90.0f), (float)WindowWidth / WindowHeight, 0.1f, 100.0f);

    //Ukljucivanje testiranja dubine i backface culling
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(0.09020, 0.09020, 0.09020, 0.4);


    float FrameStartTime = glfwGetTime();
    float FrameEndTime = glfwGetTime();
    float dt = FrameEndTime - FrameStartTime;
    while (!glfwWindowShouldClose(Window)) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        FrameStartTime = glfwGetTime();
        glUseProgram(Basic.GetId());


        Basic.SetProjection(p);
        Basic.SetView(v);


        //Crtanje meseca
        Basic.SetColor(0.49020, 0.49020, 0.49020);
        float moonAngleRotate = 0.0;
        for (int i = 0; i < steps; i++) {
            m = glm::translate(glm::mat4(1.0f), glm::vec3(5.0, 3.5, 0));
            m = glm::scale(m, glm::vec3(0.7, 0.7, 0.7));
            m = glm::rotate(m, glm::radians(moonAngleRotate), glm::vec3(1.0, 1.0, 1.0));
            Basic.SetModel(m);
            cube.Render();
            moonAngleRotate += 5.0;
        }

        //Crtanje anubisa 1
        m = glm::mat4(1.0);
        m = glm::translate(m, glm::vec3(-3.2, -0.5, -0.25));
        m = glm::scale(m, glm::vec3(0.002, 0.002, 0.002));
        m = glm::rotate(m, glm::radians(150.0f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::rotate(m, glm::radians(25.0f), glm::vec3(1.0, 0.0, 0.0));
        Basic.SetModel(m);
        Figure.Render();

        //Crtanje anubisa 2
        m = glm::mat4(1.0);
        m = glm::translate(m, glm::vec3(2.9, -0.50, -0.25));
        m = glm::scale(m, glm::vec3(0.0015, 0.0015, 0.0015));
        m = glm::rotate(m, glm::radians(240.0f), glm::vec3(0.0, 1.0, 0.0));
        m = glm::rotate(m, glm::radians(12.0f), glm::vec3(0.0, 0.0, 1.0));

        Basic.SetModel(m);
        Figure.Render();

        //Crtanje anubisa 3
        m = glm::mat4(1.0);
        m = glm::translate(m, glm::vec3(1.0, 0.0, 3.00));
        m = glm::scale(m, glm::vec3(0.0010, 0.0010, 0.0010));

        Basic.SetModel(m);
        Figure.Render();
        
        //Crtanje peska
        Basic.SetColor(0.40000, 0.20784, 0.00000);
        m = glm::translate(glm::mat4(1.0f), glm::vec3(0, -2.5, 0.0));
        m = glm::scale(m, glm::vec3(25.0, 11.0, 0.0));
        Basic.SetModel(m);
        cube.Render();

        //Crtanje piramide 1
        Basic.SetColor(0.20000, 0.10196, 0.00000);
        m = glm::scale(glm::mat4(1.0f), glm::vec3(0.5, 0.5, 0.5));
        m = glm::translate(m, glm::vec3(-1.2, -1.5, -2.5));
        m = glm::rotate(m, glm::radians(10.0f), glm::vec3(0.0, 1.0, 0.0));
        Basic.SetModel(m);
        pyramid.Render();

        //Crtanje piramide 2
        Basic.SetColor(0.20000, 0.10196, 0.00000);
        m = glm::scale(glm::mat4(1.0f), glm::vec3(0.37, 0.37, 0.37));
        m = glm::translate(m, glm::vec3(3.8, -2.0, -2.0));
        m = glm::rotate(m, glm::radians(-5.0f), glm::vec3(0.0, 1.0, 0.0));
        Basic.SetModel(m);
        secondPyramid.Render();

        //Crtanje piramide 3
        Basic.SetColor(0.20000, 0.10196, 0.00000);
        m = glm::scale(glm::mat4(1.0f), glm::vec3(0.2, 0.2, 0.2));
        m = glm::translate(m, glm::vec3(5.0, -1.2, 5.0));
        m = glm::rotate(m, glm::radians(-30.0f), glm::vec3(0.0, 1.0, 0.0));
        Basic.SetModel(m);
        thirdPyramid.Render();

        //Crtanje tepiha
        Basic.SetColor(0, 0, 0);
        m = glm::scale(glm::mat4(1.0f), glm::vec3(0.25, 0.25, 0.25));
        m = glm::translate(m, glm::vec3(0.5, 1.75 + ((double)rand() / (RAND_MAX))/8, +0.5));
        //Kretanje tepiha
        processInput(Window, x, y, z);
        m = glm::translate(m, glm::vec3(x, y, z));
        Basic.SetModel(m);
        Rug.Render();

        glfwSwapBuffers(Window);

        FrameEndTime = glfwGetTime();
        dt = FrameEndTime - FrameStartTime;
        if (dt < TargetFPS) {
            int DeltaMS = (int)((TargetFrameTime - dt) * 1e3f);
            std::this_thread::sleep_for(std::chrono::milliseconds(DeltaMS));
            FrameEndTime = glfwGetTime();
        }
        dt = FrameEndTime - FrameStartTime;
    }


    glfwTerminate();
    return 0;
}



