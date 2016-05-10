// CPSC 453: Introduction to Computer Graphics
//
// Example program that demonstrates VBOs and shader usage with GLFW.
// The use of an index buffer is also demonstrated.
//
// Usman Alim
// Department of Computer Science
// University of Calgary

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>

using namespace std;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace glm;

#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib>
using namespace std;

#include "helper.h"


// VAO and VBO to handle vertex state and data
GLuint myVAO[1];
GLuint myVBO;
GLuint myIndexBuffer;

// shader program to use
GLuint myShaderProgram;

// dynamic positions of lighting x,y,z
GLfloat lightX = 1.0f;
GLfloat lightY = 1.0f;
GLfloat lightZ = -1.0f;

GLfloat rotateX = 0.0f;
GLfloat rotateY = 0.0f;
GLfloat rotateZ = 0.0f;
GLfloat coordMax = 0.0f;
GLfloat scaleAmount = 1.0f;
GLfloat scaleX = 1.0f;
GLfloat scaleY = 1.0f;
GLfloat scaleZ = 1.0f;
GLfloat min_x, max_x, min_y, max_y, min_z, max_z;
float specpower = 128.0;
int isTeapot = 0;
float myAmb1 = 0.1;
float myAmb2 = 0.1;
float myAmb3 = 0.1;
float x_center, y_center;
float specAlb1 = 0.7;
float specAlb2 = 0.7;
float specAlb3 = 0.7;
float diffAlb1 = 1.0;
float diffAlb2 = 1.0;
float diffAlb3 = 1.0;

bool neighbour = true;
bool trilinear = false;
bool bilinear = false;
bool texchange = false;

const char* texName;

GLuint numFaces = 0;

int width, height;
int wireframe = -1;

vector<GLfloat> vertices;
vector<GLushort> faceIndices;
vector<GLushort> texIndices;
vector<GLfloat> colours;
vector<GLfloat> Fnormals;
vector<GLfloat> normals;
vector<GLfloat> texCoords;
vector<GLfloat> sortedTexCoords;
vector<GLfloat> sortedVertices;
vector<GLfloat> sortedNormals;

GLuint texture;

////////////////////////////////////////////////////////////////
// Load the shader from the specified file. Returns false if the
// shader could not be loaded
static GLubyte shaderText[8192];
bool loadShaderFile(const char *filename, GLuint shader) {
  GLint shaderLength = 0;
  FILE *fp;
  
  // Open the shader file
  fp = fopen(filename, "r");
  if(fp != NULL) {
    // See how long the file is
    while (fgetc(fp) != EOF)
      shaderLength++;
    
    // Go back to beginning of file
    rewind(fp);
    
    // Read the whole file in
    fread(shaderText, 1, shaderLength, fp);
    
    // Make sure it is null terminated and close the file
    shaderText[shaderLength] = '\0';
    fclose(fp);
  }
  else {
    return false;
  }
  
  // Load the string into the shader object
  GLchar* fsStringPtr[1];
  fsStringPtr[0] = (GLchar *)((const char*)shaderText);
  glShaderSource(shader, 1, (const GLchar **)fsStringPtr, NULL );
  
  return true;
} 


// ???? Arrange the list of texture coordinates? FUCK IT LET'S TRY IT 
void arrangeList()
{
  for (int i = 0; i < texIndices.size(); i += 2)
  {

  }
}


// Constants to help with location bindings
#define VERTEX_DATA 0
#define VERTEX_COLOUR 1
#define VERTEX_NORMAL 1
#define VERTEX_TCOORDS 2
#define VERTEX_INDICES 3

