#ifndef SPHERE_H
#define SPHERE_H

#define _USE_MATH_DEFINES
#define GLM_ENABLE_EXPERIMENTAL

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GL/glut.h>

#include <vector>
#include <stdio.h>
#include <iostream>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp> // after <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <math.h>

class Sphere {

public:

      Sphere();
      ~Sphere();

      void createBuffer();
      void createSphere(double R, double H, double K, double Z);
      void displaySphere(double R, GLuint texture);

      std::vector<GLfloat> getPosVertex() { return posVertex; };
      unsigned int sizeVertex() { return posVertex.size(); };

      std::vector<GLfloat> getNormals() { return normals; };
      unsigned int sizeNormals() { return normals.size(); };

      std::vector<GLfloat> getTexCoords() { return texCoords; };
      unsigned int sizeTexCoords() { return texCoords.size(); };

      std::vector<unsigned int> getIndexes() {return indices; };
      unsigned int sizeIndexes() { return indices.size(); };

private:

      /* Statement Exercise ... */
      std::vector<std::vector<int>> grid;

      // Buffers
      std::vector<GLfloat> posVertex;
      std::vector<GLfloat> normals;
      std::vector<GLfloat> texCoords;
      std::vector<GLuint> indices;

};
#endif
