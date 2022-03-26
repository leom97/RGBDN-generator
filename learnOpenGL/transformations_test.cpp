#define STB_IMAGE_IMPLEMENTATION	// DO THIS ONLY IN ONE .cpp FILE

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shaderClass.h"
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
float incrUpDown(GLFWwindow* window);
void printTrans(glm::mat4 T);
glm::mat4 getProjectionMatrix(float l, float r, float b, float t, float n, float f);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// Let's create the shader programmes
	// ----------------------------------
	Shader myShader("../Data/shaders/shader_trans_ex.vert", "../Data/shaders/shader_trans_ex.frag");

	// Experiments with transformations

	float delta{ 2.0f };	// a 2 meters deep parallelepiped
	float H{ SCR_HEIGHT };
	float W{ SCR_WIDTH };
	float F{ .05f };
	float fx{ F / .002f };	// sensor 5 cm from camera center, pixel width of 2 mm
	float fy{ F / .001f };	// sensor 5 cm from camera center, pixel height of 1 mm
	float cx{ 604.0f};
	float cy{ 305.0f};
	float dbar = 1.0f;	// parallelepiped 1 meter in front of the camera
	float alpha{dbar * H / fy};
	float lambda{ dbar*W/fx };

	// For the model matrix
	glm::mat4 TRz = glm::mat4(1.0f); TRz = glm::rotate(TRz, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 TRy = glm::mat4(1.0f); TRy = glm::rotate(TRy, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 Tfx = glm::mat4(1.0f); Tfx = glm::scale(Tfx, glm::vec3(-1.0f, 1.0f, 1.0f));
	glm::mat4 Tt = glm::mat4(1.0f);	Tt = glm::translate(Tt, glm::vec3( 0.0f, -lambda, -alpha));
	glm::mat4 Ts = glm::mat4(1.0f);	Ts = glm::scale(Ts, glm::vec3(delta, lambda, alpha));	// slight modification, it was lambda/2	
	glm::mat4 model = TRz * TRy * Tfx * Tt * Ts;
	
	// For the view matrix
	glm::mat4 view = glm::mat4(1.0f); view = glm::translate(view, glm::vec3(-dbar*cx/fx, -dbar*cy/fy, -dbar));

	// For the projection matrix
	float l{-F*cx/fx};
	float r{F*(W-cx)/fx};
	float b{-F * cy/fy};
	float t{ F * (H - cy) / fy };
	float n{ F };	// note, getProjectionMatrix wants a positive(!!) n value
	float f{dbar+delta};
	glm::mat4 projection = getProjectionMatrix(l,r,b,t,n,f);

	// All together 
	glm::mat4 vShaderT = projection * view * model;
	myShader.use();
	myShader.setTransformation("transformation", vShaderT);

	// Debug
	//glm::vec4 urc(1.0f, 1.0f, 1.0f, 1.0f);
	//urc = model * urc;
	//std::cout << urc.x << ", " << urc.y << ", " << urc.z << std::endl;
	//std::cout << "Lambda: "<< lambda << ", alpha:  " << alpha  << ", delta: " << delta << std::endl;

	// Texture creation 
	// ----------------
	stbi_set_flip_vertically_on_load(true);

	unsigned int textureId{};
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);

	// What if we go out of bounds? How to magnify/downsample?
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Load the images
	int txWidth{}, txHeight{}, txChannels{};
	unsigned char* data = stbi_load("../data/textures/anna.JPG", &txWidth, &txHeight, &txChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, txWidth, txHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);	// push the texture into the texture object which is currently bound
		glGenerateMipmap(GL_TEXTURE_2D);	// automatically generate the mipmap for us
	}
	else std::cout << "Cannot load texture." << std::endl;
	stbi_image_free(data);	// free stuff

	// Another texture
	unsigned int textureId2{};
	glGenTextures(1, &textureId2);
	glBindTexture(GL_TEXTURE_2D, textureId2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	data = stbi_load("../data/textures/awesomeface.png", &txWidth, &txHeight, &txChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, txWidth, txHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);	// push the texture into the texture object which is currently bound
		glGenerateMipmap(GL_TEXTURE_2D);	// automatically generate the mipmap for us
	}
	else std::cout << "Cannot load texture." << std::endl;
	stbi_image_free(data);	// free stuff

	myShader.use();
	myShader.setInt("ourTexture", 0);
	myShader.setInt("ourTexture2", 1);

	// Vertex data to buffer
	// --------------
	
	// In model coordinates
	float vertices[] = {
		// positions          // colors           
		 0.0f,  0.0f, 0.0f,   0.0f, 1.0f, 0.0f,
		 0.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,
		 0.0f,  1.0f, 1.0f,   0.0f, 0.0f, 0.0f,
		 0.0f,  0.0f, 1.0f,   1.0f, 1.0f, 1.0f,
		 1.0f,  0.0f, 0.0f,   0.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,   0.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 1.0f,   0.0f, 0.0f, 0.0f,
		 1.0f,  0.0f, 1.0f,   0.0f, 0.0f, 0.0f,
	};

	unsigned int indices[] = {
		0, 1, 2,	// Front
		2, 3, 0,
		4, 5, 6,	// Back
		6, 7, 4,
		0, 3, 4,	// Right
		3, 7, 4,
		1, 2, 5,	// Left
		2, 6, 5,
		1, 0, 5,	// Up 
		0, 4, 5,
		2, 3, 6,	// Down
		3, 7, 6
	};

	// Create a generic buffer object
	unsigned int VBOId{};
	glGenBuffers(1, &VBOId);
	unsigned int EBOId{};
	glGenBuffers(1, &EBOId);
	unsigned int VAOId{};
	glGenVertexArrays(1, &VAOId);

	glBindVertexArray(VAOId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOId);
	glBindBuffer(GL_ARRAY_BUFFER, VBOId);	// only one buffer at a time can be bound to GL_ARRAY_BUFFER (which is not a buffer, but a target) (or any other type of buffer)
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);	// send, to the buffer bound to GL_ARRAY_BUFFER (i.e. to VBO), vertices (for static draw, see the guide)
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Data interpretation
	// -------------------
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
	//glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6*sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	//glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0); 
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Rendering
	// ---------

	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);	// make a window with coordinates origin at bottom left, and obvious widths and heights
	// Please resize the window dynamically as the user plays around with it
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	float darkness{ 0.0f };

	while (!glfwWindowShouldClose(window))
	{

		processInput(window);
		
		// rendering commands: background
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);	// next time glClear is called, it will use this color
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!

		myShader.use();
		glBindVertexArray(VAOId);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


		// check for events, swap the buffers
		glfwSwapBuffers(window);	// swap front with back buffer, to avoid flickering
		glfwPollEvents();	// get events from user, like mouse clicks and so on
	}

	// Graceful exit: clean everything up
	glfwTerminate();
	return 0;

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

// Return 1 if arrow up, -1 otherwise
float incrUpDown(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		return 1.0f;
	else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		return -1.0f;
	else return 0.0f;
}

void printTrans(glm::mat4 T)
{
	const float* pSource = (const float*)glm::value_ptr(T);
	for (int j{0}; j < 4; ++j)
	{
		for (int i{ 0 }; i < 4; ++i)
		{
			std::cout << pSource[4 * i + j] << "\t\t";	// stored in column major, transpose for visualization
		}
		std::cout << std::endl;
	}
}

glm::mat4 getProjectionMatrix(float l, float r, float b, float t, float n, float f) {
	// NB: l, b might as well be negative, n, f are positive!!

	float matrix[16];
	for (int i{ 0 }; i < 16; ++i) matrix[i] = 0.0f;

	matrix[0] = 2 * n / (r - l);
	matrix[5] = 2 * n / (t - b);
	matrix[8] = (r + l) / (r - l);
	matrix[9] = (t + b) / (t - b);
	matrix[10] = -(f + n) / (f - n);
	matrix[11] = -1;
	matrix[14] = -(2 * f * n) / (f - n);

	glm::mat4 res = glm::make_mat4(matrix);
	return res;
}