#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "gl_util.h"

void window_resize_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    // TODO
}

int init_gl(GLFWwindow*& window_out) {
    // initialize glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // create window
    window_out = glfwCreateWindow(1280, 720, "test window", NULL, NULL);
    if (window_out == nullptr) {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window_out);

    // initialize glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        glfwTerminate();
        return 2;
    }

    // set up callbacks, viewport, etc
    glViewport(0, 0, 1280, 720); // TODO: not hardcode this
    glfwSetFramebufferSizeCallback(window_out, window_resize_callback);
    return 0;
}