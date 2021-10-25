//
// Created by Nobuyuki Umetani on 2021-10-08.
//

#ifndef DRAWER_POLYLINE_H_
#define DRAWER_POLYLINE_H_

#include "delfem2/opengl/new/drawer_sphere.h"
#include "delfem2/vec3.h"
#include "delfem2/geo3_v23m34q.h"

namespace delfem2::opengl {

class Drawer_Polyline {
 public:
  void InitGL() {
    {
      sphere.Compile();
      std::vector<float> vtx_xyz;
      std::vector<unsigned int> tri_vtx;
      delfem2::MeshTri3D_Sphere(
          vtx_xyz, tri_vtx,
          1.f, 32, 32);
      sphere.Initialize(vtx_xyz, 3, tri_vtx, GL_TRIANGLES);
    }
    {
      cylinder.Compile();
      std::vector<float> vtx_xyz;
      std::vector<unsigned int> tri_vtx;
      delfem2::MeshTri3D_CylinderOpen(
          vtx_xyz, tri_vtx,
          1.f, 1.f, 32, 1);
      cylinder.Initialize(vtx_xyz, 3, tri_vtx, GL_TRIANGLES);
    }
  }

  template <typename T0>
  void Draw(
      const T0 *vtx_pos,
      unsigned int nvtx,
      unsigned int ndim,
      const float mat_projection[16],
      const float mat_modelview[16]) const {
    namespace dfm2 = ::delfem2;
    for(unsigned int ivtx=0;ivtx<nvtx;++ivtx) {
      float p[3] = { static_cast<float>(vtx_pos[ivtx*ndim+0]), static_cast<float>(vtx_pos[ivtx*ndim+1]), 0.f };
      if( ndim == 3 ){ p[2] = vtx_pos[ivtx*3+2];  }
      const dfm2::CMat4f mmv = dfm2::CMat4f(mat_modelview)
          * dfm2::CMat4f::Translation(p)
          * dfm2::CMat4f::AffineScale(radius_sphere);
      sphere.Draw(mat_projection, mmv.data());
    }
    if( nvtx < 2 ){ return; }
    for(unsigned int ivtx=0;ivtx<nvtx-1;++ivtx) {
      unsigned int jvtx = ivtx+1;
      dfm2::CVec3f p0_(vtx_pos[ivtx*ndim+0], vtx_pos[ivtx*ndim+1], 0.f );
      dfm2::CVec3f p1_(vtx_pos[jvtx*ndim+0], vtx_pos[jvtx*ndim+1], 0.f );
      if( ndim == 3 ){
        p0_.z = vtx_pos[ivtx*ndim+2];
        p1_.z = vtx_pos[jvtx*ndim+2];
      }
      dfm2::CMat3f R3 = Mat3_MinimumRotation(
          dfm2::CVec3f{0,1,0},
          p1_-p0_);
      dfm2::CMat4f R4 = dfm2::CMat4f::Mat3(R3.data());
      const dfm2::CMat4f mmv = dfm2::CMat4f(mat_modelview)
          * dfm2::CMat4f::Translation(((p0_+p1_)*0.5).data())
          * R4
          * dfm2::CMat4f::ScaleXYZ(radius_cylinder,(p1_-p0_).norm(), radius_cylinder);
      cylinder.Draw(mat_projection, mmv.data());
    }
  }

  template <typename VEC>
  void Draw(
      const std::vector<VEC> vtx_pos,
      unsigned int ndim,
      const float mat_projection[16],
      const float mat_modelview[16]) const {
    namespace dfm2 = ::delfem2;
    const unsigned int nvtx = vtx_pos.size();
    for(unsigned int ivtx=0;ivtx<nvtx;++ivtx) {
      float p[3] = { static_cast<float>(vtx_pos[ivtx][0]), static_cast<float>(vtx_pos[ivtx][1]), 0.f };
      if( ndim == 3 ){ p[2] = vtx_pos[ivtx][2];  }
      const dfm2::CMat4f mmv = dfm2::CMat4f(mat_modelview)
          * dfm2::CMat4f::Translation(p)
          * dfm2::CMat4f::AffineScale(radius_sphere);
      sphere.Draw(mat_projection, mmv.data());
    }
    if( nvtx < 2 ){ return; }
    for(unsigned int ivtx=0;ivtx<nvtx-1;++ivtx) {
      unsigned int jvtx = ivtx+1;
      dfm2::CVec3f p0_(vtx_pos[ivtx][0], vtx_pos[ivtx][1], 0.f );
      dfm2::CVec3f p1_(vtx_pos[jvtx][0], vtx_pos[jvtx][1], 0.f );
      if( ndim == 3 ){
        p0_.z = vtx_pos[ivtx][2];
        p1_.z = vtx_pos[jvtx][2];
      }
      dfm2::CMat3f R3 = Mat3_MinimumRotation(
          dfm2::CVec3f{0,1,0},
          p1_-p0_);
      dfm2::CMat4f R4 = dfm2::CMat4f::Mat3(R3.data());
      const dfm2::CMat4f mmv = dfm2::CMat4f(mat_modelview)
          * dfm2::CMat4f::Translation(((p0_+p1_)*0.5).data())
          * R4
          * dfm2::CMat4f::ScaleXYZ(radius_cylinder,(p1_-p0_).norm(), radius_cylinder);
      cylinder.Draw(mat_projection, mmv.data());
    }
  }

 public:
  CShader_Mesh sphere;
  CShader_Mesh cylinder;
  float radius_sphere = 1.f;
  float radius_cylinder = 1.f;
};

}


#endif //DRAWER_POLYLINE_H_
