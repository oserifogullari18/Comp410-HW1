//
//  Display a rotating cube
//

#include "Angel.h"
#include <cmath>
#include <unistd.h>
typedef vec4 color4;
typedef vec4 point4;

// Ball Bouncing Parameters
float radius = 0.025;
float ballSpeedX = 0.0005;
float ballSpeedY = 0;
float ballSpeedZ = 0;
float gravitiy = 0.00098;
float dt = 0.1;
float rho = 0.85;
vec3 h_current(-1.0, 1.0, 0.0);
vec3 *h_current_ptr = &h_current;

// Draw Mode
GLenum draw_mode = GL_TRIANGLES;
bool isSolid = true;

// Color selection modes
int color_selection = 2;
int *color_selection_ptr = &color_selection;

// Data for Cubes
const int NumTrianglesCube = 12;
// (4 faces)^(NumTimesToSubdivide + 1)
const int NumVerticesCube = 3 * NumTrianglesCube;

point4 pointsCube[NumVerticesCube];
color4 colorsCube[NumVerticesCube];

// Vertices of a unit cube centered at origin, sides aligned with axes
point4 verticesCube[8]{point4(-radius, -radius, radius, 1.0),
                       point4(-radius, radius, radius, 1.0),
                       point4(radius, radius, radius, 1.0),
                       point4(radius, -radius, radius, 1.0),
                       point4(-radius, -radius, -radius, 1.0),
                       point4(-radius, radius, -radius, 1.0),
                       point4(radius, radius, -radius, 1.0),
                       point4(radius, -radius, -radius, 1.0)};

color4 vertexColorsCube[8] = {
    color4(0.0, 0.0, 0.0, 1.0), // black
    color4(1.0, 0.0, 0.0, 1.0), // red
    color4(1.0, 1.0, 0.0, 1.0), // yellow
    color4(0.0, 1.0, 0.0, 1.0), // green
    color4(0.0, 0.0, 1.0, 1.0), // blue
    color4(1.0, 0.0, 1.0, 1.0), // magenta
    color4(1.0, 1.0, 1.0, 1.0), // white
    color4(0.0, 1.0, 1.0, 1.0)  // cyan
};

color4 paintColor = vertexColorsCube[color_selection];

// Model-view and projection matrices uniform location
GLuint ModelView, Projection;

// quad generates two triangles for each face and assigns colors to the vertices
int Index = 0;

void quad(int a, int b, int c, int d) {
  colorsCube[Index] = paintColor;
  pointsCube[Index] = verticesCube[a];
  Index++;
  colorsCube[Index] = paintColor;
  pointsCube[Index] = verticesCube[b];
  Index++;
  colorsCube[Index] = paintColor;
  pointsCube[Index] = verticesCube[c];
  Index++;
  colorsCube[Index] = paintColor;
  pointsCube[Index] = verticesCube[a];
  Index++;
  colorsCube[Index] = paintColor;
  pointsCube[Index] = verticesCube[c];
  Index++;
  colorsCube[Index] = paintColor;
  pointsCube[Index] = verticesCube[d];
  Index++;
}

//----------------------------------------------------------------------------

// generate 12 triangles: 36 vertices and 36 colors

void colorcube() {
  Index = 0;
  quad(1, 0, 3, 2);
  quad(2, 3, 7, 6);
  quad(3, 0, 4, 7);
  quad(6, 5, 1, 2);
  quad(4, 5, 6, 7);
  quad(5, 4, 0, 1);
}

/// Data for Sphere

void init() {
  // Load shaders and use the resulting shader program
  GLuint program = InitShader("src/vshader.glsl", "src/fshader.glsl");
  glUseProgram(program);

  colorcube(); // create the cube in terms of 6 faces each of which is made of
               // two triangles

  // Create a vertex array object
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // Create and initialize a buffer object
  GLuint buffer;
  glGenBuffers(1, &buffer);
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(pointsCube) + sizeof(colorsCube), NULL,
               GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(pointsCube), pointsCube);
  glBufferSubData(GL_ARRAY_BUFFER, sizeof(pointsCube), sizeof(colorsCube),
                  colorsCube);

  // set up vertex arrays
  GLuint vPosition = glGetAttribLocation(program, "vPosition");
  glEnableVertexAttribArray(vPosition);
  glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

  GLuint vColor = glGetAttribLocation(program, "vColor");
  glEnableVertexAttribArray(vColor);
  glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
                        BUFFER_OFFSET(sizeof(pointsCube)));

  // Retrieve transformation uniform variable locations
  ModelView = glGetUniformLocation(program, "ModelView");
  Projection = glGetUniformLocation(program, "Projection");

  // Set projection matrix
  mat4 projection;
  projection = Ortho(-1.0, 1.0, -1.0, 1.0, -1.0,
                     1.0); // Ortho(): user-defined function in mat.h
  glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_COLOR_MATERIAL);

  // specify the color to clear the screen
  glClearColor(0, 0, 0, 1.0);
}