// Load a TGA as a 2D Texture. Completely initialize the state
// Notice that texture min and max filters and the wrap mode 
// can be specified also.
bool LoadTGATexture(const char *szFileName, GLenum minFilter, 
        GLenum magFilter, GLenum wrapMode)
{
  GLbyte *pBits;
  int nWidth, nHeight, nComponents;
  GLenum eFormat;
  
  // Read the texture bits
  pBits = ReadTGABits(szFileName, &nWidth, &nHeight, &nComponents, &eFormat);
  if(pBits == NULL) 
    return false;
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    
  
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(GL_TEXTURE_2D, 0, nComponents, nWidth, nHeight, 0,
         eFormat, GL_UNSIGNED_BYTE, pBits);
  
  free(pBits);
  
  if(minFilter == GL_LINEAR_MIPMAP_LINEAR || 
     minFilter == GL_LINEAR_MIPMAP_NEAREST ||
     minFilter == GL_NEAREST_MIPMAP_LINEAR ||
     minFilter == GL_NEAREST_MIPMAP_NEAREST)
    glGenerateMipmap(GL_TEXTURE_2D);
  
  return true;
}

// Called to draw scene
void renderScene() {
  // Clear the window and the depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram( myShaderProgram );
  
  // change the position of the light (overriding what's in the vertex shader)
  glUniform3f(glGetUniformLocation(myShaderProgram, "light_pos"), lightX, lightY, lightZ);

  // override power of specular reflection with user controls
  glUniform1f(glGetUniformLocation(myShaderProgram, "specular_power"), specpower);

  // ovveride ambient value with user controls
  glUniform3f(glGetUniformLocation(myShaderProgram, "ambient"), myAmb1, myAmb2, myAmb3);

  glUniform3f(glGetUniformLocation(myShaderProgram, "specular_albedo"), specAlb1, specAlb2, specAlb3);

  glUniform3f(glGetUniformLocation(myShaderProgram, "diffuse_albedo"), diffAlb1, diffAlb2, diffAlb3);  

  glUniform1i(glGetUniformLocation(myShaderProgram, "texObject"), 0 );

  //setting model-view and projection matrices 
  mat4 identity(1.0f);

  // specifying rotations

  mat4 rotationX = rotate(mat4(1.0f), rotateX, vec3(1.0f, 0.0f, 0.0f));
  mat4 rotationY = rotate(mat4(1.0f), rotateY, vec3(0.0f, 1.0f, 0.0f));
  mat4 rotationZ = rotate(mat4(1.0f), rotateZ, vec3(1.0f, 0.0f, 1.0f));
  mat4 scaling = scale(mat4(1.0f), vec3(scaleX, scaleY, scaleZ));

  

  //view: takes 3 vectors. 1 = eye position, 2 = coords of point you're looking at, 3 = the "up" vector of the camera
  mat4 view = lookAt(vec3(0.0f,0.0f, -3.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f)) * rotationX * rotationY * rotationZ * scaling;

  glUniformMatrix4fv (glGetUniformLocation(myShaderProgram,"mv_matrix"), 1, GL_FALSE, value_ptr(view)) ; 

  mat4 proj = perspective(45.0f, (float)width / (float)height, 0.1f, 1000.0f);
  glUniformMatrix4fv (glGetUniformLocation(myShaderProgram,"proj_matrix"), 1, GL_FALSE, value_ptr(proj));

  // Note that this version of the draw command uses the
  // bound index buffer to get the vertex coordinates.


  // WIREFRAME SETTINGS

  if (wireframe == 1)
  {
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
  }
  if (wireframe == -1)
  {
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
  }

  // TEXTURE RENDERING SETTINGS

  if (texchange) // checks for user-specified texture mapping changes
  {
    if (neighbour)
    {
      if ( !LoadTGATexture( texName, GL_NEAREST, GL_NEAREST, 
                        GL_REPEAT) ) {
      printf("Could not load texture\n");
      }
    }

    if (bilinear)
    {
      if ( !LoadTGATexture( texName, GL_LINEAR, GL_LINEAR, 
                        GL_REPEAT) ) {
      printf("Could not load texture\n");
     }
    }

    if (trilinear)
    {
      if ( !LoadTGATexture( texName, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, 
                        GL_REPEAT) ) {
        printf("Could not load texture\n");
      }
    }

    texchange = false;
  }




  glDrawArrays( GL_TRIANGLES, 0, faceIndices.size());
  
}

