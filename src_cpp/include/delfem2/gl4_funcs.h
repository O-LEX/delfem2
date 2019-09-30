/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */


#ifndef GL4_FUNCS_H
#define GL4_FUNCS_H

#include <vector>
#include <assert.h>
#include <iostream> // this must be delated in future

/**
 * @details the OpenGL ES 2.0 only accept float array. So there is no "double" version of this file
 */
void GL4_VAO_Pos(unsigned int& VAO,
                       unsigned int& VBO,
                       const float* aP, int nP, int nDim);

void GL4_VAO_PosNrm(unsigned int& VAO,
                    unsigned int& VBO_pos,
                    unsigned int& VBO_nrm,
                    const float* aP, int nP, int nDim,
                    const float* aN);


class CGL4_VAO_Mesh
{
public:
  class CElem{
  public:
    int GL_MODE;
    unsigned int size;
    int EBO;
  };
public:
  CGL4_VAO_Mesh(){
    VAO = 0;
    VBO_pos = 0;
  }
  void Draw(unsigned int iel) const;
  void Delete_EBOs(){
    for(int ie=0;ie<aElem.size();++ie){
       unsigned int ebo = aElem[ie].EBO;
       if( glIsBuffer(ebo) ){ glad_glDeleteBuffers(1,&ebo); }
     }
     aElem.clear();
  }
public:
  unsigned int VAO;
  unsigned int VBO_pos;
  std::vector<CElem> aElem;
};


const std::string glsl33vert_projection =
"uniform mat4 matrixProjection;\n"
"uniform mat4 matrixModelView;\n"
"layout (location = 0) in vec3 posIn;\n"
"layout (location = 1) in vec3 nrmIn;\n"
"out vec3 nrmPrj;\n"
"void main()\n"
"{\n"
"  gl_Position = matrixProjection * matrixModelView * vec4(posIn.x, posIn.y, posIn.z, 1.0);\n"
"  vec4 v0 = matrixModelView * vec4(nrmIn.x, nrmIn.y, nrmIn.z, 0.0);\n"
"  nrmPrj = v0.xyz;\n"
"  if( length(nrmIn) < 1.e-30 ){ nrmPrj = vec3(0.f, 0.f, 1.f); }\n"
"}\0";

const std::string glsl33frag =
"uniform vec3 color;\n"
"in vec3 nrmPrj;\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"  FragColor = abs(nrmPrj.z)*vec4(color.x, color.y, color.z, 1.0f);\n"
"}\n\0";

#endif /* utility_glew_h */

