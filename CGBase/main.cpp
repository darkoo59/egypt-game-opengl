#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include <thread>
#include "shader.hpp"
#include "camera.hpp"
#include "model.hpp"
#include "texture.hpp"
#include "renderable.hpp"


int WindowWidth = 1280;
int WindowHeight = 720;
const float TargetFPS = 60.0f;
const std::string WindowTitle = "Egypt world";
const int steps = 360;
const float moonAngle = 3.1415926 * 2.f / steps;
float x = 0.0, y = 4.0, z = -26.0;
unsigned pyramidVerticesCount;
double spotlightX = 0.0;
double spotlightY = -0.1;
double spotlightZ = 0.0;

struct Input {
    bool MoveLeft;
    bool MoveRight;
    bool MoveUp;
    bool MoveDown;
    bool LookLeft;
    bool LookRight;
    bool LookUp;
    bool LookDown;
    bool RugLeft;
    bool RugRight;
    bool RugUp;
    bool RugDown;
};

struct EngineState {
    Input* mInput;
    Camera* mCamera;
    bool mDrawDebugLines;
    float mDT;
};

static void
ErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error: " << description << std::endl;
}

static void
KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    EngineState* State = (EngineState*)glfwGetWindowUserPointer(window);
    Input* UserInput = State->mInput;
    bool IsDown = action == GLFW_PRESS || action == GLFW_REPEAT;
    switch (key) {
    case GLFW_KEY_A: UserInput->MoveLeft = IsDown; break;
    case GLFW_KEY_D: UserInput->MoveRight = IsDown; break;
    case GLFW_KEY_W: UserInput->MoveUp = IsDown; break;
    case GLFW_KEY_S: UserInput->MoveDown = IsDown; break;

    case GLFW_KEY_RIGHT: UserInput->LookLeft = IsDown; break;
    case GLFW_KEY_LEFT: UserInput->LookRight = IsDown; break;
    case GLFW_KEY_UP: UserInput->LookUp = IsDown; break;
    case GLFW_KEY_DOWN: UserInput->LookDown = IsDown; break;

    case GLFW_KEY_L: UserInput->RugLeft = IsDown; break;
    case GLFW_KEY_J: UserInput->RugRight = IsDown; break;
    case GLFW_KEY_I: UserInput->RugUp = IsDown; break;
    case GLFW_KEY_K: UserInput->RugDown = IsDown; break;

    case GLFW_KEY_C: {
        if (IsDown) {
            State->mDrawDebugLines ^= true; break;
        }
    } break;

    case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
    }
}

static void
FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    WindowWidth = width;
    WindowHeight = height;
    glViewport(0, 0, width, height);
}

static void
HandleInput(EngineState* state, const Shader& shader) {
    Input* UserInput = state->mInput;
    Camera* FPSCamera = state->mCamera;
    if (UserInput->MoveLeft) FPSCamera->Move(-1.0f, 0.0f, state->mDT);
    if (UserInput->MoveRight) FPSCamera->Move(1.0f, 0.0f, state->mDT);
    if (UserInput->MoveDown) FPSCamera->Move(0.0f, -1.0f, state->mDT);
    if (UserInput->MoveUp) FPSCamera->Move(0.0f, 1.0f, state->mDT);

    if (UserInput->LookLeft) FPSCamera->Rotate(1.0f, 0.0f, state->mDT);
    if (UserInput->LookRight) FPSCamera->Rotate(-1.0f, 0.0f, state->mDT);
    if (UserInput->LookDown) FPSCamera->Rotate(0.0f, -1.0f, state->mDT);
    if (UserInput->LookUp) FPSCamera->Rotate(0.0f, 1.0f, state->mDT);

    if (UserInput->RugLeft)
    {
        glUseProgram(shader.GetId());
        x += 0.25;
        spotlightX += 1.00;
        shader.SetUniform3f("uSpotlight.Direction", glm::vec3(spotlightX, spotlightY, spotlightZ));
        glUseProgram(0);
    }
    if (UserInput->RugRight)
    {
        glUseProgram(shader.GetId());
        x -= 0.25;
        spotlightX -= 1.0;
        shader.SetUniform3f("uSpotlight.Direction", glm::vec3(spotlightX, spotlightY, spotlightZ));
        glUseProgram(0);
    }
    if (UserInput->RugDown)
    {
        glUseProgram(shader.GetId());
        y -= 0.25;
        spotlightY -= 0.25;
        if (y < -3.5)
            y = -3.5;
        else
            shader.SetUniform3f("uSpotlight.Direction", glm::vec3(spotlightX, spotlightY, spotlightZ));
        glUseProgram(0);
    }
    if (UserInput->RugUp)
    {
        glUseProgram(shader.GetId());
        y += 0.25;
        spotlightY -= 0.25;
        if (y > 13)
            y = 13;
        else
            shader.SetUniform3f("uSpotlight.Direction", glm::vec3(spotlightX, spotlightY, spotlightZ));
        glUseProgram(0);
    }
}

