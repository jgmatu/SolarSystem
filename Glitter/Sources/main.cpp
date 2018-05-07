
#define GLM_ENABLE_EXPERIMENTAL
#define _USE_MATH_DEFINES
#define STB_IMAGE_IMPLEMENTATION

// System Headers.
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <fstream>
#include <streambuf>
#include <iostream>
#include "ShaderProgram.hpp"
#include <exception>
#include <math.h> /* sin */
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp> // after <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <stb_image.h>
#include "Sphere.hpp"
#include "BOX.h"

GLFWwindow* window;

unsigned int VAO, VBO, VBO2, VBO3, EBO, skyboxVAO;
unsigned int textureSun, textureEarth, textureMoon, cubemapTexture;

ShaderProgram *shaderProgramSun;
ShaderProgram *shaderProgramEarth;
ShaderProgram *shaderProgramMoon;
ShaderProgram *shaderProgramSkyBox;

Sphere *sphere;

glm::mat4 view(1.0); // Neccesary global to handler the camera events...
                     // This variable is initalize on a first possition
                     // before enter on render loop in loop function...

std::string
getDataFile(const std::string& filename)
{
   std::ifstream file(filename);

   file.open("r");
   std::string data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
   file.close();
   return data;
}

std::string
getVertexShaderSource(const std::string& filename)
{
   return getDataFile(filename);
}

std::string
getFragmentShaderSource(const std::string& filename)
{
   return getDataFile(filename);
}

static void
key_callback(GLFWwindow* _window, int key, int scancode, int action, int mods)
{
      std::cout << scancode << " " << mods << std::endl;

      if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(_window, GLFW_TRUE);
      }

      if (key == GLFW_KEY_Q) {
            view = glm::translate(view,  glm::vec3(0.0, 0.0, 0.1f));
      }

      if (key == GLFW_KEY_E) {
            view = glm::translate(view,  glm::vec3(0.0, 0.0, -0.1f));
      }

      if (key == GLFW_KEY_A) {
            view = glm::translate(view, glm::vec3(-0.1f, 0.0, 0.0));
      }

      if (key == GLFW_KEY_D) {
            view = glm::translate(view, glm::vec3(0.1f, 0.0, 0.0));
      }

      if (key == GLFW_KEY_X) {
            view = glm::translate(view, glm::vec3(0.0, -0.1f, 0.0));
      }

      if (key == GLFW_KEY_W) {
            view = glm::translate(view, glm::vec3(0.0, 0.1f, 0.0));
      }

      if (key == GLFW_KEY_UP) {
            fprintf(stderr, "%s\n", "KEY UP");
            view *= glm::rotate(0.1f, glm::vec3(0.0, 1.0f, 0.0));
      }
}

unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void load_texture(const char * filename, unsigned int* textureID) {
      int width, heigth, channels;
      unsigned char* pixels = stbi_load(filename, &width, &heigth, &channels, 0);

      glGenTextures(1, textureID);
      glBindTexture(GL_TEXTURE_2D, *textureID); // all upcoming GL_TEXTURE_2D operations now have

      // set the texture wrapping parameters.
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

      if (channels == 4) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, heigth, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
      } else if (channels == 3) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, heigth, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
      }
      free(pixels);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glGenerateMipmap(GL_TEXTURE_2D);
}

void
setBuffersSphere()
{

      glGenVertexArrays(1, &VAO);
      glGenBuffers(1, &VBO);  // Create VertexArrayObject.
      glGenBuffers(1, &VBO2); // Create VertexArrayObject.
      glGenBuffers(1, &VBO3); // Create VertexArrayObject.
      glGenBuffers(1, &EBO);  // Create VertexArrayObject.

      glBindVertexArray(VAO);

      // Position attribute...
      glBindBuffer(GL_ARRAY_BUFFER, VBO); // Bind Vertex VAO...
      glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * sphere->sizeVertex(), sphere->getPosVertex().data(), GL_STATIC_DRAW); // Assign buffer to VBO
      glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
      glEnableVertexAttribArray(0);

      // Normal attribute...
      glBindBuffer(GL_ARRAY_BUFFER, VBO2);
      glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * sphere->sizeNormals(), sphere->getNormals().data(), GL_STATIC_DRAW);
      glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, 0);
      glEnableVertexAttribArray(1);

      // Text Coord attribute...
      glBindBuffer(GL_ARRAY_BUFFER, VBO3);
      glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * sphere->sizeTexCoords(), sphere->getTexCoords().data(), GL_STATIC_DRAW);
      glVertexAttribPointer(2, 2, GL_FLOAT, false, 0, 0);
      glEnableVertexAttribArray(2);

      // Buffer indexes...
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere->sizeIndexes() * sizeof(GLuint), sphere->getIndexes().data(), GL_STATIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind the VBO.
      glBindVertexArray(0); // Unbind the VAO.
}