// This function does any needed initialization on the rendering
// context. 
void setupRenderingContext() {
  // Background
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f );
  glEnable(GL_DEPTH_TEST);
  
  // First setup the shaders
  //Now, let's setup the shaders
  GLuint hVertexShader = glCreateShader(GL_VERTEX_SHADER);
  GLuint hFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  myShaderProgram = (GLuint)NULL;
  GLint testVal;
  
  if( !loadShaderFile("textured-phong.vs.glsl", hVertexShader) ) {
    glDeleteShader( hVertexShader );
    glDeleteShader( hFragmentShader );
    cout << "The shader " << "textured-phong.vs.glsl" << " could not be found." << endl;
  }
  
  if( !loadShaderFile("textured-phong.fs.glsl", hFragmentShader) ) {
    glDeleteShader( hVertexShader );
    glDeleteShader( hFragmentShader );
    cout << "The shader " << "textured-phong.fs.glsl" << " could not be found." << endl;
  }
  
  glCompileShader(hVertexShader);
  glCompileShader(hFragmentShader);
  
  // Check for any error generated during shader compilation
  glGetShaderiv(hVertexShader, GL_COMPILE_STATUS, &testVal);
  if( testVal == GL_FALSE ) {
    char source[8192];
    char infoLog[8192];
    glGetShaderSource( hVertexShader, 8192, NULL, source );
    glGetShaderInfoLog( hVertexShader, 8192, NULL, infoLog);
    cout << "The shader: " << endl << (const char*)source << endl << " failed to compile:" << endl;
    fprintf( stderr, "%s\n", infoLog);
    glDeleteShader(hVertexShader);
    glDeleteShader(hFragmentShader);
  }
  glGetShaderiv(hFragmentShader, GL_COMPILE_STATUS, &testVal);
  if( testVal == GL_FALSE ) {
    char source[8192];
    char infoLog[8192];
    glGetShaderSource( hFragmentShader, 8192, NULL, source);
    glGetShaderInfoLog( hFragmentShader, 8192, NULL, infoLog);
    cout << "The shader: " << endl << (const char*)source << endl << " failed to compile:" << endl;
    fprintf( stderr, "%s\n", infoLog);    
    glDeleteShader(hVertexShader);
    glDeleteShader(hFragmentShader);
  }
  
  // Create the shader program and bind locations for the vertex
  // attributes before linking. The linking process can also generate errors
  
  myShaderProgram = glCreateProgram();
  glAttachShader(myShaderProgram, hVertexShader);
  glAttachShader(myShaderProgram, hFragmentShader);
  
  glBindAttribLocation( myShaderProgram, VERTEX_DATA, "position" );
  glBindAttribLocation( myShaderProgram, VERTEX_NORMAL, "normal" );
  glBindAttribLocation( myShaderProgram, VERTEX_TCOORDS, "tcoords");
  
  glLinkProgram( myShaderProgram );
  glDeleteShader( hVertexShader );
  glDeleteShader( hFragmentShader );
  glGetProgramiv( myShaderProgram, GL_LINK_STATUS, &testVal );
  if( testVal == GL_FALSE ) {
    char infoLog[1024];
    glGetProgramInfoLog( myShaderProgram, 1024, NULL, infoLog);
    cout << "The shader program" << "(textured-phong.vs.glsl + textured-phong.fs.glsl) failed to link:" << endl << (const char*)infoLog << endl;
    glDeleteProgram(myShaderProgram);
    myShaderProgram = (GLuint)NULL;
  }
  
  // Now setup the geometry in a vertex buffer object
  
  // setup the vertex state array object. All subsequent buffers will
  // be bound to it.
  glGenVertexArrays(1, myVAO);
  glBindVertexArray(myVAO[0]);
  
  glGenBuffers(1, &myVBO);
  glBindBuffer( GL_ARRAY_BUFFER, myVBO );
  
  // Allocate space and load vertex data into the buffer.
  // We are using one VBO for all the data. For this demo, we won't be
  // making use of the normals but the code below shown how we might
  // store them in the VBO.
  glBufferData(GL_ARRAY_BUFFER, (sizeof(GLfloat) * sortedVertices.size()) + 
               (sizeof(GLfloat) * sortedNormals.size()) + (sizeof(GLfloat) * sortedTexCoords.size()), NULL, 
               GL_STATIC_DRAW);

  glBufferSubData(GL_ARRAY_BUFFER, 0, (sizeof(GLfloat) * sortedVertices.size()), 
                 sortedVertices.data());

  glBufferSubData(GL_ARRAY_BUFFER, (sizeof(GLfloat) * sortedVertices.size()) ,(sizeof(GLfloat) * sortedNormals.size()), 
                  sortedNormals.data());

  glBufferSubData(GL_ARRAY_BUFFER, ((sizeof(GLfloat) * sortedVertices.size()) + (sizeof(GLfloat) * sortedNormals.size())), (sizeof(GLfloat) * sortedTexCoords.size()), sortedTexCoords.data());

  // Load face indices into the index buffer
  glGenBuffers(1, &myIndexBuffer );
  // glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, myIndexBuffer );
  // glBufferData( GL_ELEMENT_ARRAY_BUFFER, (sizeof(GLushort) * faceIndices.size()), faceIndices.data(), GL_STATIC_DRAW );
  
  // Now we'll use the attribute locations to map our vertex data (in
  // the VBO) to the shader
  glEnableVertexAttribArray( VERTEX_DATA );
  glVertexAttribPointer( VERTEX_DATA, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) 0 );
 
  glEnableVertexAttribArray( VERTEX_NORMAL );
  glVertexAttribPointer( VERTEX_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, 
                         (const GLvoid*)((sizeof(GLfloat) * sortedVertices.size())));

  glEnableVertexAttribArray(VERTEX_TCOORDS);
  glVertexAttribPointer(VERTEX_TCOORDS, 2, GL_FLOAT, GL_FALSE, 0,(const GLvoid*)((sizeof(GLfloat) * sortedVertices.size()) + (sizeof(GLfloat) * sortedNormals.size())));

  glGenTextures(1, &texture);
  glBindTexture( GL_TEXTURE_2D, texture );

  cout << "Loading texture... \n";

  if ( !LoadTGATexture( texName, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, 
                        GL_REPEAT) ) {
    printf("Could not load texture\n");
   }
}


