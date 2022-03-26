#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);	// we want to use a certain version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);	// and we want to use the (more difficult) core profile

	GLFWwindow * window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << " Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glViewport(0, 0, 800, 600);	// make a window with coordinates origin at bottom left, and obvious widths and heights

	// Please resize the window dynamically as the user plays around with it
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// Rendering loop
	while (!glfwWindowShouldClose(window))
	{
		// user input
		processInput(window);	// if I press ESC, glfwWindowShouldClose will be set to false

		// rendering commands
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);	// next time glClear is called, it will use this color
		glClear(GL_COLOR_BUFFER_BIT);	// clear the color buffer to the color above (buffer = info to be displayed, e.g. color at every pixel)

		// check for events, swap the buffers
		glfwSwapBuffers(window);	// swap front with back buffer, to avoid flickering
		glfwPollEvents();	// get events from user, like mouse clicks and so on
	}
	
	// Graceful exit: clean everything up
	glfwTerminate();
	return 0;

}

// To resize the window as the user resizes the window
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// If the user presses ESC, tell GLFW the window should close
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}