static void
DrawFloor(unsigned vao, const Shader& shader, unsigned diffuse) {
    glUseProgram(shader.GetId());
    glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuse);
    float Size = 4.0f;
    for (int i = -2; i < 4; ++i) {
        for (int j = -2; j < 4; ++j) {
            glm::mat4 Model(1.0f);
            Model = glm::translate(Model, glm::vec3(2.0, -2.0f, 2.0));
            Model = glm::scale(Model, glm::vec3(50 * Size, 0.1f, 50 * Size));
            shader.SetModel(Model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }

    glBindVertexArray(0);
    glUseProgram(0);
}

static void
DrawMoon(unsigned vao, const Shader& shader, unsigned diffuse) {
    glUseProgram(shader.GetId());
    glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuse);
    float moonAngleRotate = 0.0;
    for (int i = 0; i < steps; i++) {
        glm::mat4 Model(1.0f);
        Model = glm::translate(glm::mat4(1.0f), glm::vec3(-5.0, 30.5, -30.0));
        Model = glm::scale(Model, glm::vec3(5, 5, 5));
        Model = glm::rotate(Model, glm::radians(moonAngleRotate), glm::vec3(1.0, 0.0, 0.0));
        Model = glm::rotate(Model, glm::radians(moonAngleRotate), glm::vec3(1.0, 1.0, 0.0));
        shader.SetModel(Model);
        glDrawArrays(GL_TRIANGLES, 0, 18);
        moonAngleRotate += 1.0;
    }

    glBindVertexArray(0);
    glUseProgram(0);
}

static void
DrawPyramid(unsigned vao, const Shader& shader, glm::vec3 position, glm::vec3 scale, unsigned diffuse) {
    glUseProgram(shader.GetId());
    glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuse);
    glm::mat4 ModelMatrix(1.0f);
    ModelMatrix = glm::translate(ModelMatrix, position);
    ModelMatrix = glm::scale(ModelMatrix, scale);
    shader.SetModel(ModelMatrix);
    glDrawArrays(GL_TRIANGLES, 0, pyramidVerticesCount);
    glBindVertexArray(0);
    glUseProgram(0);
}

