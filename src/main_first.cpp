//
//  Display a rotating cube
//

#include "Angel.h"
#include <cmath>

typedef vec4 color4;
typedef vec4 point4;

const int NumVerticesBallCUBE = 36;
float radius = 0.025;
float MaxValues = 0.5;
int color_counter = 0;
float ballSpeedX = 0.0005;
float ballSpeedY = 0;
float ballSpeedZ = 0;
float gravitiy = 0.00098;
float dt = 0.1;
float MaxY = 1.01;
float MinY = -1.01;
float rho = 0.85;
int second_counter = 1;
int *second_counter_ptr = &second_counter;
int color_selection;
vec3 h_current(-1.0, 1.0, 0.0);
vec3 *h_current_ptr = &h_current;

point4 pointsBall[NumVerticesBallCUBE];
color4 colorsBall[NumVerticesBallCUBE];

// Vertices of a unit cube centered at origin, sides aligned with axes
point4 verticesBall[8]{point4(-radius, -radius, radius, 1.0),
                       point4(-radius, radius, radius, 1.0),
                       point4(radius, radius, radius, 1.0),
                       point4(radius, -radius, radius, 1.0),
                       point4(-radius, -radius, -radius, 1.0),
                       point4(-radius, radius, -radius, 1.0),
                       point4(radius, radius, -radius, 1.0),
                       point4(radius, -radius, -radius, 1.0)};

color4 vertex_colors[8] = {
    color4(0.0, 0.0, 0.0, 1.0), // black
    color4(1.0, 0.0, 0.0, 1.0), // red
    color4(1.0, 1.0, 0.0, 1.0), // yellow
    color4(0.0, 1.0, 0.0, 1.0), // green
    color4(0.0, 0.0, 1.0, 1.0), // blue
    color4(1.0, 0.0, 1.0, 1.0), // magenta
    color4(1.0, 1.0, 1.0, 1.0), // white
    color4(0.0, 1.0, 1.0, 1.0)  // cyan
};

color4 paintColor = vertex_colors[1];

// Array of rotation angles (in degrees) for each coordinate axis
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int Axis = Xaxis;
GLfloat Theta[NumAxes] = {0.0, 0.0, 0.0};

// Model-view and projection matrices uniform location
GLuint ModelView, Projection;

//----------------------------------------------------------------------------

const int NumTimesToSubdivide = 5;
const int NumTriangles = 4096;
// (4 faces)^(NumTimesToSubdivide + 1)
const int NumVerticesSPHERE = 3 * NumTriangles;
point4 pointsSPHERE[NumVerticesSPHERE];
color4 colorsSPHERE[NumVerticesSPHERE];
vec3 normals[NumVerticesSPHERE];

//----------------------------------------------------------------------
int IndexSPHERE = 0;
void triangle(const point4 &a, const point4 &b, const point4 &c) {
  colorsSPHERE[IndexSPHERE] = paintColor;
  pointsSPHERE[IndexSPHERE] = a;
  IndexSPHERE++;
  colorsSPHERE[IndexSPHERE] = paintColor;
  pointsSPHERE[IndexSPHERE] = b;
  IndexSPHERE++;
  colorsSPHERE[IndexSPHERE] = paintColor;
  pointsSPHERE[IndexSPHERE] = c;
  IndexSPHERE++;
}

point4 unit(const point4 &p) {
  float len = p.x * p.x + p.y * p.y + p.z * p.z;
  point4 t;
  if (len > DivideByZeroTolerance) {
    t = p / sqrt(len);
    t.w = 1.0;
  }
  return t;
}

void divide_triangle(const point4 &a, const point4 &b, const point4 &c,
                     int count) {
  if (count > 0) {
    point4 v1 = unit(a + b);
    point4 v2 = unit(a + c);
    point4 v3 = unit(b + c);
    divide_triangle(a, v1, v2, count - 1);
    divide_triangle(c, v2, v3, count - 1);
    divide_triangle(b, v3, v1, count - 1);
    divide_triangle(v1, v3, v2, count - 1);
  } else {
    triangle(a, b, c);
  }
}

void tetrahedron(int count, vec4 ballCenter) {
  point4 v[4] = {vec4(0.0, 0.0, 1.0, 1.0), vec4(0.0, 0.942809, -0.333333, 1.0),
                 vec4(-0.816497, -0.471405, -0.333333, 1.0),
                 vec4(0.816497, -0.471405, -0.333333, 1.0)};
  IndexSPHERE = 0;
  divide_triangle(v[0], v[1], v[2], count);
  divide_triangle(v[3], v[2], v[1], count);
  divide_triangle(v[0], v[3], v[1], count);
  divide_triangle(v[0], v[2], v[3], count);

  for (int i = 0; i < NumVerticesSPHERE; i++) {
    pointsSPHERE[i] =
        vec4(pointsSPHERE[i].x * (radius), pointsSPHERE[i].y * (radius),
             pointsSPHERE[i].z * (radius), 1.0);
    pointsSPHERE[i] =
        vec4(pointsSPHERE[i].x + ballCenter.x, pointsSPHERE[i].y + ballCenter.y,
             pointsSPHERE[i].z + ballCenter.z, 1.0);
  }
}