/**
 * Keyboard callback function.
 */
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);

  if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT))
  {
    scaleX += 0.05f;
    scaleY += 0.05f;
    scaleZ += 0.05f;
  }

  if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT) && (scaleX > 0))
 {
    scaleX -= 0.05f;
    scaleY -= 0.05f;
    scaleZ -= 0.05f;
  }

  if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT))
    rotateX += 2.0f;

  if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT))
    rotateX -= 2.0f;

  if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT))
    rotateY += 2.0f;

  if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT))
    rotateY -= 2.0f;

  if (key == GLFW_KEY_Q && (action == GLFW_PRESS || action == GLFW_REPEAT))
    rotateZ -= 2.0f;

  if (key == GLFW_KEY_E && (action == GLFW_PRESS || action == GLFW_REPEAT))
    rotateZ += 2.0f;

  if (key == GLFW_KEY_F && (action == GLFW_PRESS || action == GLFW_REPEAT) && (specpower > 0))
    specpower -= 2.0f;

  if (key == GLFW_KEY_R && (action == GLFW_PRESS || action == GLFW_REPEAT))
    specpower += 2.0f;

  if (key == GLFW_KEY_G && (action == GLFW_PRESS || action == GLFW_REPEAT) && ((myAmb1 > 0) || (myAmb2 > 0) || (myAmb3 > 0)))
  {
    myAmb1 -= 0.1f;
    myAmb2 -= 0.1f;
    myAmb3 -= 0.1f;
  }

  if (key == GLFW_KEY_T && (action == GLFW_PRESS || action == GLFW_REPEAT))
  {
    myAmb1 += 0.1f;
    myAmb2 += 0.1f;
    myAmb3 += 0.1f;
  }

  if (key == GLFW_KEY_Y && (action == GLFW_PRESS || action == GLFW_REPEAT))
  {
    specAlb1 += 0.1f;
    specAlb2 += 0.1f;
    specAlb3 += 0.1f;
  }
    

  if (key == GLFW_KEY_H && (action == GLFW_PRESS || action == GLFW_REPEAT) && (specAlb1 > 0))
 {
    specAlb1 -= 0.1f;
    specAlb2 -= 0.1f;
    specAlb3 -= 0.1f;
  }

 if (key == GLFW_KEY_Z && (action == GLFW_PRESS || action == GLFW_REPEAT) && (specAlb1 > 0))
    specAlb1 -= 0.1f;

  if (key == GLFW_KEY_X && (action == GLFW_PRESS || action == GLFW_REPEAT))
    specAlb1 += 0.1f;

  if (key == GLFW_KEY_C && (action == GLFW_PRESS || action == GLFW_REPEAT) && (specAlb2 > 0))
    specAlb2 -= 0.1f;

  if (key == GLFW_KEY_V && (action == GLFW_PRESS || action == GLFW_REPEAT))
    specAlb2 += 0.1f;

  if (key == GLFW_KEY_B && (action == GLFW_PRESS || action == GLFW_REPEAT) && (specAlb3 > 0))
    specAlb3 -= 0.1f;

  if (key == GLFW_KEY_N && (action == GLFW_PRESS || action == GLFW_REPEAT))
    specAlb3 += 0.1f;

  if (key == GLFW_KEY_SEMICOLON && (action == GLFW_PRESS || action == GLFW_REPEAT) && (diffAlb1 > 0))
    diffAlb1 -= 0.1f;

  if (key == GLFW_KEY_P && (action == GLFW_PRESS || action == GLFW_REPEAT))
    diffAlb1 += 0.1f;

  if (key == GLFW_KEY_K && (action == GLFW_PRESS || action == GLFW_REPEAT) && (diffAlb2 > 0))
    diffAlb2 -= 0.1f;

  if (key == GLFW_KEY_I && (action == GLFW_PRESS || action == GLFW_REPEAT))
    diffAlb2 += 0.1f;

  if (key == GLFW_KEY_L && (action == GLFW_PRESS || action == GLFW_REPEAT) && (diffAlb3 > 0))
    diffAlb3 -= 0.1f;

  if (key == GLFW_KEY_O && (action == GLFW_PRESS || action == GLFW_REPEAT))
    diffAlb3 += 0.1f;
    
  if (key == GLFW_KEY_U && (action == GLFW_PRESS || action == GLFW_REPEAT))
  {
    diffAlb1 += 0.1f;
    diffAlb2 += 0.1f;
    diffAlb3 += 0.1f;
  }

  if (key == GLFW_KEY_J && (action == GLFW_PRESS || action == GLFW_REPEAT) && (diffAlb1 > 0))
 {
    diffAlb1 -= 0.1f;
    diffAlb2 -= 0.1f;
    diffAlb3 -= 0.1f;
  }

  if ((key == GLFW_KEY_1 && action == GLFW_PRESS) && neighbour == false)
  {
    neighbour = true;
    bilinear = false;
    trilinear = false;
    texchange = true;
  }

  if ((key == GLFW_KEY_2 && action == GLFW_PRESS) && bilinear == false)
  {
    neighbour = false;
    bilinear = true;
    trilinear = false;
    texchange = true;
  }

  if ((key == GLFW_KEY_3 && action == GLFW_PRESS) && trilinear == false)
  {
    neighbour = false;
    bilinear = false;
    trilinear = true;
    texchange = true;
  }

}