void
setBuffersSkyBox() {

      glGenVertexArrays(1, &skyboxVAO);
      glGenBuffers(1, &VBO);  // Create VertexArrayObject.
      glGenBuffers(1, &VBO2); // Create VertexArrayObject.
      glGenBuffers(1, &VBO3); // Create VertexArrayObject.
      glGenBuffers(1, &EBO);  // Create VertexArrayObject.

      glBindVertexArray(skyboxVAO);

      // Position attribute...
      glBindBuffer(GL_ARRAY_BUFFER, VBO); // Bind Vertex VAO...
      glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(cubeVertexPos), cubeVertexPos, GL_STATIC_DRAW); // Assign buffer to VBO
      glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
      glEnableVertexAttribArray(0);

      // Normal attribute...
      glBindBuffer(GL_ARRAY_BUFFER, VBO2);
      glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(cubeVertexNormal), cubeVertexNormal, GL_STATIC_DRAW);
      glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, 0);
      glEnableVertexAttribArray(1);

      // Text Coord attribute...
      glBindBuffer(GL_ARRAY_BUFFER, VBO3);
      glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(cubeVertexTexCoord), cubeVertexTexCoord, GL_STATIC_DRAW);
      glVertexAttribPointer(2, 2, GL_FLOAT, false, 0, 0);
      glEnableVertexAttribArray(2);

      // Buffer indexes...
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * cubeNTriangleIndex * sizeof(GLuint), cubeTriangleIndex, GL_STATIC_DRAW);

      glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind the VBO.
      glBindVertexArray(0); // Unbind the VAO.
}


void
init()
{
      // Initialize GLFW. Most GLFW functions will not work before doing this.
      if (!glfwInit()) {
            throw;
      }

      // Configure GLFW.
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);

      // Create the window.
      window = glfwCreateWindow(800, 600, "Solar System!", nullptr, nullptr);
      if (window == nullptr) {
            std::cerr << "Failed to create the GLFW window" << std::endl;
            throw;
      }

      // Setup a key callback. It will be called every time a key is pressed, repeated or released.
      glfwSetKeyCallback(window, key_callback);

      // Make the OpenGL context current.
      glfwMakeContextCurrent(window);

      // Enable v-sync.
      glfwSwapInterval(1);

      // Make the window visible.
      glfwShowWindow(window);
      gladLoadGL();

      {
            shaderProgramSun = new ShaderProgram();

            shaderProgramSun->createVertexShader(getVertexShaderSource("./Glitter/Sources/sun_vertex.glsl"));
            shaderProgramSun->createFragmentShader(getFragmentShaderSource("./Glitter/Sources/sun_fragments.glsl"));
            shaderProgramSun->link();

            shaderProgramSun->createUniform("projection");
            shaderProgramSun->createUniform("view");
            shaderProgramSun->createUniform("model");
            shaderProgramSun->createUniform("diffuseTexture");
      }

      {
            shaderProgramEarth = new ShaderProgram();

            shaderProgramEarth->createVertexShader(getVertexShaderSource("./Glitter/Sources/earth_vertex.glsl"));
            shaderProgramEarth->createFragmentShader(getFragmentShaderSource("./Glitter/Sources/earth_fragments.glsl"));
            shaderProgramEarth->link();

            shaderProgramEarth->createUniform("projection");
            shaderProgramEarth->createUniform("view");
            shaderProgramEarth->createUniform("model");
            shaderProgramEarth->createUniform("diffuseTexture");
      }

      {
            shaderProgramMoon = new ShaderProgram();

            shaderProgramMoon->createVertexShader(getVertexShaderSource("./Glitter/Sources/moon_vertex.glsl"));
            shaderProgramMoon->createFragmentShader(getFragmentShaderSource("./Glitter/Sources/moon_fragments.glsl"));
            shaderProgramMoon->link();

            shaderProgramMoon->createUniform("projection");
            shaderProgramMoon->createUniform("view");
            shaderProgramMoon->createUniform("model");
            shaderProgramMoon->createUniform("diffuseTexture");
      }

      {
            shaderProgramSkyBox = new ShaderProgram();

            shaderProgramSkyBox->createVertexShader(getVertexShaderSource("./Glitter/Sources/skybox_vertex.glsl"));
            shaderProgramSkyBox->createFragmentShader(getFragmentShaderSource("./Glitter/Sources/skybox_fragments.glsl"));
            shaderProgramSkyBox->link();

            shaderProgramSkyBox->createUniform("projection");
            shaderProgramSkyBox->createUniform("view");
            shaderProgramSkyBox->createUniform("model");
//            shaderProgramSkyBox->createUniform("skybox");
      }

      {
            load_texture("../resources/sun.png", &textureSun);
            load_texture("../resources/earth_diffuse.jpg", &textureEarth);
            load_texture("../resources/moon.png", &textureMoon);

            sphere = new Sphere();
            sphere->createBuffer();
            setBuffersSphere();
      }

      {
            std::vector<std::string> faces
            {
                  "./ame_nebula/purplenebula_rt.tga",
                  "./ame_nebula/purplenebula_lf.tga",
                  "./ame_nebula/purplenebula_up.tga",
                  "./ame_nebula/purplenebula_dn.tga",
                  "./ame_nebula/purplenebula_ft.tga",
                  "./ame_nebula/purplenebula_bk.tga"
            };
            cubemapTexture = loadCubemap(faces);
            setBuffersSkyBox();
      }
}