static void
DrawStone(const Shader& shader, glm::vec3 position, glm::vec3 scale) {
    glm::mat4 ModelMatrix(1.0f);
    ModelMatrix = glm::mat4(1.0f);
    ModelMatrix = glm::translate(ModelMatrix, position);
    ModelMatrix = glm::scale(ModelMatrix, scale);
    shader.SetModel(ModelMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

static void
DrawStones(unsigned vao, const Shader& shader, unsigned diffuse) {
    glUseProgram(shader.GetId());
    glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuse);

    DrawStone(shader, glm::vec3(5.1f, -2.5f, 14.0f), glm::vec3(1.5f));
    DrawStone(shader, glm::vec3(10.1f, -2.5f, 3.0f), glm::vec3(1.5f));
    DrawStone(shader, glm::vec3(-51.1f, -2.5f, -13.0f), glm::vec3(1.5f));
    DrawStone(shader, glm::vec3(-10.1f, -2.5f, -3.0f), glm::vec3(1.5f));
    DrawStone(shader, glm::vec3(15.1f, -2.5f, 34.0f), glm::vec3(1.5f));
    DrawStone(shader, glm::vec3(1.1f, -2.5f, -23.0f), glm::vec3(1.5f));

    DrawStone(shader, glm::vec3(16.1f, -2.5f, -14.0f), glm::vec3(1.5f));
    DrawStone(shader, glm::vec3(50.1f, -2.5f, -3.0f), glm::vec3(1.5f));
    DrawStone(shader, glm::vec3(-31.1f, -2.5f, 13.0f), glm::vec3(1.5f));
    DrawStone(shader, glm::vec3(-13.1f, -2.5f, 3.0f), glm::vec3(1.5f));
    DrawStone(shader, glm::vec3(46.1f, -2.5f, -34.0f), glm::vec3(1.5f));
    DrawStone(shader, glm::vec3(2.1f, -2.5f, 23.0f), glm::vec3(1.5f));

    glBindVertexArray(0);
    glUseProgram(0);
}

int main() {
    GLFWwindow* Window = 0;
    if (!glfwInit()) {
        std::cerr << "Failed to init glfw" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    Window = glfwCreateWindow(WindowWidth, WindowHeight, WindowTitle.c_str(), 0, 0);
    if (!Window) {
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(Window);

    GLenum GlewError = glewInit();
    if (GlewError != GLEW_OK) {
        std::cerr << "Failed to init glew: " << glewGetErrorString(GlewError) << std::endl;
        glfwTerminate();
        return -1;
    }

    EngineState State = { 0 };
    Camera FPSCamera;
    Input UserInput = { 0 };
    State.mCamera = &FPSCamera;
    State.mInput = &UserInput;
    glfwSetWindowUserPointer(Window, &State);

    glfwSetErrorCallback(ErrorCallback);
    glfwSetFramebufferSizeCallback(Window, FramebufferSizeCallback);
    glfwSetKeyCallback(Window, KeyCallback);

    glViewport(0.0f, 0.0f, WindowWidth, WindowHeight);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    unsigned FloorDiffuseTexture = Texture::LoadImageToTexture("resources/Sand_Diffuse.jpg");
    unsigned MoonDiffuseTexture = Texture::LoadImageToTexture("resources/Moon_Diffuse.jpg");
    unsigned PyramidDiffuseTexture = Texture::LoadImageToTexture("resources/Pyramid_Diffuse.jpg");
    unsigned StoneSpecularTexture = Texture::LoadImageToTexture("resources/Stone_Specular2.jpg");
    unsigned CarpetTexture = Texture::LoadImageToTexture("resources/rug/rug-Diff.png");
    unsigned ChairTexture = Texture::LoadImageToTexture("resources/anubis/Diffuse.jpg");

    std::vector<float> CubeVertices = {
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
         0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
         0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,

        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,

         0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,

        -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,

         0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
         0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
    };
    unsigned CubeVAO;
    glGenVertexArrays(1, &CubeVAO);
    glBindVertexArray(CubeVAO);
    unsigned CubeVBO;
    glGenBuffers(1, &CubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, CubeVBO);
    glBufferData(GL_ARRAY_BUFFER, CubeVertices.size() * sizeof(float), CubeVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    std::vector<float> pyramidVertices =
    {
        -0.5f, 0.0f, 0.5f,     0.0f, -1.0f, 0.0f,       0.0f, 0.0f,
        0.5f, 0.0f, -0.5f,     0.0f, -1.0f, 0.0f,       4.0f, 0.0f,
        0.5f, 0.0f, 0.5f,      0.0f, -1.0f, 0.0f,       4.0f, 4.0f,
        0.5f, 0.0f, -0.5f,     0.0f, -1.0f, 0.0f,       4.0f, 4.0f,
        -0.5f, 0.0f, 0.5f,     0.0f, -1.0f, 0.0f,       0.0f, 4.0f,
        -0.5f, 0.0f, -0.5f,    0.0f, -1.0f, 0.0f,       0.0f, 0.0f,

        -0.5f, 0.0f, 0.5f,     -0.6f, 0.5f, 0.0f,       4.0f, 0.0f,
        0.0f, 0.6f, 0.0f,      -0.6f, 0.5f, 0.0f,       2.0f, 2.0f,
        -0.5f, 0.0f, -0.5f,    -0.6f, 0.5f, 0.0f,       0.0f, 0.0f,

        -0.5f, 0.0f, -0.5f,     0.0f, 0.5f, -0.6f,      2.0f, 0.0f,
        0.0f, 0.6f, 0.0f,       0.0f, 0.5f, -0.6f,      2.0f, 2.0f,
        0.5f, 0.0f, -0.5f,      0.0f, 0.5f, -0.6f,      0.0f, 0.0f,

        0.5f, 0.0f, -0.5f,      0.6f, 0.5f, 0.0f,       6.0f, 0.0f,
        0.0f, 0.6f, 0.0f,       0.6f, 0.5f, 0.0f,       2.0f, 3.0f,
        0.5f, 0.0f, 0.5f,       0.6f, 0.5f, 0.0f,       0.0f, 0.0f,

        0.5f, 0.0f, 0.5f,       0.0f, 0.5f, 0.6f,       4.0f, 0.0f,
        0.0f, 0.6f, 0.0f,       0.0f, 0.5f, 0.6f,       2.0f, 2.0f,
        -0.5f, 0.0f, 0.5f,      0.0f, 0.5f, 0.6f,       0.0f, 0.0f
    };

    pyramidVerticesCount = CubeVertices.size();
    unsigned PyramidVAO;     
    glGenVertexArrays(1, &PyramidVAO); 
    glBindVertexArray(PyramidVAO); 
    unsigned PyramidVBO; 
    glGenBuffers(1, &PyramidVBO); 
    glBindBuffer(GL_ARRAY_BUFFER, PyramidVBO);   
    glBufferData(GL_ARRAY_BUFFER, pyramidVertices.size() * sizeof(float), pyramidVertices.data(), GL_STATIC_DRAW); 

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); 
    glEnableVertexAttribArray(0);    

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    Model Rug("resources/rug/rug.obj");
    if (!Rug.Load())
    {
        std::cout << "Failed to load rug model!\n";
        glfwTerminate();
        return -1;
    }

    Model Egy("resources/anubis/Egy1.obj");
    if (!Egy.Load())
    {
        std::cout << "Failed to load egy model!\n";
        glfwTerminate();
        return -1;
    }

    Shader ColorShader("shaders/color.vert", "shaders/color.frag");

    Shader PhongShaderMaterialTexture("shaders/basic.vert", "shaders/phong_material_texture.frag");
    glUseProgram(PhongShaderMaterialTexture.GetId());
    PhongShaderMaterialTexture.SetUniform3f("uDirLight.Position", glm::vec3(-5.0, 30.5, -30.0));
    PhongShaderMaterialTexture.SetUniform3f("uDirLight.Direction", glm::vec3(1.0f, -150.0f, 1.0f));
    PhongShaderMaterialTexture.SetUniform3f("uDirLight.Ka", glm::vec3(0.55020, 0.55020, 0.55020));
    PhongShaderMaterialTexture.SetUniform3f("uDirLight.Kd", glm::vec3(0.55020, 0.55020, 0.55020));
    PhongShaderMaterialTexture.SetUniform3f("uDirLight.Ks", glm::vec3(1.0f));

    PhongShaderMaterialTexture.SetUniform3f("uPointLight.Position", glm::vec3(65.1f, 10.0f, 0.0f));
    PhongShaderMaterialTexture.SetUniform3f("uPointLight.Direction", glm::vec3(0.0f, -5.0f, 1.0f));
    PhongShaderMaterialTexture.SetUniform3f("uPointLight.Ka", glm::vec3(1.00000f, 1.00000f, 1.00000f));
    PhongShaderMaterialTexture.SetUniform3f("uPointLight.Kd", glm::vec3(1.00000f, 1.00000f, 1.00000f));
    PhongShaderMaterialTexture.SetUniform3f("uPointLight.Ks", glm::vec3(1.0f));
    PhongShaderMaterialTexture.SetUniform1f("uPointLight.Kc", 1.0f);
    PhongShaderMaterialTexture.SetUniform1f("uPointLight.Kl", 0.092f);
    PhongShaderMaterialTexture.SetUniform1f("uPointLight.Kq", 0.032f);

    PhongShaderMaterialTexture.SetUniform3f("uPointLightSecond.Position", glm::vec3(5.0f, 10.0f, 30.0f));
    PhongShaderMaterialTexture.SetUniform3f("uPointLightSecond.Direction", glm::vec3(0.0f, -5.0f, 1.0f));
    PhongShaderMaterialTexture.SetUniform3f("uPointLightSecond.Ka", glm::vec3(1.00000f, 1.00000f, 0.90196f));
    PhongShaderMaterialTexture.SetUniform3f("uPointLightSecond.Kd", glm::vec3(1.00000f, 1.00000f, 0.90196f));
    PhongShaderMaterialTexture.SetUniform3f("uPointLightSecond.Ks", glm::vec3(1.0f));
    PhongShaderMaterialTexture.SetUniform1f("uPointLightSecond.Kc", 1.0f);
    PhongShaderMaterialTexture.SetUniform1f("uPointLightSecond.Kl", 0.092f);
    PhongShaderMaterialTexture.SetUniform1f("uPointLightSecond.Kq", 0.032f);

    PhongShaderMaterialTexture.SetUniform3f("uPointLightThird.Position", glm::vec3(-35.0f, 10.0f, -50.1f));
    PhongShaderMaterialTexture.SetUniform3f("uPointLightThird.Direction", glm::vec3(0.0f, -5.0f, 1.0f));
    PhongShaderMaterialTexture.SetUniform3f("uPointLightThird.Ka", glm::vec3(1.00000f, 1.00000f, 0.90196f));
    PhongShaderMaterialTexture.SetUniform3f("uPointLightThird.Kd", glm::vec3(1.00000f, 1.00000f, 0.90196f));
    PhongShaderMaterialTexture.SetUniform3f("uPointLightThird.Ks", glm::vec3(1.0f));
    PhongShaderMaterialTexture.SetUniform1f("uPointLightThird.Kc", 1.0f);
    PhongShaderMaterialTexture.SetUniform1f("uPointLightThird.Kl", 0.092f);
    PhongShaderMaterialTexture.SetUniform1f("uPointLightThird.Kq", 0.032f);

    PhongShaderMaterialTexture.SetUniform3f("uSpotlight.Position", glm::vec3(0.0f, 3.5f, -20.0f));
    PhongShaderMaterialTexture.SetUniform3f("uSpotlight.Direction", glm::vec3(0.0f, -0.1f, 0.0f));
    PhongShaderMaterialTexture.SetUniform3f("uSpotlight.Ka", glm::vec3(1.00000f, 1.00000f, 0.80000f));
    PhongShaderMaterialTexture.SetUniform3f("uSpotlight.Kd", glm::vec3(1.00000f, 1.00000f, 0.80000f));
    PhongShaderMaterialTexture.SetUniform3f("uSpotlight.Ks", glm::vec3(1.0f));
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight.Kc", 1.0f);
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight.Kl", 0.092f);
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight.Kq", 0.032f);
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight.InnerCutOff", glm::cos(glm::radians(30.5f)));
    PhongShaderMaterialTexture.SetUniform1f("uSpotlight.OuterCutOff", glm::cos(glm::radians(30.5f)));
    PhongShaderMaterialTexture.SetUniform1i("uMaterial.Kd", 0);
    PhongShaderMaterialTexture.SetUniform1i("uMaterial.Ks", 1);
    PhongShaderMaterialTexture.SetUniform1f("uMaterial.Shininess", 128.0f);
    glUseProgram(0);

    glm::mat4 Projection = glm::perspective(45.0f, WindowWidth / (float)WindowHeight, 0.1f, 100.0f);
    glm::mat4 View = glm::lookAt(FPSCamera.GetPosition(), FPSCamera.GetTarget(), FPSCamera.GetUp());
    glm::mat4 ModelMatrix(1.0f);

    float TargetFrameTime = 1.0f / TargetFPS;
    float StartTime = glfwGetTime();
    float EndTime = glfwGetTime();
    glClearColor(0.1f, 0.1f, 0.2f, 0.0f);

    Shader* CurrentShader = &PhongShaderMaterialTexture;
    int pulsCount = 0;
    double r = 0.0;
    while (!glfwWindowShouldClose(Window)) {
        glfwPollEvents();
        HandleInput(&State, *CurrentShader);

        CurrentShader = &PhongShaderMaterialTexture;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Projection = glm::perspective(45.0f, WindowWidth / (float)WindowHeight, 0.1f, 100.0f);
        View = glm::lookAt(FPSCamera.GetPosition(), FPSCamera.GetTarget(), FPSCamera.GetUp());
        StartTime = glfwGetTime();
        glUseProgram(CurrentShader->GetId());
        CurrentShader->SetProjection(Projection);
        CurrentShader->SetView(View);
        CurrentShader->SetUniform3f("uViewPos", FPSCamera.GetPosition());
        r = ((double)rand() / (RAND_MAX)) * 100;
        if (r >= 97.90) {
            CurrentShader->SetUniform3f("uPointLight.Ka", glm::vec3(0.0, 0.0, 0.0));
            CurrentShader->SetUniform3f("uPointLight.Kd", glm::vec3(0.0, 0.0, 0.0));
            CurrentShader->SetUniform3f("uPointLightSecond.Ka", glm::vec3(0.0, 0.0, 0.0));
            CurrentShader->SetUniform3f("uPointLightSecond.Kd", glm::vec3(0.0, 0.0, 0.0));
            CurrentShader->SetUniform3f("uPointLightThird.Ka", glm::vec3(0.0, 0.0, 0.0));
            CurrentShader->SetUniform3f("uPointLightThird.Kd", glm::vec3(0.0, 0.0, 0.0));
        }
        else
        {
            CurrentShader->SetUniform3f("uPointLight.Ka", glm::vec3(0.347059, 0.347059, 0.347059));
            CurrentShader->SetUniform3f("uPointLight.Kd", glm::vec3(0.347059, 0.347059, 0.347059));
            CurrentShader->SetUniform3f("uPointLightSecond.Ka", glm::vec3(0.347059, 0.347059, 0.347059));
            CurrentShader->SetUniform3f("uPointLightSecond.Kd", glm::vec3(0.347059, 0.347059, 0.347059));
            CurrentShader->SetUniform3f("uPointLightThird.Ka", glm::vec3(0.347059, 0.347059, 0.347059));
            CurrentShader->SetUniform3f("uPointLightThird.Kd", glm::vec3(0.347059, 0.347059, 0.347059));
        }

        glClearColor(0.1f, 0.1f, 0.2f, 0.0f);

        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.5, 2.7 + ((double)rand() / (RAND_MAX)) / 8, +0.5));
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(x, y, z));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(7.0f, 7.0f, 7.0f));
        CurrentShader->SetModel(ModelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, CarpetTexture);
        Rug.Render();
     
        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-38.0f, -1.0f, -38.0f));
        CurrentShader->SetModel(ModelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ChairTexture);
        Egy.Render();

        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(13.0f, -1.0f, 42.0f));
        CurrentShader->SetModel(ModelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ChairTexture);
        Egy.Render();

        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(53.0f, -1.0f, 10.0f));
        CurrentShader->SetModel(ModelMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ChairTexture);
        Egy.Render();

        DrawFloor(CubeVAO, *CurrentShader, FloorDiffuseTexture);
        DrawMoon(CubeVAO, *CurrentShader, MoonDiffuseTexture);
        DrawPyramid(PyramidVAO, *CurrentShader, glm::vec3(65.0, -5.0, 0.0), glm::vec3(25.0, 25.0, 25.0), PyramidDiffuseTexture);
        DrawPyramid(PyramidVAO, *CurrentShader, glm::vec3(-35.0, -5.0, -50.0), glm::vec3(25.0, 25.0, 25.0), PyramidDiffuseTexture);
        DrawPyramid(PyramidVAO, *CurrentShader, glm::vec3(5.0, -5.0, 30.0), glm::vec3(25.0, 25.0, 25.0), PyramidDiffuseTexture);
        DrawStones(CubeVAO, *CurrentShader, StoneSpecularTexture);

        glUseProgram(ColorShader.GetId());

        ColorShader.SetProjection(Projection);
        ColorShader.SetView(View);
        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, 1.0f, -2.0f));
        ColorShader.SetModel(ModelMatrix);

        // Draw point light for first pyramid
        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(65.1f, 10.0f, 0.0f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1.0f));
        ColorShader.SetModel(ModelMatrix);
        glBindVertexArray(PyramidVAO);
        if (r >= 97.90) {
            ColorShader.SetUniform3f("uColor", glm::vec3(0.00000, 0.00000, 0.00000));
        }
        else
            ColorShader.SetUniform3f("uColor", glm::vec3(1.00000, 1.00000, 0.80000));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        //Draw point light for second pyramid
        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(5.0f, 10.0f, 30.0f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1.0f));
        ColorShader.SetModel(ModelMatrix);
        glBindVertexArray(PyramidVAO);
        if (r >= 97.90) {
            ColorShader.SetUniform3f("uColor", glm::vec3(0.00000, 0.00000, 0.00000));
        }
        else
            ColorShader.SetUniform3f("uColor", glm::vec3(1.00000, 1.00000, 0.80000));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        //Draw point light for third pyramid
        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-35.0f, 10.0f, -50.1f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1.0f));
        ColorShader.SetModel(ModelMatrix);
        glBindVertexArray(PyramidVAO);
        if (r >= 97.90) {
            ColorShader.SetUniform3f("uColor", glm::vec3(0.00000, 0.00000, 0.00000));
        }
        else
            ColorShader.SetUniform3f("uColor", glm::vec3(1.00000, 1.00000, 0.80000));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Draw spotlight
        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-5.0f, 25.5f, -30.0f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1.0f));
        ColorShader.SetModel(ModelMatrix);
        ColorShader.SetUniform3f("uColor", glm::vec3(1.00000, 1.00000, 0.80000));

        glBindVertexArray(CubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Draw ambientlight
        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-5.0, 27.0f, -30.0));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.5f));
        ColorShader.SetModel(ModelMatrix);
        ColorShader.SetUniform3f("uColor", glm::vec3(1.0f, 1.0f, 0.8f));
        glBindVertexArray(CubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glBindVertexArray(0);
        glUseProgram(0);
        glfwSwapBuffers(Window);

        pulsCount++;
        EndTime = glfwGetTime();
        float WorkTime = EndTime - StartTime;
        if (WorkTime < TargetFrameTime) {
            int DeltaMS = (int)((TargetFrameTime - WorkTime) * 1000.0f);
            std::this_thread::sleep_for(std::chrono::milliseconds(DeltaMS));
            EndTime = glfwGetTime();
        }
        State.mDT = EndTime - StartTime;
    }

    glfwTerminate();
    return 0;
}
