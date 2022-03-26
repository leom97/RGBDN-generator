#define STB_IMAGE_IMPLEMENTATION	// DO THIS ONLY IN ONE .cpp FILE

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shaderClass.h>
#include <camera.h>
#include <model.h>

#include "conf.h"

//#include "filesystem.h"
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window, Model& ourModel);

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = conf::SCR_WIDTH / 2.0f;
float lastY = conf::SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(conf::SCR_WIDTH, conf::SCR_HEIGHT, "Window", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // Use the reverse z trick
    if(conf::depth_mode == "reverse")    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
 
    // build and compile shaders
    // -------------------------
    Shader normalShader("../Data/shaders/standard2_todo_merge.vert", "../Data/shaders/standard2_todo_merge.frag");
    Shader shadowShader("../Data/shaders/shadowShader.vert", "../Data/shaders/shadowShader.frag");
   
    // load models
    // -----------
    Model ourModel("C:/Code/University/TUM/learnOpenGL/data/models/backpack/backpack.obj");

    // lights and shadows
    // ------
    std::cout << "Scene is assumed in the world ball of radius " << conf::scene_size << std::endl;
    glm::vec3 lightCol(1.0f, 1.0f, 1.0f);
    normalShader.use();
    normalShader.setVec3("light.color", lightCol);
    normalShader.setInt("shadowMap", 2);

    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window, ourModel);

        // render
        // ------
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);        

        // For the light
        glm::vec3 lightDir(-1.0f, 0.0f, -1.0f);
        glm::vec3 lightPos; // this is needed, even though the lighting is directional. This is because we render the scene from the light's point of view, for the sake of shadowing
        
        lightPos = -lightDir / sqrtf(pow(lightDir.x,2)+ pow(lightDir.y, 2)+ pow(lightDir.z, 2));    // i.e. backproject on the unit sphere
        lightPos = (conf::scene_size + conf::light_nearPlane) * lightPos;   // whatever is outside the sphere of radius conf::scene_size is cut

        glm::mat4 lightProjection, lightView, lightSpaceMatrix;
        lightProjection = glm::ortho(-conf::scene_size, conf::scene_size, -conf::scene_size, conf::scene_size, conf::light_nearPlane, conf::light_farPlane);
        lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0)); // look from light position to origin, up is y
        lightSpaceMatrix = lightProjection * lightView;

        shadowShader.use();
        shadowShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        // For the rest
        normalShader.use();
        normalShader.setVec3("light.wDir", lightDir);
        normalShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)conf::SCR_WIDTH / (float)conf::SCR_HEIGHT, conf::near, conf::far);
        if (conf::depth_mode == "reverse") {
            glm::mat4 maybe_Id = glm::mat4(1.0f);
            maybe_Id[2][2] = -0.5f;     
            maybe_Id[3][2] = 0.5f;  // NB: 3rd column index, 2nd row index!
            projection = maybe_Id * projection;
        }
        glm::mat4 view = camera.GetViewMatrix();
        normalShader.setMat4("projection", projection);
        normalShader.setMat4("view", view);
        normalShader.setFloat("Near", conf::near);
        normalShader.setFloat("Far", conf::far);

        // camera positions for not just lambertian colors
        normalShader.setVec3("camPos", camera.Position);

        // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
        normalShader.setMat4("model", model);

        ourModel.Draw(normalShader, shadowShader);


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window, Model & ourModel)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        ourModel.save_depth = true;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}