void
loop() {

      float aspectRatio = 4.0f / 3.0f;
      glm::mat4 projection = glm::perspective(glm::radians(60.0f), aspectRatio, 0.001f, 1000.0f);

      view = glm::translate(view, glm::vec3(0.0, 0.0, -5.0));

      glm::mat4 sun(1.0);
      sun = glm::translate(sun, glm::vec3(0.0, 0.0, 0.0));

      glm::mat4 earth(1.0);
      earth = glm::translate(earth, glm::vec3(0.0, 0.0, 0.0));

      glm::mat4 moon(1.0);
      moon = glm::translate(moon, glm::vec3(0.0, 0.0, 0.0));

      glm::mat4 sky(1.0);
      sky = glm::translate(sky, glm::vec3(0.0, 0.0, 0.0));
      sky *= glm::scale(sky, glm::vec3(20.0f, 20.0f, 20.0f));

      glEnable(GL_DEPTH_TEST);
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      while (!glfwWindowShouldClose(window)) {
            float angle = std::fmod(glfwGetTime(), (2.0f * M_PI));

            // Poll for window events. The key callback above will only be
            // invoked during this call.
            glfwPollEvents();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Render...
            glBindVertexArray(VAO); // Activa la geometría que se va a pintar.

            // Sun...
            {
                  shaderProgramSun->bind();

                  // Projection and view matrix...
                  shaderProgramSun->setUniform("projection", projection);
                  shaderProgramSun->setUniform("view", view);
                  shaderProgramSun->setUniform("model", sun);

                  glActiveTexture(GL_TEXTURE0); // Activate first texture unit.
                  shaderProgramSun->setUniform("diffuseTexture", 0); // El índice es el mismo que en glActiveTexture().

                  glBindTexture(GL_TEXTURE_2D, textureSun);
                  glDrawElements(GL_TRIANGLES, sphere->sizeIndexes() * sizeof(GLuint), GL_UNSIGNED_INT, 0);
            }

            // Earth...
            {
                  earth  = glm::scale(sun, glm::vec3(0.5f, 0.5f, 0.5f));
                  earth *= glm::rotate(angle, glm::vec3(0.0f, 1.0, 0.0f));
                  earth *= glm::translate(sun, glm::vec3(4.0, 0.0, 0.0));
                  earth *= glm::rotate(angle, glm::vec3(0.3f, 1.0f, 0.0f));

                  shaderProgramEarth->bind();

                  shaderProgramEarth->setUniform("projection", projection);
                  shaderProgramEarth->setUniform("view", view);
                  shaderProgramEarth->setUniform("model", earth);

                  glActiveTexture(GL_TEXTURE0); // Activate first texture unit.
                  shaderProgramEarth->setUniform("diffuseTexture", 0); // El índice es el mismo que en glActiveTexture().

                  glBindTexture(GL_TEXTURE_2D, textureEarth);
                  glDrawElements(GL_TRIANGLES, sphere->sizeIndexes() * sizeof(GLuint), GL_UNSIGNED_INT, 0);
            }

            // Moon...
            {
                  moon = glm::scale(earth, glm::vec3(0.75f, 0.75f, 0.75f));
                  moon *= glm::translate(moon, glm::vec3(1.0, 0.0, 0.0));

                  shaderProgramMoon->bind();

                  // Projection and view matrix...
                  shaderProgramMoon->setUniform("projection", projection);
                  shaderProgramMoon->setUniform("view", view);
                  shaderProgramMoon->setUniform("model", moon);

                  glActiveTexture(GL_TEXTURE0); // Activate first texture unit.
                  shaderProgramMoon->setUniform("diffuseTexture", 0); // El índice es el mismo que en glActiveTexture().

                  glBindTexture(GL_TEXTURE_2D, textureMoon);
                  glDrawElements(GL_TRIANGLES, sphere->sizeIndexes() * sizeof(GLuint), GL_UNSIGNED_INT, 0);
            }

            {
                  shaderProgramSkyBox->bind();

                  // Projection and view matrix...
                  shaderProgramSkyBox->setUniform("projection", projection);
                  shaderProgramSkyBox->setUniform("view", view);
                  shaderProgramSkyBox->setUniform("model", sky);

                  glActiveTexture(GL_TEXTURE0); // Activate first texture unit.
                  shaderProgramSkyBox->setUniform("skybox", 0); // El índice es el mismo que en glActiveTexture().

                  // ... set view and projection matrix
                  glBindVertexArray(skyboxVAO);
                  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
                  glDepthMask(GL_TRUE);

                  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
                  glDrawElements(GL_TRIANGLES, 3 * cubeNTriangleIndex * sizeof(GLuint), GL_UNSIGNED_INT, 0);
            }
            glfwSwapBuffers(window); // swap the color buffers.
      }
}

void
run()
{
      try {
            init();
            loop();
      } catch (std::exception& e) {
            std::cout << e.what() << std::endl;
      }
      glfwTerminate();
}

int
main() {
      run();
      return 0;
}
