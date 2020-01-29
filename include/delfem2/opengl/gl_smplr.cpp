/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */


#include <iostream>
#include <cmath>
#include <stack>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include "delfem2/vec3.h"
// ------
#include "glad/glad.h" // gl3.0+
#if defined(__APPLE__) && defined(__MACH__) // Mac
  #include <OpenGL/gl.h>
#elif defined(_WIN32) // windows
  #include <windows.h>
  #include <GL/gl.h>
#else
  #include <GL/gl.h>
#endif
#include "delfem2/opengl/gl_smplr.h"

namespace dfm2 = delfem2;

// --------------------------------------------------------

static double DotX(const double* p0, const double* p1, int ndof){
  double v=0;
  for(int i=0;i<ndof;++i){ v += p0[i]*p1[i]; }
  return v;
}

static void ScaleX(double* p0, int n, double s)
{
  for(int i=0;i<n;++i){ p0[i] *= s; }
}

static void NormalizeX(double* p0, int n)
{
  const double ss = DotX(p0,p0,n);
  ScaleX(p0,n,1.0/sqrt(ss));
}

// --------------------------------------------


void delfem2::opengl::CRender2Tex::SetCoord
(double elen, double depth_max,
 const std::vector<double>& org_prj,
 const std::vector<double>& dir_prj,
 const std::vector<double>& dir_width)
{
  this->lengrid = elen;
  this->z_range = depth_max;
  z_axis[0] = dir_prj[0];   z_axis[1] = dir_prj[1];   z_axis[2] = dir_prj[2];
  origin[0] = org_prj[0];   origin[1] = org_prj[1];   origin[2] = org_prj[2];
  x_axis[0] = dir_width[0]; x_axis[1] = dir_width[1]; x_axis[2] = dir_width[2];
  NormalizeX(z_axis,3);
  NormalizeX(x_axis,3);
}

void delfem2::opengl::CRender2Tex::AffMatT3f_MVP
 (float mMV[16],
  float mP[16]) const
{
  {
    const double* ax = this->x_axis;
    const double* az = this->z_axis;
    double ay[3]; dfm2::Cross3(ay, az, ax);
    const double o[3] = { dfm2::Dot3(ax,origin), dfm2::Dot3(ay,origin), dfm2::Dot3(az,origin) };
    mMV[ 0] = ax[0];  mMV[ 1] = ay[0];  mMV[ 2] = az[0];  mMV[ 3] = 0;
    mMV[ 4] = ax[1];  mMV[ 5] = ay[1];  mMV[ 6] = az[1];  mMV[ 7] = 0;
    mMV[ 8] = ax[2];  mMV[ 9] = ay[2];  mMV[10] = az[2];  mMV[11] = 0;
    mMV[12] = -o[0];  mMV[13] = -o[1];  mMV[14] = -o[2];  mMV[15] = 1;
  }
  {
    double l = 0.0;
    double r = +lengrid*nResX;
    double b = 0.0;
    double t = +lengrid*nResY;
    double n = -z_range;
    double f = 0;
    mP[0*4+0] = 2.0/(r-l);    mP[1*4+0] = 0.0;         mP[2*4+0] = 0.0;        mP[3*4+0] = -(l+r)/(r-l);
    mP[0*4+1] = 0.0;          mP[1*4+1] = 2.0/(t-b);   mP[2*4+1] = 0.0;        mP[3*4+1] = -(t+b)/(t-b);
    mP[0*4+2] = 0.0;          mP[1*4+2] = 0.0;         mP[2*4+2] = 2.0/(n-f);  mP[3*4+2] = -(n+f)/(n-f);
    mP[0*4+3] = 0.0;          mP[1*4+3] = 0.0;         mP[2*4+3] = 0.0;        mP[3*4+3] = 1.0;
  }
}


void dfm2::opengl::CRender2Tex::Start()
{
  glGetIntegerv(GL_VIEWPORT, view); // current viewport
  ::glViewport(0, 0, nResX, nResY);
  ::glBindFramebuffer(GL_FRAMEBUFFER, id_framebuffer);
}

void dfm2::opengl::CRender2Tex::End()
{
  ::glBindFramebuffer(GL_FRAMEBUFFER, 0);
  ::glViewport(view[0], view[1], view[2], view[3]);  
}

