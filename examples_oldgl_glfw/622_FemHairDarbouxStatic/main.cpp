/*
 * Copyright (c) 2020 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#if defined(_WIN32) // windows
#  define NOMINMAX   // to remove min,max macro
#  include <windows.h>  // this should come before glfw3.h
#endif
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>

#include "delfem2/ls_solver_block_sparse.h"
#include "delfem2/ls_pentadiagonal.h"
#include "delfem2/hair_darboux_solver.h"
#include "delfem2/hair_darboux_util.h"
#include "delfem2/mshuni.h"
#include "delfem2/glfw/viewer3.h"
#include "delfem2/glfw/util.h"
#include "delfem2/opengl/old/v3q.h"
#include "delfem2/opengl/old/funcs.h"

namespace dfm2 = delfem2;

// -------------------------------------

void myGlutDisplay(
    const std::vector<dfm2::CVec3d> &aP,
    const std::vector<dfm2::CVec3d> &aS,
    std::vector<unsigned int> &aIP_HairRoot) {
  assert(!aIP_HairRoot.empty());
  const unsigned int nhair = static_cast<unsigned int>(aIP_HairRoot.size()) - 1;
  for (unsigned int ihair = 0; ihair < nhair; ++ihair) {
    const unsigned int ips = aIP_HairRoot[ihair];
    const unsigned int ipe = aIP_HairRoot[ihair + 1];
    assert(aP.size() == aS.size());
    ::glDisable(GL_LIGHTING);
    ::glColor3d(1, 0, 0);
    ::glPointSize(3);
    ::glBegin(GL_POINTS);
    for (unsigned int ip = ips; ip < ipe; ++ip) {
      ::glVertex3d(aP[ip].x, aP[ip].y, aP[ip].z);
    }
    ::glEnd();
    // ------------
    ::glColor3d(0, 0, 0);
    ::glLineWidth(3);
    ::glBegin(GL_LINES);
    unsigned int ns = ipe - ips - 1;
    for (unsigned int is = 0; is < ns; ++is) {
      const unsigned int ip0 = ips + is + 0;
      assert(ip0 < aP.size());
      const unsigned int ip1 = ips + is + 1;
      assert(ip1 < aP.size());
      ::glVertex3d(aP[ip0].x, aP[ip0].y, aP[ip0].z);
      ::glVertex3d(aP[ip1].x, aP[ip1].y, aP[ip1].z);
    }
    ::glEnd();
    // ------------
    ::glBegin(GL_LINES);
    for (unsigned int is = 0; is < ns; ++is) {
      const unsigned int ip0 = ips + is + 0;
      assert(ip0 < aP.size());
      const unsigned int ip1 = ips + is + 1;
      assert(ip1 < aP.size());
      dfm2::CVec3d p01 = 0.5 * (aP[ip0] + aP[ip1]);
      double l01 = (aP[ip0] - aP[ip1]).norm();
      dfm2::opengl::myGlVertex(p01);
      dfm2::opengl::myGlVertex(p01 + (l01 * 0.5) * aS[is]);
    }
    ::glEnd();
  }
}

template <class BLOCK_LINEAR_SOLVER>
void Simulation(
    BLOCK_LINEAR_SOLVER &ls_solver,
    dfm2::glfw::CViewer3 &viewer,
    std::mt19937 &reng,
    std::uniform_real_distribution<double>& dist01,
    const std::vector<dfm2::CVec3d> &aP0,
    const std::vector<dfm2::CVec3d> &aS0,
    const double stiff_stretch,
    const double stiff_bendtwist[3],
    std::vector<unsigned int> &aIP_HairRoot){
  std::vector<dfm2::CVec3d> aS = aS0, aP = aP0;
  for (unsigned int ip = 0; ip < aP.size(); ++ip) {
    //auto rnd = dfm2::CVec3d::Random(dist01, reng) * 0.1;
    if (ls_solver.dof_bcflag[ip * 4 + 0] == 0) { aP[ip].p[0] += dist01(reng)*0.1; }
    if (ls_solver.dof_bcflag[ip * 4 + 1] == 0) { aP[ip].p[1] += dist01(reng)*0.1; }
    if (ls_solver.dof_bcflag[ip * 4 + 2] == 0) { aP[ip].p[2] += dist01(reng)*0.1; }
    if (ls_solver.dof_bcflag[ip * 4 + 3] == 0) {
      assert(ip != aP.size() - 1);
      aS[ip].x += dist01(reng) * 0.1;
      aS[ip].y += dist01(reng) * 0.1;
      aS[ip].z += dist01(reng) * 0.1;
    }
  }
  dfm2::MakeDirectorOrthogonal_RodHair(aS, aP);
  for (int iframe = 0; iframe < 100; ++iframe) {
    dfm2::MakeDirectorOrthogonal_RodHair(aS, aP);
    Solve_RodHair(
        aP, aS, ls_solver,
        stiff_stretch, stiff_bendtwist, 0.0,
        aP0, aS0, aIP_HairRoot);
    viewer.DrawBegin_oldGL();
    myGlutDisplay(aP, aS, aIP_HairRoot);
    viewer.SwapBuffers();
    glfwPollEvents();
    if (glfwWindowShouldClose(viewer.window)) { return; }
  }
}

int main() {
  std::mt19937 reng(std::random_device{}());
  std::uniform_real_distribution<double> dist01(0.0, 1.0);
  dfm2::glfw::CViewer3 viewer(1.5);
  dfm2::glfw::InitGLOld();
  viewer.OpenWindow();
  delfem2::opengl::setSomeLighting();
  // --------------
  while (true) {
    std::vector<dfm2::CVec3d> aP0, aS0;
    std::vector<unsigned int> aIP_HairRoot;
    {
      delfem2::HairDarbouxShape hs{
        30,
        0.2,
        dist01(reng) * 0.3,
        (dist01(reng)+1)*0.1 };
      std::vector<dfm2::CVec3d> ap, as;
      hs.MakeConfigDaboux(ap,as);
      assert(as.size() == ap.size());
      for(unsigned int itr=0;itr<10;++itr) {
        aIP_HairRoot.push_back(aP0.size());
        std::vector<dfm2::CVec3d> ap1 = ap;
        for(auto& p : ap1){
          p.z += itr*0.3;
        }
        aP0.insert(aP0.end(), ap1.begin(), ap1.end());
        aS0.insert(aS0.end(), as.begin(), as.end());
      }
      aIP_HairRoot.push_back(aP0.size());
    }
    // -----------------
    const double stiff_stretch = dist01(reng) + 1.0;
    const double stiff_bendtwist[3] = {
        dist01(reng) + 1.0,
        dist01(reng) + 1.0,
        dist01(reng) + 1.0};
    {
      dfm2::LinearSystemSolver_BlockSparse ls_solver;
      {
        std::vector<unsigned int> psup_ind, psup;
        delfem2::JArray_PSuP_Hair(
            psup_ind, psup,
            aIP_HairRoot);
        ls_solver.Initialize(aP0.size(), 4, psup_ind, psup);
        dfm2::MakeBCFlag_RodHair(
            ls_solver.dof_bcflag,
            aIP_HairRoot);
      }
      Simulation(
          ls_solver, viewer, reng,
          dist01,
          aP0, aS0, stiff_stretch, stiff_bendtwist,
          aIP_HairRoot);
      if( glfwWindowShouldClose(viewer.window)){ break; }
    }
    {
      dfm2::LinearSystemSolver_BlockPentaDiagonal<4> ls_solver;
      ls_solver.Initialize(aP0.size());
      dfm2::MakeBCFlag_RodHair(
          ls_solver.dof_bcflag,
          aIP_HairRoot);
      Simulation(
          ls_solver, viewer, reng,
          dist01,
          aP0, aS0, stiff_stretch, stiff_bendtwist,
          aIP_HairRoot);
      if( glfwWindowShouldClose(viewer.window)){ break; }
    }
  }
  glfwDestroyWindow(viewer.window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
