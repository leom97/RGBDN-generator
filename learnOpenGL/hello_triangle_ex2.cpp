#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

constexpr unsigned int SCR_W{ 800 };
constexpr unsigned int SCR_H{ 600 };

// Shaders sources
const char* vertexShaderSource =
"#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";
const char* fragmentShaderSource =
"#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\n\0";

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);	// we want to use a certain version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);	// and we want to use the (more difficult) core profile

	GLFWwindow* window = glfwCreateWindow(SCR_W, SCR_H, "LearnOpenGL", NULL, NULL);
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


	// We start here

	// ----------------------- Shaders
	// Create two shader objects, put source code, compile, create program, link
	unsigned int vertexShaderId{ glCreateShader(GL_VERTEX_SHADER) };
	glShaderSource(vertexShaderId, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShaderId);

	unsigned int fragmentShaderId{ glCreateShader(GL_FRAGMENT_SHADER) };
	glShaderSource(fragmentShaderId, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShaderId);

	unsigned int shaderFlow{ glCreateProgram() };
	glAttachShader(shaderFlow, vertexShaderId);
	glAttachShader(shaderFlow, fragmentShaderId);
	glLinkProgram(shaderFlow);

	// ----------------------- Data
	float triangleVertices1[] = {
		-.5f, .0f, .0f,
		.0f, .5f, .0f,
		.5f, .0f, .0f
	};
	float triangleVertices2[] = {
		.5f, .0f, .0f,
		.0f, -.5f, .0f,
		-.5f, .0f, .0f
	};

	// ----------------------- Buffers, vertex array objects
	// Generate the buffers, bind them, pipe data into them, interpret data and save into VAO
	unsigned int vert_buff1{};
	glGenBuffers(1, &vert_buff1);

	unsigned int vert_arr1{};
	glGenVertexArrays(1, &vert_arr1);
	glBindVertexArray(vert_arr1);

	glBindBuffer(GL_ARRAY_BUFFER, vert_buff1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices1), triangleVertices1, GL_STATIC_DRAW);

	// Note, the element buffer object is saved inside the VAO as soon as I call vertexAttribute somethign
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	unsigned int vert_buff2{};
	glGenBuffers(1, &vert_buff2);

	unsigned int vert_arr2{};
	glGenVertexArrays(1, &vert_arr2);
	glBindVertexArray(vert_arr2);

	glBindBuffer(GL_ARRAY_BUFFER, vert_buff2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices2), triangleVertices2, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);	// NB: must be called every time as it is registered in the VAO

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Rendering loop
	glViewport(0, 0, 800, 600);	// make a window with coordinates origin at bottom left, and obvious widths and heights
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	// Please resize the window dynamically as the user plays around with it
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	while (!glfwWindowShouldClose(window))
	{
		// user input
		processInput(window);	// if I press ESC, glfwWindowShouldClose will be set to false

		// rendering commands
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);	// next time glClear is called, it will use this color
		glClear(GL_COLOR_BUFFER_BIT);	// clear the color buffer to the color above (buffer = info to be displayed, e.g. color at every pixel)

		glUseProgram(shaderFlow);
		glBindVertexArray(vert_arr1);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(vert_arr2);
		glDrawArrays(GL_TRIANGLES, 0, 3);

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