void dfm2::opengl::CRender2Tex::ExtractFromTexture_Depth
 (std::vector<float>& aZ)
{
#ifdef EMSCRIPTEN
  std::cout << "the function \"glGetTexImage\" is not supported in emscripten" << std::endl;
  return;
#endif
  aZ.resize(nResX*nResY);
  ::glBindFramebuffer(GL_FRAMEBUFFER, id_framebuffer);
  ::glBindTexture(GL_TEXTURE_2D, id_tex_depth);
  ::glGetTexImage(GL_TEXTURE_2D, 0,
                  GL_DEPTH_COMPONENT, GL_FLOAT,
                  (void*)aZ.data());
  ::glBindTexture(GL_TEXTURE_2D, 0);
  ::glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void dfm2::opengl::CRender2Tex::ExtractFromTexture_RGBA8UI
(std::vector<std::uint8_t>& aRGBA)
{
#ifdef EMSCRIPTEN
  std::cout << "the function \"glGetTexImage\" is not supported in emscripten" << std::endl;
  return;
#endif
  aRGBA.resize(nResX*nResY*4);
  ::glBindTexture(GL_TEXTURE_2D, id_tex_color);
  ::glGetTexImage(GL_TEXTURE_2D, 0,
                  GL_RGBA, GL_UNSIGNED_BYTE,
                  (void*)aRGBA.data());
  ::glBindTexture(GL_TEXTURE_2D, 0);
  ::glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void dfm2::opengl::CRender2Tex::ExtractFromTexture_RGBA32F
 (std::vector<float>& aRGBA)
{
#ifdef EMSCRIPTEN
  std::cout << "the function \"glGetTexImage\" is not supported in emscripten" << std::endl;
  return;
#endif
  aRGBA.resize(nResX*nResY*4);
  std::vector<float> aF_RGBA(nResX*nResY*4);
  ::glBindTexture(GL_TEXTURE_2D, id_tex_color);
  ::glGetTexImage(GL_TEXTURE_2D, 0,
                  GL_RGBA, GL_FLOAT,
                  (void*)aF_RGBA.data());
  ::glBindTexture(GL_TEXTURE_2D, 0);
  ::glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void dfm2::opengl::CRender2Tex::InitGL()
{
  ::glEnable(GL_TEXTURE_2D);
  ::glActiveTexture(GL_TEXTURE0);

  { // initialize color texture
    // create and bind texture
    if( id_tex_color > 0 ){ glDeleteTextures(1, &id_tex_color); }
    ::glGenTextures(1, &id_tex_color);
    ::glBindTexture(GL_TEXTURE_2D, id_tex_color);
    // define size and format of level 0
    if( is_rgba_8ui ){
      ::glTexImage2D(GL_TEXTURE_2D, 0,
                     GL_RGBA, nResX, nResY, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }
    else{
      ::glTexImage2D(GL_TEXTURE_2D, 0,
                     GL_RGBA, nResX, nResY, 0,
                     GL_RGBA, GL_FLOAT, nullptr);
    }
    // set the filtering so we don't need mips
    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  }

  { // depth texture
    // create to render to
    if( id_tex_depth > 0 ){ glDeleteTextures(1, &id_tex_depth); }
    ::glGenTextures(1, &id_tex_depth);
    ::glBindTexture(GL_TEXTURE_2D, id_tex_depth);
    // define size and format of level 0
    ::glTexImage2D(GL_TEXTURE_2D, 0,
                   GL_DEPTH_COMPONENT32F, nResX, nResY, 0,
                   GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    // set the filtering so we don't need mips
    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  }
  ::glBindTexture(GL_TEXTURE_2D,0);
  {
    ::glGenFramebuffers(1,&id_framebuffer);
    ::glBindFramebuffer(GL_FRAMEBUFFER, id_framebuffer);
    // attach the texture as the first color attachment
    ::glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                             id_tex_color, 0);
    ::glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                             id_tex_depth, 0);
    // Always check that our framebuffer is ok
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER) ;
    if(status != GL_FRAMEBUFFER_COMPLETE){
      std::cout << "error!: " << status << std::endl;
      std::cout << GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT << std::endl;
      std::cout << GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT << std::endl;
      std::cout << GL_FRAMEBUFFER_UNSUPPORTED << std::endl;
      std::cout << GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER << std::endl;
      std::cout << GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE << std::endl;
      std::cout << GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT << std::endl;
      std::cout << GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER << std::endl;
      std::cout << GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER << std::endl;
    }
    ::glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
}


