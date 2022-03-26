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
#include <vector>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window, Model& ourModel);
glm::vec3 getLightDir(float t, float f);
glm::mat4 getProjectionMatrix(float l, float r, float b, float t, float n, float f);

// camera
bool lock{ true };  // the camera cannot move
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = conf::SCR_WIDTH / 2.0f;
float lastY = conf::SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// light direction
float theta{ 0.0f }, phi{ 180.0f };   // in degrees
float v_deg{ 2.0f };    // how fast are those angles changing?

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// save snapshot
bool save{ false };
int nSnapshots{ 0 };

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
    Shader normalShader("../Data/shaders/standard.vert", "../Data/shaders/standard.frag");   
    // load models
    // -----------
    Model ourModel("C:/Code/University/TUM/learnOpenGL/data/models/backpack/backpack.obj");

    // lights and shadows
    // ------
    glm::vec3 lightCol(10.0f, 10.0f, 10.0f);
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
        normalShader.use();
        glm::vec3 lightDir = getLightDir(theta, phi);
        normalShader.setVec3("light.wDir", lightDir);

        // Save stuff
        if (save) {
            ourModel.save_to_txt = true;
            ourModel.nSnapshots = nSnapshots;

            {
                // Save light
                std::string light_path{ conf::out_folder + std::to_string(nSnapshots) + "_" + "light_direction.txt" };
                std::ofstream fout(light_path);
                fout << std::setprecision(10);
                std::vector<double> v = { lightDir.x, lightDir.y, lightDir.z };
                std::copy(v.begin(), v.end(),
                    std::ostream_iterator<double>(fout, "\n"));
                std::cout << "Light direction successfully saved to " + light_path << std::endl;
                fout.close();
            }

            {
                // Save camera
                std::string camera_path{ conf::out_folder + std::to_string(nSnapshots) + "_" + "camera_pose.txt" };
                std::ofstream fout(camera_path);
                fout << std::setprecision(10);
                glm::mat4 cp = camera.GetViewMatrix();
                cp = glm::inverse(cp);  // C->W !
                std::vector<double> v;
                for (int i{ 0 }; i < 4; ++i) for (int j{ 0 }; j<4; ++j)    v.push_back(cp[j][i]);
                std::copy(v.begin(), v.end(),
                    std::ostream_iterator<double>(fout, "\n"));
                std::cout << "Camera pose successfully saved to " + camera_path << std::endl;
                fout.close(); 
            }

            save = false;
            ++nSnapshots;
        }

        // Matrices and other geometry
        //glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)conf::SCR_WIDTH / (float)conf::SCR_HEIGHT, conf::near, conf::far);
        glm::mat4 projection = getProjectionMatrix(conf::l, conf::r, conf::b, conf::t, conf::near, conf::far);
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
        normalShader.setVec3("camPos", camera.Position);    // Camera positions for not just lambertian colors

        // Render the model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
        normalShader.setMat4("model", model);
        ourModel.Draw(normalShader);

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
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && !lock)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && !lock)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && !lock)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && !lock)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        save = true;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        theta += v_deg;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        theta -= v_deg;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        phi -= v_deg;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        phi += v_deg;
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
        lock = !lock;

}

glm::vec3 getLightDir(float t, float f)
{
    // t= theta in degrees, f = phi in degrees

    glm::mat4 ry = glm::mat4(1.0f);
    glm::mat4 rx = glm::mat4(1.0f);

    ry = glm::rotate(ry, glm::radians(f), glm::vec3(0.0f, 1.0f, 0.0f));
    rx = glm::rotate(rx, glm::radians(t), glm::vec3(1.0f, 0.0f, 0.0f));

    glm::mat4 r = ry * rx;

    glm::vec3 res = glm::vec3(0.0f);
    res.x = r[2][0];
    res.y = r[2][1];
    res.z = r[2][2];

    return res;

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

    float xoffset = (lock ? 0.0f : xpos - lastX);
    float yoffset = (lock ? 0.0f : lastY - ypos); // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (!lock) camera.ProcessMouseScroll(static_cast<float>(yoffset));
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