void mouseButton (GLFWwindow *window, int button, int action, int mods)
{
  if (button == GLFW_MOUSE_BUTTON_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT))
    wireframe = wireframe * -1;    
}

/**
 * Error callback function
 */
static void error_callback(int error, const char* description)
{
  fputs(description, stderr);
}


void ChangeSize(int w, int h) { 
  // Set Viewport to window dimensions
  glViewport(0, 0, w, h);
}

int main(int argc, char **argv)
{
  GLFWwindow* window;
  glfwSetErrorCallback(error_callback);
  if (!glfwInit())
    exit(EXIT_FAILURE);
  
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
  //Anti-aliasing - This will have smooth polygon edges
  glfwWindowHint(GLFW_SAMPLES, 4); 

  ifstream filereader (argv[1]);
  ifstream mtlReader (argv[3]);

  //OPEN FILE
  
  if (argc != 4) // are there 4 arguments?
  {
    cout << "Usage: " << argv[0] << " <OBJ filename> <TGA filename> <MTL filename>\n";
    return 1; // terminate the program
  }

  if (!filereader.is_open()) //check if the file is open
  {
    cout << "The file could not be opened.\n";
    return 1;
  }

  if (!mtlReader.is_open())
  {
    cout << "The MTL file could not be opened. \n";
    return 1;
  }

  string teapotTest = "models/teapot.obj";
  if (teapotTest.compare(argv[1]) == 0)
  {
    isTeapot = 1;
  }

  // READ OBJ

  char c[256];// creating a character array (buffer) to read the file into
  char s;
  cout << "Reading the file... \n";

  string str(argv[1]);
  texName = argv[2];

  while (filereader >> s) // will terminate at end of file
  {
    if (s == '#') //peek can look at the next character in the file, we use it to ignore comment lines
    {
      filereader.getline(c,256);
      cout << "Comment: " << c << '\n';
    }

    if (s == 'g') //peek can look at the next character in the file, we use it to ignore comment lines
    {
      filereader.getline(c,256);
      cout << "Ignoring: " << c << '\n';
    }

    if (s == 'm')
    {
      filereader.getline(c,256);
      cout << "Ignoring: " << c << '\n';
    }

    if (s == 'u')
    {
      filereader.getline(c,256);
      cout << "Ignoring: " << c << '\n';
    }


    if (s == 'v')
    { 
      // Additional case for "vt"
      if (filereader.peek() == 't')
      {
        //store the "t" in "vt" to throw away
        char m;
        filereader >> m;

        GLfloat tcoord1, tcoord2;

        filereader >> tcoord1;
        filereader >> tcoord2;

        texCoords.push_back(tcoord1);
        texCoords.push_back(tcoord2);

      }

      else
      {
        GLfloat x;
        GLfloat y;
        GLfloat z;

        filereader >> x;
        filereader >> y;
        filereader >> z;

        if (x > coordMax)
          coordMax = x;

        if (y > coordMax)
          coordMax = y;

        if (z > coordMax)
          coordMax = z;


        if (isTeapot == 1)
        {
          vertices.push_back(z);
          vertices.push_back(y);
          vertices.push_back(x);
        }
        else 
        {
          vertices.push_back(x);
          vertices.push_back(y);
          vertices.push_back(z);
        }
        
        vertices.push_back(1.0f);
      }
    }


    if (s == 'f')
    {

      GLushort index1, index2, index3, tex1, tex2, tex3;
      char s,t,u;

      filereader >> index1;
      filereader >> s;
      filereader >> tex1;

      filereader >> index2;
      filereader >> t;
      filereader >> tex2;

      filereader >> index3;
      filereader >> u;
      filereader >> tex3;

      // cout << "Texture indices: " << tex1 << ", " << tex2 << ", " << tex3 << "\n";

      index1--;
      index2--;
      index3--;

      tex1--;
      tex2--;
      tex3--;

      faceIndices.push_back(index1);
      faceIndices.push_back(index2);
      faceIndices.push_back(index3);

      texIndices.push_back(tex1);
      texIndices.push_back(tex2);
      texIndices.push_back(tex3);

    }
  }
  
  min_x = max_x = vertices[0];
  min_y = max_y = vertices[1];
  min_z = max_z = vertices[2];

  GLfloat min_general = vertices[0];
  GLfloat max_general = vertices[0];

  // Compute maximum, minimum for each axis.

  for (int i = 0; i < vertices.size(); i += 4)
  {
    if (vertices[i] < min_x)
      min_x = vertices[i];
    if (vertices[i] > max_x)
      max_x = vertices[i];
    if (vertices[i + 1] < min_y)
      min_y = vertices[i + 1];
    if (vertices[i + 1] > max_y)
      max_y = vertices[i + 1];
    if (vertices[i + 2] < min_z)
      min_z = vertices[i + 2];
    if (vertices[i + 2] > max_z)
      max_z = vertices[i + 2];
  }

  // cout << "X max/min: " << max_x << "/" << min_x << " Y max/min: " << max_y << "/" << min_y << "\n";

  x_center = (max_x + min_x) / 2;
  y_center = (max_y + min_y) / 2;

  // cout << "BEFORE DRAWING: X center: " << x_center << ", Y center: " << y_center << "\n";

  // compute general maximum/minimum for all vertices.

   for (int i = 0; i < vertices.size(); i += 4)
  {
    if (vertices[i] < min_general)
      min_general = vertices[i];
    if (vertices[i] > max_general)
      max_general = vertices[i];
    if (vertices[i + 1] < min_general)
      min_general = vertices[i + 1];
    if (vertices[i + 1] > max_general)
      max_general = vertices[i + 1];
    if (vertices[i + 2] < min_general)
      min_general = vertices[i + 2];
    if (vertices[i + 2] > max_general)
      max_general = vertices[i + 2];
  }


// PER-VERTEX SCALING: Scales down the vertex coordinates to ones easily viewable without scaling.
  for (int i = 0; i < vertices.size(); i+= 4)
  {
    vertices[i] = vertices[i] / (max_general - min_general);
    vertices[i + 1] = vertices[i + 1] / (max_general - min_general);
    vertices [i + 2] = vertices[i + 2] / (max_general - min_general);
  }

  cout << "Calculating face normals... \n";

  //make face normals
  for (int i = 0; i < faceIndices.size(); i += 3)
  {
    int i1, i2, i3;
    vec3 vertex1;
    vec3 vertex2;
    vec3 vertex3;
    vec3 facenormal = vec3(0.0f, 0.0f, 0.0f);

    // get the vertices to be looked at
    i1 = faceIndices[i];
    i2 = faceIndices[i + 1]; 
    i3 = faceIndices[i + 2];
    
    // assembling vertex x/y/z vectors
    vertex1 = vec3(vertices[4 * i1], vertices[(4 * i1) + 1], vertices[(4 * i1) + 2]);
    vertex2 = vec3(vertices[4 * i2], vertices[(4 * i2) + 1], vertices[(4 * i2) + 2]);
    vertex3 = vec3(vertices[4 * i3], vertices[(4 * i3) + 1], vertices[(4 * i3) + 2]);

    facenormal = cross((vertex2-vertex1),(vertex3-vertex1));
    
    // Compute length for divide-by-zero check.
    float length = sqrt((facenormal.x * facenormal.x) + (facenormal.y * facenormal.y) + (facenormal.z * facenormal.z));
    

    if (length == 0)
    {
     
      Fnormals.push_back(facenormal.x);
      Fnormals.push_back(facenormal.y);
      Fnormals.push_back(facenormal.z); 
      continue;      
    }

    facenormal = normalize(facenormal);
   
      Fnormals.push_back(facenormal.x);
      Fnormals.push_back(facenormal.y);
      Fnormals.push_back(facenormal.z);
  }
 cout << "This model has " << vertices.size() / 4 << " vertices!\n";


// CALCULATE VERTEX NORMALS
  cout << "Calculating vertex normals... \n";
  for (int i = 0; i < vertices.size(); i = i + 4)
  {
    vec3 vertexnormal = vec3(0.0f,0.0f,0.0f);
    
    for (int j = 0; j < faceIndices.size(); j += 3)
    {
      if (faceIndices[j] == i / 4)
      {
        vertexnormal.x += Fnormals[j];
        vertexnormal.y += Fnormals[j+1];
        vertexnormal.z += Fnormals[j+2];
      }
      if (faceIndices[j+1] == i / 4)
      {
        vertexnormal.x += Fnormals[j];
        vertexnormal.y += Fnormals[j+1];
        vertexnormal.z += Fnormals[j+2];
      }
      if (faceIndices[j+2] == i / 4)
      {
        vertexnormal.x += Fnormals[j];
        vertexnormal.y += Fnormals[j+1];
        vertexnormal.z += Fnormals[j+2];
      }  
    }

    normals.push_back(vertexnormal.x);
    normals.push_back(vertexnormal.y);
    normals.push_back(vertexnormal.z);
    
  } 

  for (int i = 0; i < faceIndices.size(); i++)
  {
    GLushort v = faceIndices[i];
    GLushort t = texIndices[i];
    GLushort f = faceIndices[i];

    sortedVertices.push_back(vertices[v*4]);
    sortedVertices.push_back(vertices[v*4+1]);
    sortedVertices.push_back(vertices[v*4+2]);
    sortedVertices.push_back(vertices[v*4+3]); 

    sortedTexCoords.push_back(texCoords[t*2]);
    sortedTexCoords.push_back(texCoords[t*2+1]);

    // cout << "Looking at index "<< t*2 << " and " << t*2+1 <<": Value: " << texCoords[t*2] << " and " << texCoords[t*2+1] << ". \n";

    sortedNormals.push_back(normals[f*3]);
    sortedNormals.push_back(normals[f*3+1]);
    sortedNormals.push_back(normals[f*3+2]);

  }

  while (mtlReader >> s)
  {
    
    if (s == 'K')
    {
      char test = mtlReader.peek();
      if (test == 'a')
      {
        char throwaway;
        mtlReader >> throwaway;
        float x,y,z;

        mtlReader >> x;
        mtlReader >> y;
        mtlReader >> z;

        myAmb1 = x;
        myAmb2 = y;
        myAmb3 = z;

      }

      if (test == 'd')
      {
        char throwaway;
        mtlReader >> throwaway;
        float x,y,z;

        mtlReader >> x;
        mtlReader >> y;
        mtlReader >> z;

        diffAlb1 = x;
        diffAlb2 = y;
        diffAlb3 = z;
      }

      if (test == 's')
      {
        char throwaway;
        mtlReader >> throwaway;
        float x,y,z;

        mtlReader >> x;
        mtlReader >> y;
        mtlReader >> z;

        specAlb1 = x;
        specAlb2 = y;
        specAlb3 = z;
      }
      else mtlReader.getline(c,256);

    }
    else
    {
      mtlReader.getline(c,256);
    }

  }

  window = glfwCreateWindow(800, 800, "Homework 3: Textured OBJ Viewer", NULL, NULL);
  if (!window)
    {
      if( myShaderProgram ) {
        glDeleteProgram( myShaderProgram );
        glDeleteVertexArrays(1, myVAO);
      }
      glfwTerminate();
      exit(EXIT_FAILURE);
    }
  glfwMakeContextCurrent(window);
  glfwSetKeyCallback(window, key_callback);
  glfwSetMouseButtonCallback(window, mouseButton);

  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  if(err!=GLEW_OK)
    {
      //Problem: glewInit failed, something is seriously wrong.
      printf("glewInit failed, aborting.\n");
      exit(EXIT_FAILURE);
    }
  
  // These two lines will print out the version of OpenGL and GLSL
  // that are being used so that problems due to version differences
  // can easily be identified.
  
  printf( "OpenGL version: %s\n", (const char*)glGetString(GL_VERSION) );
  printf( "GLSL version: %s\n", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
  
  // Do any necessary initializations (enabling buffers, setting up
  // shaders, geometry etc., before entering the main loop.)
  // This is done by calling the function setupRenderingContext().
  
  setupRenderingContext();
  if( myShaderProgram ) {
    while (!glfwWindowShouldClose(window))
      {
     
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        
        renderScene();
        
        // Note that buffer swapping and polling for events is done here
        // so please don't do it in the function used to draw the scene.
        glfwSwapBuffers(window);
        glfwPollEvents();
      }
  }
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}

