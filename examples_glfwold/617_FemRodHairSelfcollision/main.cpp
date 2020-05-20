/*
 * Copyright (c) 2020 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "delfem2/femrod.h"
#include "delfem2/geo3_v23m34q.h"
#include "delfem2/mats.h"
#include "delfem2/srchbi_v3bvh.h"
#include "delfem2/bv.h"

// --------------
#include <GLFW/glfw3.h>
#include "delfem2/opengl/glfw/viewer_glfw.h"
#include "delfem2/opengl/v3q_glold.h"
#include "delfem2/opengl/funcs_glold.h"

namespace dfm2 = delfem2;

// -------------------------------------

void myDraw
(const std::vector<dfm2::CVec3d>& aP,
 const std::vector<dfm2::CVec3d>& aS,
 std::vector<unsigned int>& aIP_HairRoot)
{
  const unsigned int nhair = aIP_HairRoot.size()-1;
  for(unsigned int ihair=0;ihair<nhair;++ihair){
    const unsigned int ips = aIP_HairRoot[ihair];
    const unsigned int ipe = aIP_HairRoot[ihair+1];
    assert( aP.size() == aS.size() );
    ::glDisable(GL_LIGHTING);
    ::glColor3d(1,0,0);
    ::glPointSize(3);
    ::glBegin(GL_POINTS);
    for(unsigned int ip=ips;ip<ipe;++ip){
      ::glVertex3d(aP[ip].x(), aP[ip].y(), aP[ip].z());
    }
    ::glEnd();
    // ------------
    ::glColor3d(0,0,0);
    ::glLineWidth(3);
    ::glBegin(GL_LINES);
    unsigned int ns = ipe-ips-1;
    for(unsigned int is=0;is<ns;++is){
      const unsigned int ip0 = ips+is+0; assert( ip0 < aP.size() );
      const unsigned int ip1 = ips+is+1; assert( ip1 < aP.size() );
      ::glVertex3d(aP[ip0].x(), aP[ip0].y(), aP[ip0].z());
      ::glVertex3d(aP[ip1].x(), aP[ip1].y(), aP[ip1].z());
    }
    ::glEnd();
    // --------------
    ::glBegin(GL_LINES);
    for(unsigned int is=0;is<ns;++is){
      const unsigned int ip0 = ips+is+0; assert( ip0 < aP.size() );
      const unsigned int ip1 = ips+is+1; assert( ip1 < aP.size() );
      dfm2::CVec3d p01 = 0.5*(aP[ip0]+aP[ip1]);
      double l01 = (aP[ip0]-aP[ip1]).Length();
      dfm2::opengl::myGlVertex(p01);
      dfm2::opengl::myGlVertex(p01+(l01*0.5)*aS[is]);
    }
    ::glEnd();
  }

  { // draw rectangle wall
    ::glEnable(GL_LIGHTING);
    ::glBegin(GL_QUADS);
    ::glNormal3d(1.0, 1.2, 0);
    ::glVertex3d(-0.8,-1,-2);
    ::glVertex3d(-1.4,+2,-2);
    ::glVertex3d(-1.4,+2,+2);
    ::glVertex3d(-0.8,-1,+2);
    ::glEnd();
  }

}

class CHairShape{
public:
  unsigned int np;
  double pitch;
  double rad0;
  double dangle;
  double p0[3];
};

void MakeProblemSetting_Spiral
(std::vector<dfm2::CVec3d>& aP0,
 std::vector<dfm2::CVec3d>& aS0,
 std::vector<unsigned int>& aIP_HairRoot,
 const std::vector<CHairShape>& aHairShape)
{
  aIP_HairRoot.assign(1,0);
  aP0.clear();
  aS0.clear();
  for(unsigned int ihair=0;ihair<aHairShape.size();++ihair){
    const unsigned int np = aHairShape[ihair].np;
    const double pitch = aHairShape[ihair].pitch;
    const double dangle = aHairShape[ihair].dangle;
    const double rad0 = aHairShape[ihair].rad0;
    const double* p0 = aHairShape[ihair].p0;
    for(unsigned int ip=0;ip<np;++ip){
      dfm2::CVec3d p = dfm2::CVec3d(p0[0]+ip*pitch, p0[1]+rad0*cos(dangle*ip), p0[2]+rad0*sin(dangle*ip));
      aP0.push_back(p);
    }
    const unsigned int np0 = aIP_HairRoot[ihair];
    for(unsigned int is=0;is<np-1;++is){
      const dfm2::CVec3d v = (aP0[np0+is+1] - aP0[np0+is+0]).Normalize();
      dfm2::CVec3d s(1.3, 1.5, 1.7);
      s = (s-(s*v)*v).Normalize();
      aS0.push_back(s);
    }
    aS0.emplace_back(1,0,0);
    aIP_HairRoot.push_back(aP0.size());
  }
}

double IterateEdgeEdgeIntersection(
    double p[3],
    const dfm2::CVec3d& p0s,
    const dfm2::CVec3d& p0e,
    const dfm2::CVec3d& p1s,
    const dfm2::CVec3d& p1e,
    const dfm2::CVec3d& q0s,
    const dfm2::CVec3d& q0e,
    const dfm2::CVec3d& q1s,
    const dfm2::CVec3d& q1e,
    unsigned int nitr )
{
  dfm2::CVec3d v0;
  for(unsigned int itr=0;itr<nitr;++itr) {
    const double s0 = p[0], t0 = p[1], u0 = p[2];
    v0 =
        + ((1 - s0) * (1 - u0)) * p0s + ((1 - s0) * u0) * p0e + (s0 * (1 - u0)) * p1s + (s0 * u0) * p1e
        - ((1 - t0) * (1 - u0)) * q0s - ((1 - t0) * u0) * q0e - (t0 * (1 - u0)) * q1s - (t0 * u0) * q1e;
    const dfm2::CVec3d ds = -(1 - u0) * p0s - u0 * p0e + (1 - u0) * p1s + u0 * p1e;
    const dfm2::CVec3d dt = +(1 - u0) * q0s + u0 * q0e - (1 - u0) * q1s - u0 * q1e;
    const dfm2::CVec3d du =
        - (1 - s0) * p0s + (1 - s0) * p0e - s0 * p1s + s0 * p1e
        + (1 - t0) * q0s - (1 - t0) * q0e + t0 * q1s - t0 * q1e;
    double A[9] = {
        ds * ds, ds * dt, ds * du,
        dt * ds, dt * dt, dt * du,
        du * ds, du * dt, du * du};
    {
      double eps = (A[0] + A[4] + A[8]) * 1.0e-10 + 1.0e-20;
      A[0] += eps;
      A[4] += eps;
      A[8] += eps;
    }
    dfm2::Inverse_Mat3(A);
    double R[3] = {v0 * ds, v0 * dt, v0 * du};
    double D[3];
    dfm2::MatVec3(D, A, R);
    p[0] -= D[0];
    p[1] -= D[1];
    p[2] -= D[2];
    if (p[0] < 0) { p[0] = 0.0; } else if (p[0] > 1) { p[0] = 1.0; }
    if (p[1] < 0) { p[1] = 0.0; } else if (p[1] > 1) { p[1] = 1.0; }
    if (p[2] < 0) { p[2] = 0.0; } else if (p[2] > 1) { p[2] = 1.0; }
  }
  return v0.Length();
}

void SolveSelfCollisionRod(
    std::vector<dfm2::CVec3d>& aPt,
    std::vector<dfm2::CContactHair>& aCollision,
    const double clearance,
    const std::vector<dfm2::CVec3d>& aP,
    const std::vector<unsigned int>& aIP_HairRoot, // indexes of root point
    bool is_project)
{
  aCollision.clear();
  const unsigned int nr = aP.size();
  assert( aPt.size() == nr );
  std::vector<bool> aIsRod(nr,true);
  for(unsigned int ihair=0;ihair<aIP_HairRoot.size()-1;++ihair){
    unsigned int ip0 = aIP_HairRoot[ihair];
    unsigned int ip1 = aIP_HairRoot[ihair+1]-1;
    aIsRod[ip0] = false;
    aIsRod[ip1] = false;
  }
  std::vector<dfm2::CBV3_Sphere<double>> aBV(nr);
  for(unsigned int ir=0;ir<nr;++ir) {
    if (!aIsRod[ir]) {
      aBV[ir].Set_Inactive();
      continue;
    }
    const dfm2::CVec3d p0s = aP[ir];
    const dfm2::CVec3d p1s = aP[ir + 1];
    const dfm2::CVec3d p0e = aPt[ir];
    const dfm2::CVec3d p1e = aPt[ir + 1];
    aBV[ir].SetPoints4(p0s.p,p1s.p,p0e.p,p1e.p, clearance*0.5);
  }

  for(unsigned int ir=0;ir<nr;++ir){
    if( !aIsRod[ir] ){ continue; }
    const dfm2::CVec3d p0s = aP[ir+0];
    const dfm2::CVec3d p1s = aP[ir+1];
    const dfm2::CVec3d p0e = aPt[ir+0];
    const dfm2::CVec3d p1e = aPt[ir+1];
    for(unsigned int jr=ir+1;jr<nr;++jr) {
      if( !aIsRod[jr] ){ continue; }
      if( ir == jr || ir == jr+1 || ir == jr-1 ){ continue; } // neighbouring rod is always intersecting
      if( !aBV[ir].IsIntersect(aBV[jr]) ) continue;
      // -------
      const dfm2::CVec3d q0s = aP[jr+0];
      const dfm2::CVec3d q1s = aP[jr+1];
      const dfm2::CVec3d q0e = aPt[jr+0];
      const dfm2::CVec3d q1e = aPt[jr+1];
      // collision
      double p[3] = {0.5, 0.5, -0.5};
      double len = IterateEdgeEdgeIntersection(p,
          p0s, p0e, p1s, p1e, q0s, q0e, q1s, q1e, 3);
      if( len >= clearance ){ continue; }
      double s1 = p[0], t1 = p[1];
      dfm2::CVec3d vs = (1-s1)*p0s + s1*p1s - (1-t1)*q0s - t1*q1s;
      dfm2::CContactHair ch{ir + 0, ir + 1, s1,
                            jr + 0, jr + 1, t1,
                            vs.Normalize()};
      aCollision.push_back(ch);
      if( is_project ) {
        dfm2::CVec3d ve = (1 - s1) * p0e + s1 * p1e - (1 - t1) * q0e - t1 * q1e;
        dfm2::CVec3d dv1 = vs.Normalize() * clearance - ve;
        double W = 1.0 / ((1 - s1) * (1 - s1) + s1 * s1 + (1 - t1) * (1 - t1) + t1 * t1);
        aPt[ir + 0] += dv1 * W * (1 - s1);
        aPt[ir + 1] += dv1 * W * s1;
        aPt[jr + 0] -= dv1 * W * (1 - t1);
        aPt[jr + 1] -= dv1 * W * t1;
      }
    } // jr
  } // ir
}




int main(int argc,char* argv[])
{
  dfm2::opengl::CViewer_GLFW viewer;
  viewer.Init_oldGL();
  viewer.nav.camera.view_height = 1.5;
  viewer.nav.camera.camera_rot_mode = delfem2::CCamera<double>::CAMERA_ROT_MODE::TBALL;
  viewer.nav.camera.Rot_Camera(-0.4, -0.1);
  delfem2::opengl::setSomeLighting();
  // -------
  std::vector<dfm2::CVec3d> aP0; // initial position
  std::vector<dfm2::CVec3d> aS0; // initial director vector
  std::vector<unsigned int> aIP_HairRoot; // indexes of root point
  { // make the un-deformed shape of hair
    std::vector<CHairShape> aHairShape;
    double rad0 = 0.0;
    double dangle = 0.0;
    const int nhair = 2;
    for(int ihair=0;ihair<nhair;++ihair){
      const double px = -1.0-ihair*0.2;
      const double py = ihair;
      const double pz = 0.0;
      const CHairShape hs{30, 0.1, rad0, dangle,
                          {px, py, pz } };
      aHairShape.push_back(hs);
    }
    MakeProblemSetting_Spiral(aP0,aS0,aIP_HairRoot,
                              aHairShape);
    assert( aS0.size() == aP0.size() );
  }
  for(int itr=0;itr<10;++itr){ // relax director vectors
    ParallelTransport_RodHair(aP0, aS0, aIP_HairRoot);
  }
  std::vector<int> aBCFlag; // boundary condition
  dfm2::MakeBCFlag_RodHair( // set fixed boundary condition
      aBCFlag,
      aIP_HairRoot);
  assert( aBCFlag.size() == aP0.size()*4 );
  dfm2::CMatrixSparse<double> mats; // sparse matrix
  dfm2::MakeSparseMatrix_RodHair( // make sparse matrix pattern
      mats,
      aIP_HairRoot);
  // -----------------
  std::vector<dfm2::CVec3d> aP = aP0, aS = aS0;
  std::vector<dfm2::CVec3d> aPV (aP0.size(), dfm2::CVec3d(0,0,0)); // velocity
  std::vector<dfm2::CVec3d> aPt = aP; // temporally positions
  std::vector<dfm2::CContactHair> aCollision; // collision in the previous time-step
  double dt = 0.01;
  double mass = 1.0e-2;
  dfm2::CVec3d gravity(0,-10,0);
  const double stiff_stretch = 10000;
  const double stiff_bendtwist[3] = { 2000, 2000, 2000 };
  // -------
  const double clearance = 0.01;
  double time_cur = 0.0;
  while (true)
  {
    time_cur += dt;
    { // set fixed boundary condition
      unsigned int ip0 = aIP_HairRoot[1];
      double z0 = 0.4*sin(2.0*time_cur);
      aP[ip0].p[2] = aP[ip0+1].p[2] = z0;
      aPt[ip0].p[2] = aPt[ip0+1].p[2] = z0;
    }
    for(unsigned int ip=0;ip<aP.size();++ip){
      if( aBCFlag[ip*4+0] !=0 ) { continue; } // this is not fixed boundary
      aPt[ip] = aP[ip] + dt * aPV[ip] + (dt * dt / mass) * gravity;
    }
    SolveSelfCollisionRod(aPt,aCollision,
                          clearance,
                          aP, aIP_HairRoot,
                          false);
    std::cout << "####################" << std::endl;
    for(auto& ch: aCollision){
      std::cout << "     " << ch.ip0 << " " << ch.ip1 << " " << ch.s << std::endl;
      std::cout << "     " << ch.iq0 << " " << ch.iq1 << " " << ch.t << std::endl;
      std::cout << "     " << ch.norm << " " << ch.norm.Length() << std::endl;
    }
//    if( time_cur > 3.3 ){
    {
      aCollision.resize(1);
      dfm2::CContactHair& ch = aCollision[0];
      ch.ip0 = 4;
      ch.ip1 = 5;
      ch.s = 0.5;
      ch.iq0 = 43;
      ch.iq1 = 44;
      ch.t = 0.5;
      ch.norm = dfm2::CVec3d(0,0,1);
      const unsigned int aIP[4] = {ch.ip0, ch.ip1, ch.iq0, ch.iq1};
      const double aW[4] = {1-ch.s, ch.s, -(1-ch.t), -ch.t};
      dfm2::CVec3d a = aW[0]*aP[aIP[0]] + aW[1]*aP[aIP[1]] + aW[2]*aP[aIP[2]] + aW[3]*aP[aIP[3]];
      double r0 = a*ch.norm - clearance;
      if( r0 > 0 ){
        aCollision.clear();
      }
    }

    dfm2::MakeDirectorOrthogonal_RodHair(aS,aPt);
    Solve_RodHairContact(
        aPt, aS, mats,
        stiff_stretch, stiff_bendtwist, mass/(dt*dt),
        aP0, aS0, aBCFlag, aIP_HairRoot,
        clearance, aCollision);
    /*
    SolveSelfCollisionRod(aPt,aCollision,
                          clearance,
                          aP, aIP_HairRoot,
                          false);
    for(auto& ch: aCollision){
      std::cout << "     #" << ch.ip0 << " " << ch.ip1 << " " << ch.s << std::endl;
      std::cout << "     #" << ch.iq0 << " " << ch.iq1 << " " << ch.t << std::endl;
      std::cout << "     #" << ch.norm << " " << ch.norm.Length() << std::endl;
    }
     */
    for(unsigned int ip=0;ip<aP.size();++ip){
      if( aBCFlag[ip*4+0] != 0 ){ continue; }
      aPV[ip] = (aPt[ip] - aP[ip])/dt;
      aP[ip] = aPt[ip];
    }
    // -------------
    viewer.DrawBegin_oldGL();
    myDraw(aP, aS, aIP_HairRoot);
    viewer.DrawEnd_oldGL();
    if (glfwWindowShouldClose(viewer.window)) { goto EXIT; }
  }
EXIT:
  glfwDestroyWindow(viewer.window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
