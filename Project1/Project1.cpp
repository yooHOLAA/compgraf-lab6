#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "Model.h"

const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 600;

// Параметры камеры
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// Углы Эйлера для мыши
float yaw = -90.0f;
float pitch = 0.0f;
bool firstMouse = true;
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
float cameraSpeed = 0.005f;
float mouseSensitivity = 0.05f;

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Lab6 - Phong Lighting", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    glEnable(GL_DEPTH_TEST);

    // Загрузка модели
    Model ourModel("models/cnc_machine.obj");
    Shader ourShader("vertex.glsl", "fragment.glsl");

    // Матрица проекции
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);

    // Матрица модели с масштабированием
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));

    while (!glfwWindowShouldClose(window)) {
        // Управление камерой WASD (горизонтальное движение)
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPos += cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPos -= cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

        // Управление камерой по вертикали (Space - вверх, Ctrl - вниз)
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            cameraPos += cameraSpeed * cameraUp;
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
            cameraPos -= cameraSpeed * cameraUp;

        // Видовая матрица
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        // Очистка экрана
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Использование шейдера
        ourShader.use();

        // Передача матриц
        ourShader.setMat4("projection", glm::value_ptr(projection));
        ourShader.setMat4("view", glm::value_ptr(view));
        ourShader.setMat4("model", glm::value_ptr(model));

        // Позиция камеры (для расчета specular)
        ourShader.setVec3("viewPos", cameraPos.x, cameraPos.y, cameraPos.z);

        // Параметры материала (оранжево-золотистый металлик)
        ourShader.setVec3("material.ambient", 0.2f, 0.1f, 0.0f);
        ourShader.setVec3("material.diffuse", 0.8f, 0.6f, 0.2f);
        ourShader.setVec3("material.specular", 0.9f, 0.9f, 0.9f);
        ourShader.setFloat("material.shininess", 32.0f);

        // Параметры источника света (белый, позиция сверху-спереди)
        ourShader.setVec3("light.position", 0.0f, 2.0f, 3.0f);
        ourShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
        ourShader.setVec3("light.diffuse", 0.8f, 0.8f, 0.8f);
        ourShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

        // Отрисовка модели
        ourModel.Draw(ourShader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}