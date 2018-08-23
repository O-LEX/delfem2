#ifndef VOXEL_H
#define VOXEL_H

#include <vector>

#include "vec3.h"

int Adj_Grid
(int ivox_picked, int iface_picked,
 int ndivx, int ndivy, int ndivz);

class CCubeGrid
{
public:
  CCubeGrid(){
    this->ivx = 0;
    this->ivy = 0;
    this->ivz = 0;
    is_active = true;
  }
  CCubeGrid(int i, int j, int k){
    this->ivx = i;
    this->ivy = j;
    this->ivz = k;
    is_active = true;
  }
  bool operator<(const CCubeGrid& rhs) const
  {
    if( this->ivx != rhs.ivx ){ return this->ivx < rhs.ivx; }
    if( this->ivy != rhs.ivy ){ return this->ivy < rhs.ivy; }
    if( this->ivz != rhs.ivz ){ return this->ivz < rhs.ivz; }
    return false;
  }
public:
  int ivx, ivy, ivz;
  bool is_active;
};

void Draw_CubeGrid(bool is_picked, int iface_picked,
                   double elen, const CVector3& org,
                   const CCubeGrid& cube);
void Pick_CubeGrid(int& icube_pic, int& iface_pic,
                   const CVector3& src_pic, const CVector3& dir_pic,
                   double elen,
                   const CVector3& org,
                   const std::vector<CCubeGrid>& aCube);
void Adj_CubeGrid(int& ivx, int& ivy, int& ivz,
                  int ivox, int iface,
                  std::vector<CCubeGrid>& aCube);
void Add_CubeGrid(std::vector<CCubeGrid>& aVox,
                  int ivx1, int ivy1, int ivz1);
void Del_CubeGrid(std::vector<CCubeGrid>& aCube,
                  int i1, int j1, int k1);
void AABB_CubeGrid(int aabb[6],
                   const std::vector<CCubeGrid>& aCube);

////////////////////////////////////////////////////////////////////////////////////

class CVoxelGrid
{
public:
  void Init_AABB(const int aabb[6]){
    ndivx = aabb[1]-aabb[0];
    ndivy = aabb[3]-aabb[2];
    ndivz = aabb[5]-aabb[4];
    iorgx = aabb[0];
    iorgy = aabb[2];
    iorgz = aabb[4];
    const int ngridvx = ndivx*ndivy*ndivz;
    aIsVox.assign(ngridvx,0);
  }
  void Set(int ivx, int ivy, int ivz, int isVox){
    int igvx = ivx-iorgx;
    int igvy = ivy-iorgy;
    int igvz = ivz-iorgz;
    std::cout << "  " << igvx << " " << igvy << " " << igvz << std::endl;
    if( igvx<0 || igvx>ndivx ){ return; }
    if( igvy<0 || igvy>ndivy ){ return; }
    if( igvz<0 || igvz>ndivz ){ return; }
    int igv = igvx*(ndivy*ndivz)+igvy*ndivz+igvz;
    aIsVox[igv] = isVox;
  }
public:
  int ndivx, ndivy, ndivz;
  int iorgx, iorgy, iorgz;
  std::vector<int> aIsVox;
};

#endif