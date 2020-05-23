/*
 * Copyright (c) 2020 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <iostream>
#include <fstream>
#include <cmath>
#include "delfem2/imgio.h"
#include "delfem2/primitive.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "delfem2/opengl/glfw/viewer_glfw.h"
#include "delfem2/opengl/gl_funcs.h"
#include "delfem2/opengl/tex_gl.h"
#include "delfem2/opengl/funcs_glold.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace dfm2 = delfem2;

// -----------------------------

std::string LoadFile
(const std::string& fname)
{
  std::ifstream inputFile1(fname.c_str());
  std::istreambuf_iterator<char> vdataBegin(inputFile1);
  std::istreambuf_iterator<char> vdataEnd;
  return std::string(vdataBegin,vdataEnd);
}

void setShaderProgram(
    int& id_shader_program,
    int isp,
    unsigned int nTexWidth,
    unsigned int nTexHeight)
{
  std::string glslVert, glslFrag;
  glUseProgram(0);
  glDeleteProgram(id_shader_program);
  if( isp == 0 ){
    glslVert = LoadFile(std::string(PATH_INPUT_DIR)+"/glsl_tex.vert");
    glslFrag = LoadFile(std::string(PATH_INPUT_DIR)+"/glsl_tex.frag");
  }
  else if( isp == 1 ){
    glslVert = LoadFile(std::string(PATH_INPUT_DIR)+"/glsl_tex.vert");
    glslFrag = LoadFile(std::string(PATH_INPUT_DIR)+"/glsl_tex_diffx.frag");
  }
  else if( isp == 2 ){
    glslVert = LoadFile(std::string(PATH_INPUT_DIR)+"/glsl_tex.vert");
    glslFrag = LoadFile(std::string(PATH_INPUT_DIR)+"/glsl_tex_sobel.frag");
  }
  id_shader_program = delfem2::opengl::setUpGLSL(glslVert, glslFrag);
  //  
  glUseProgram(id_shader_program);
  {
    GLint texLoc = glGetUniformLocation(id_shader_program, "ourTexture");
    glUniform1i(texLoc, 0); // GL_TEXTURE0
  }
  {
    GLint texLoc = glGetUniformLocation(id_shader_program, "nTexWidth");
    glUniform1i(texLoc, nTexWidth); // GL_TEXTURE0
  }
  {
    GLint texLoc = glGetUniformLocation(id_shader_program, "nTexHeight");
    glUniform1i(texLoc, nTexHeight); // GL_TEXTURE0
  }
  glUseProgram(0);
}

// ------------------------------------------------------

int main(int argc,char* argv[])
{
  dfm2::opengl::CTexRGB_Rect2D tex;
  int width, height;
  {
    double scale = 0.01;
    std::string name_img_in_test_inputs = "lenna.png";
    // -----
    int channels;
    unsigned char *img = stbi_load((std::string(PATH_INPUT_DIR)+"/"+name_img_in_test_inputs).c_str(),
                                   &width, &height, &channels, 0);
    tex.Initialize(width, height, img, "rgb");
    delete[] img;
    tex.max_x = -scale*width*0.5;
    tex.min_x = +scale*width*0.5;
    tex.max_y = -scale*height*0.5;
    tex.min_y = +scale*height*0.5;
    tex.z = -0.5;
  }
  // -------------------
  
  dfm2::opengl::CViewer_GLFW viewer;
  viewer.Init_oldGL();
  if(!gladLoadGL()) {     // glad: load all OpenGL function pointers
    printf("Something went wrong in loading OpenGL functions!\n");
    exit(-1);
  }
  viewer.nav.camera.view_height = 2.0;
  viewer.nav.camera.camera_rot_mode = delfem2::CCamera<double>::CAMERA_ROT_MODE::TBALL;
  delfem2::opengl::setSomeLighting();
  std::cout<<"Vendor:"<<glGetString(GL_VENDOR)<<'\n';
  std::cout<<"GPU: "<<glGetString(GL_RENDERER)<<'\n';
  std::cout<<"OpenGL ver.: "<<glGetString(GL_VERSION)<<'\n';
  std::cout<<"OpenGL shading ver.: " <<glGetString(GL_SHADING_LANGUAGE_VERSION)<<std::endl;
  
  // -------------------------

  tex.InitGL();

  int id_shader_program;
  while (!glfwWindowShouldClose(viewer.window))
  {
    glfwSetWindowTitle(viewer.window, "naive");
    setShaderProgram(id_shader_program, 0,width,height);
    for(int iframe=0;iframe<100;++iframe){
      viewer.DrawBegin_oldGL();
      ::glBindTexture(GL_TEXTURE_2D, tex.id_tex);
      glUseProgram(id_shader_program);
      tex.Draw_oldGL();
      glfwSwapBuffers(viewer.window);
      glfwPollEvents();
      if( glfwWindowShouldClose(viewer.window) ) goto EXIT;
    }
    // -------
    glfwSetWindowTitle(viewer.window, "diffx");
    setShaderProgram(id_shader_program, 1,width,height);
    for(int iframe=0;iframe<100;++iframe){
      viewer.DrawBegin_oldGL();
      ::glBindTexture(GL_TEXTURE_2D, tex.id_tex);
      glUseProgram(id_shader_program);
      tex.Draw_oldGL();
      glfwSwapBuffers(viewer.window);
      glfwPollEvents();
      if( glfwWindowShouldClose(viewer.window) ) goto EXIT;
    }
    // -------
    glfwSetWindowTitle(viewer.window, "sobel");
    setShaderProgram(id_shader_program, 2,width,height);
    for(int iframe=0;iframe<100;++iframe){
      viewer.DrawBegin_oldGL();
      ::glBindTexture(GL_TEXTURE_2D, tex.id_tex);
      glUseProgram(id_shader_program);
      tex.Draw_oldGL();
      glfwSwapBuffers(viewer.window);
      glfwPollEvents();
      if( glfwWindowShouldClose(viewer.window) ) goto EXIT;
    }
  }
EXIT:
  glfwDestroyWindow(viewer.window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}