//----------------------------------------------------------------------

GLuint cube_indices[] = {0, 1, 3, 2, 3, 0, 1, 5, 6, 6, 2, 1, 7, 6, 5, 5, 4, 7,
                         4, 0, 3, 3, 7, 4, 4, 5, 1, 1, 0, 4, 3, 2, 6, 6, 7, 3};
int IndexBall = 0;
point4 BallCenter = point4(0, 0, 0, 0);

// colors for the inner cube or sphere is determined here.
void quadBall(int a, int b, int c, int d) {
  // Initialize colors
  colorsBall[IndexBall] = paintColor;
  pointsBall[IndexBall] = verticesBall[a] + BallCenter;
  IndexBall++;
  colorsBall[IndexBall] = paintColor;
  pointsBall[IndexBall] = verticesBall[b] + BallCenter;
  IndexBall++;
  colorsBall[IndexBall] = paintColor;
  pointsBall[IndexBall] = verticesBall[c] + BallCenter;
  IndexBall++;
  colorsBall[IndexBall] = paintColor;
  pointsBall[IndexBall] = verticesBall[a] + BallCenter;
  IndexBall++;
  colorsBall[IndexBall] = paintColor;
  pointsBall[IndexBall] = verticesBall[c] + BallCenter;
  IndexBall++;
  colorsBall[IndexBall] = paintColor;
  pointsBall[IndexBall] = verticesBall[d] + BallCenter;
  IndexBall++;
}

void SmallBall() {
  IndexBall = 0;
  quadBall(1, 0, 3, 2);
  quadBall(2, 3, 7, 6);
  quadBall(3, 0, 4, 7);
  quadBall(6, 5, 1, 2);
  quadBall(4, 5, 6, 7);
  quadBall(5, 4, 0, 1);
}

// Create a vertex array object
GLuint vao;
void init() {
  // Load shaders and use the resulting shader program
  GLuint program = InitShader("src/vshader.glsl", "src/fshader.glsl");
  glUseProgram(program);

  // create the ball
  SmallBall();
  tetrahedron(NumTimesToSubdivide, BallCenter);

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // Create and initialize a buffer object
  GLuint buffer;
  glGenBuffers(1, &buffer);
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(pointsBall) + sizeof(colorsBall), NULL,
               GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(pointsBall), pointsBall);
  glBufferSubData(GL_ARRAY_BUFFER, sizeof(pointsBall), sizeof(colorsBall),
                  colorsBall);

  // set up vertex arrays
  GLuint vPosition = glGetAttribLocation(program, "vPosition");
  glEnableVertexAttribArray(vPosition);
  glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

  GLuint vColor = glGetAttribLocation(program, "vColor");
  glEnableVertexAttribArray(vColor);
  glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
                        BUFFER_OFFSET(sizeof(pointsBall)));

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
  glClearColor(0, 0, 0, 0);
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
  glDrawArrays(GL_TRIANGLES, 0, NumVerticesBallCUBE);
  glFlush();
}

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  switch (key) {
  case GLFW_KEY_ESCAPE:
  case GLFW_KEY_Q:
    exit(EXIT_SUCCESS);
    break;
  case GLFW_KEY_C:
    color_counter += 1;
    color_selection = color_counter % 2;
    printf("%d", color_counter);
    printf("%d", color_selection);
    glColor();
    paintColor = vertex_colors[color_selection];
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

void mouse_button_callback(GLFWwindow *window, int button, int action,
                           int mods) {
  if (action == GLFW_PRESS) {
    switch (button) {
    case GLFW_MOUSE_BUTTON_RIGHT:
      Axis = Xaxis;
      // set the current object to be drawn
      // cube
      // sphere
    case GLFW_MOUSE_BUTTON_MIDDLE:
      Axis = Yaxis;
      break;
    case GLFW_MOUSE_BUTTON_LEFT:
      Axis = Zaxis;
      // set how the triangles are drawn;
      // wireframe (i.e., as lines)s olid mode break;
    }
  }
}

void update(void) {
  Theta[Axis] += 1.0;
  if (Theta[Axis] > 360.0) {
    Theta[Axis] -= 360.0;
  }
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

  init();

  double frameRate = 120, currentTime, previousTime = 0.0;
  while (!glfwWindowShouldClose(window)) {
    currentTime = glfwGetTime();
    if (currentTime - previousTime >= 1 / frameRate) {
      previousTime = currentTime;
      update();
    }

    display();
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
