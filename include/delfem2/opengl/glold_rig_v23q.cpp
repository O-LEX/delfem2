/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */


#include "delfem2/mat3.h"
#include "delfem2/vec3.h"
#include "delfem2/rig_v3q.h"


// -------------------------

#if defined(__APPLE__) // Mac
#include <OpenGL/gl.h>
#elif defined(_WIN32) // windows
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "delfem2/opengl/glold_funcs.h"
#include "delfem2/opengl/glold_v23.h"
#include "delfem2/opengl/glold_rig_v23q.h"


#ifndef M_PI 
#define M_PI 3.1415926535
#endif

namespace dfm2 = delfem2;

// -------------------------------------------------------

void dfm2::opengl::Draw_RigBone
(int ibone,
 bool is_selected,
 int ielem_selected,
 const std::vector<dfm2::CRigBone>& aBone,
 double rad_bone_sphere,
 double rad_rot_hndlr)
{
  { // draw point
    if(is_selected){ ::glColor3d(0,1,1); }
    else{            ::glColor3d(1,0,0); }
    const dfm2::CVec3d pos = aBone[ibone].Pos();
    delfem2::opengl::DrawSphereAt(32, 32, rad_bone_sphere, pos.x(),pos.y(),pos.z());
  }
  if(is_selected){
    dfm2::opengl::DrawHandlerRotation_Mat4(aBone[ibone].affmat3Global, rad_rot_hndlr, ielem_selected);
    int ibone_parent = aBone[ibone].ibone_parent;
    if( ibone_parent>=0&&ibone_parent<(int)aBone.size() ){
      const dfm2::CVec3d pp(aBone[ibone_parent].Pos());
    }
    else{
    }
  }
}

void dfm2::opengl::DrawBone
(const std::vector<dfm2::CRigBone>& aBone,
 int ibone_selected,
 int ielem_selected,
 double rad_bone_sphere,
 double rad_rot_hndlr)
{
  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);
  ::glPointSize(3);
  for( int iskel=0;iskel<(int)aBone.size();++iskel){
    Draw_RigBone(iskel,
                 (iskel==ibone_selected),ielem_selected,aBone,
                 rad_bone_sphere,rad_rot_hndlr);
  }
  // draw edges whilte
  for( int ibone=0;ibone<(int)aBone.size();++ibone){
    const dfm2::CRigBone& bone = aBone[ibone];
    const int ibone_p = aBone[ibone].ibone_parent;
    if( ibone_p < 0 || ibone_p >= (int)aBone.size() ){ continue; }
    const dfm2::CRigBone& bone_p = aBone[ibone_p];
    bool is_selected_p = (ibone_p == ibone_selected);
    if(is_selected_p){ ::glColor3d(1.0,1.0,1.0); }
    else{              ::glColor3d(0.0,0.0,0.0); }
    ::glBegin(GL_LINES);
    dfm2::opengl::myGlVertex(bone.Pos());
    dfm2::opengl::myGlVertex(bone_p.Pos());
    ::glEnd();
  }
}

void dfm2::opengl::DrawJoints(
    const std::vector<double>& aJntPos,
    const std::vector<int>& aIndBoneParent)
{
  for(int ib=0;ib<aJntPos.size()/3;++ib){
    const double* p = aJntPos.data()+ib*3;
    ::glColor3d(1,0,0);
    ::glDisable(GL_LIGHTING);
    ::glDisable(GL_DEPTH_TEST);
    dfm2::opengl::DrawSphereAt(8, 8, 0.01, p[0], p[1], p[2]);
    int ibp = aIndBoneParent[ib];
    if( ibp == -1 ){ continue; }
    const double* pp = aJntPos.data()+ibp*3;
    ::glColor3d(0,0,0);
    ::glBegin(GL_LINES);
    ::glVertex3dv(p);
    ::glVertex3dv(pp);
    ::glEnd();
  }
}