//---------------------------------------------------------------------
//
// display
//

void display(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //  Generate the model-view matrix
  // -1, 1, 0
  const vec3 displacement(h_current_ptr->x, h_current_ptr->y, h_current_ptr->z);
  mat4 model_view =
      (Translate(displacement) *
       Scale(1.0, 1.0, 1.0)); // Scale(), Translate(), RotateX(), RotateY(),
                              // RotateZ(): user-defined functions in mat.h

  h_current_ptr->x = h_current_ptr->x + ballSpeedX;

  h_current_ptr->y =
      h_current_ptr->y + ballSpeedY * dt - 0.5 * gravitiy * pow(dt, 2);

  if (h_current_ptr->y < -1) {
    h_current_ptr->y = -1;
    ballSpeedY = -ballSpeedY * rho;
  } else {
    ballSpeedY = ballSpeedY - gravitiy * dt;
  }
  // ballSpeedY = ballSpeedY + *second_counter_ptr * gravitiy; // t*a
  // initial_points_ptr->y = initial_points_ptr->y + ballSpeedY;

  // if (initial_points_ptr->y + radius <= MinY && ballSpeedX > 0) {
  //   printf("%f %f %f \n", initial_points_ptr->y, MinY, radius);

  // } else {
  //   *second_counter_ptr = 0;
  //   ballSpeedY = ballSpeedY - *second_counter_ptr * gravitiy;
  //   initial_points_ptr->y = initial_points_ptr->y + ballSpeedY;
  // };

  glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);
  glDrawArrays(draw_mode, 0, NumVerticesCube);
  glFlush();
}

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  if (action == GLFW_PRESS) {
    switch (key) {
    case GLFW_KEY_ESCAPE:
    case GLFW_KEY_Q:
      exit(EXIT_SUCCESS);
      break;
    case GLFW_KEY_C:
      if (*color_selection_ptr == 2) {
        *color_selection_ptr = 7;
      } else {
        *color_selection_ptr = 2;
      }
      paintColor = vertexColorsCube[*color_selection_ptr];
      init();
      break;
    case GLFW_KEY_I:
      h_current_ptr->x = -1;
      h_current_ptr->y = 1;
      ballSpeedY = 0;
      break;
    case GLFW_KEY_H:
      printf("Press h to get help \n");
      printf("Press i to initialize the pose \n");
      printf("Press c to change color \n");
      printf("Press q to quit \n");
      break;
    }
  }
}

void mouse_button_callback(GLFWwindow *window, int button, int action,
                           int mods) {
  if (action == GLFW_PRESS) {
    switch (button) {
    case GLFW_MOUSE_BUTTON_RIGHT:
      break;
      // set the current object to be drawn
      // cube
      // sphere
    case GLFW_MOUSE_BUTTON_MIDDLE:
      break;
    case GLFW_MOUSE_BUTTON_LEFT:
      if (isSolid) {
        isSolid = false;
        draw_mode = GL_LINES;
      } else {
        isSolid = true;
        draw_mode = GL_TRIANGLES;
      }
      // set how the triangles are drawn;
      // wireframe (i.e., as lines)s olid mode break;
    }
  }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(
      0, 0, width,
      height); // may not need this since the default is usually the window size

  // Set projection matrix
  mat4 projection;
  if (width <= height)
    projection = Ortho(-1.0, 1.0, -1.0 * (GLfloat)height / (GLfloat)width,
                       1.0 * (GLfloat)height / (GLfloat)width, -1.0, 1.0);
  else
    projection =
        Ortho(-1.0 * (GLfloat)width / (GLfloat)height,
              1.0 * (GLfloat)width / (GLfloat)height, -1.0, 1.0, -1.0, 1.0);

  glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);
}

//---------------------------------------------------------------------
//
// main
//

int main() {
  if (!glfwInit())
    exit(EXIT_FAILURE);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

  GLFWwindow *window = glfwCreateWindow(1440, 1080, "Ball", NULL, NULL);
  glfwMakeContextCurrent(window);
  glewExperimental = GL_TRUE;
  glewInit();
  if (!window) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwSetKeyCallback(window, key_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  init();

  while (!glfwWindowShouldClose(window)) {
    display();
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
