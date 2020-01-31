/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef DFM2_BVH_H
#define DFM2_BVH_H

#include <stack>
#include <vector>
#include <set>
#include <assert.h>
#include <iostream>

namespace delfem2 {

class CNodeBVH2
{
public:
  int iroot; // -1: root, else: parent
  int ichild[2]; // if ichild[1] == -1, then this is leaf and ichild[0] is the stored number
};

// make BVH topology
int BVH_MakeTreeTopology(std::vector<CNodeBVH2>& aNodeBVH,
                         const int nfael,
                         const std::vector<int>& aElemSur,
                         const std::vector<double>& aElemCenter);
  
int MortonCode_FindSplit(const std::uint32_t* sortedMC,
              unsigned int start,
              unsigned int last);
std::pair<int,int> MortonCode_DeterminRange(const std::uint32_t* sortedMC,
                                  int nMC,
                                  int i);

  
/**
 * @brief compute morton code for 3d coordinates of a point. Each coordinate must be within the range of [0,1]
 * @details defined for "float" and "double"
 */
template <typename REAL>
std::uint32_t MortonCode(REAL x, REAL y, REAL z);

  
/**
 * @details defined for "float" and "double"
 */
template <typename REAL>
void GetSortedMortenCode(std::vector<unsigned int> &aSortedId,
                         std::vector<unsigned int> &aSortedMc,
                         const std::vector<REAL> &aXYZ,
                         const REAL min_xyz[3], const REAL max_xyz[3]);
  
void BVH_TreeTopology_Morton(std::vector<CNodeBVH2>& aNodeBVH,
                             const std::vector<unsigned int>& aSortedId,
                             const std::vector<unsigned int>& aSortedMc);

void Check_BVH(const std::vector<CNodeBVH2>& aNodeBVH,
               unsigned int N);
void Check_MortonCode_RangeSplit(const std::vector<std::uint32_t>& aSortedMc);
void Check_MortonCode_Sort(const std::vector<unsigned int>& aSortedId,
                           const std::vector<std::uint32_t>& aSortedMc,
                           const std::vector<double> aXYZ,
                           const double bbmin[3], const double bbmax[3]);



// -------------------------------------------------------------------
// template functions from here
  
/**
 * @brief build Bounding Box for AABB
 */
template <typename BBOX, typename REAL>
void BVH_BuildBVHGeometry_Mesh(
    std::vector<BBOX>& aBB,
    int ibvh,
    const std::vector<CNodeBVH2>& aNodeBVH,
    double margin,
    const REAL* aXYZ, unsigned int nXYZ,
    const unsigned int* aElem, unsigned int nnoel, unsigned int nElem);

template <typename BBOX, typename REAL>
void BVH_BuildBVHGeometry_Points(
    std::vector<BBOX>& aBB,
    int ibvh,
    const std::vector<CNodeBVH2>& aNodeBVH,
    double margin,
    const REAL* aXYZ, unsigned int nXYZ);

template <typename BBOX>
void BVH_BuildGeometryBranch(
    std::vector<BBOX>& aBB,
    int ibvh,
    const std::vector<delfem2::CNodeBVH2>& aNodeBVH);

template <typename BBOX>
void BuildBoundingBoxesBVH_Dynamic(int ibvh,
                                   double dt,
                                   const std::vector<double>& aXYZ,
                                   const std::vector<double>& aUVW,
                                   const std::vector<unsigned int>& aTri,
                                   const std::vector<CNodeBVH2>& aNodeBVH,
                                   std::vector<BBOX>& aBB);
  
template <typename BBOX>
void BVH_GetIndElem_IncludePoint(std::vector<int>& aIndElem,
                                 //
                                 double px, double py, double pz,
                                 int ibvh,
                                 const std::vector<CNodeBVH2>& aBVH,
                                 const std::vector<BBOX>& aBB);
  
// potential maximum distance of the nearest point
template <typename BBOX>
void BVH_Range_DistToNearestPoint(double& min, double& max,
                                  //
                                  double px, double py, double pz,
                                  int ibvh,
                                  const std::vector<delfem2::CNodeBVH2>& aBVH,
                                  const std::vector<BBOX>& aBB);
  
template <typename BBOX>
void  BVH_GetIndElem_IntersectRay(std::vector<int>& aIndElem,
                                  //
                                  const double src[3], const double dir[3],
                                  int ibvh,
                                  const std::vector<delfem2::CNodeBVH2>& aBVH,
                                  const std::vector<BBOX>& aBB);
  
  
template <typename BBOX>
void BVH_GetIndElem_IntersectLine(std::vector<int>& aIndElem,
                                  //
                                  const double src[3], const double dir[3],
                                  int ibvh,
                                  const std::vector<delfem2::CNodeBVH2>& aBVH,
                                  const std::vector<BBOX>& aBB);
  
template <typename BBOX>
void BVH_GetIndElem_InsideRange(std::vector<int>& aIndElem,
                                //
                                double min, double max,
                                double px, double py, double pz,
                                int ibvh,
                                const std::vector<delfem2::CNodeBVH2>& aBVH,
                                const std::vector<BBOX>& aBB);
  
} // end namespace delfem2


// --------------------------------------------------------------

/**
 * @brief build Bounding Box for AABB
 */
template <typename BBOX, typename REAL>
void delfem2::BVH_BuildBVHGeometry_Mesh(
    std::vector<BBOX>& aBB,
    int ibvh,
    const std::vector<delfem2::CNodeBVH2>& aNodeBVH,
    double margin,
    const REAL* aXYZ, unsigned int nXYZ,
    const unsigned int* aElem, unsigned int nnoel, unsigned int nElem)
{
  aBB.resize( aNodeBVH.size() );
  assert( ibvh < (int)aNodeBVH.size() );
  const int ichild0 = aNodeBVH[ibvh].ichild[0];
  const int ichild1 = aNodeBVH[ibvh].ichild[1];
  if( ichild1 == -1 ){ // leaf node
    assert( ichild0 >= 0 );
    const unsigned int ielem = ichild0;
    assert( ielem < nElem );
    BBOX& bb = aBB[ibvh];
    bb.Set_Inactive();
    for(unsigned int inoel=0;inoel<nnoel;++inoel){
      const unsigned int ino0 = aElem[ielem*nnoel+inoel];
      bb.AddPoint(aXYZ[ino0*3+0], aXYZ[ino0*3+1], aXYZ[ino0*3+2], margin);
    }
    return;
  }
  // branch node is the bounding volume of child nodes
  assert( aNodeBVH[ichild0].iroot == ibvh );
  assert( aNodeBVH[ichild1].iroot == ibvh );
  BVH_BuildBVHGeometry_Mesh(aBB, ichild0,aNodeBVH, margin,aXYZ,nXYZ,aElem,nnoel,nElem);
  BVH_BuildBVHGeometry_Mesh(aBB, ichild1,aNodeBVH, margin,aXYZ,nXYZ,aElem,nnoel,nElem);
  BBOX& bb = aBB[ibvh];
  bb  = aBB[ichild0];
  bb += aBB[ichild1];
  return;
}

template <typename BBOX, typename REAL>
void delfem2::BVH_BuildBVHGeometry_Points(
    std::vector<BBOX>& aBB,
    int ibvh,
    const std::vector<delfem2::CNodeBVH2>& aNodeBVH,
    double margin,
    const REAL* aXYZ, unsigned int nXYZ)
{
  aBB.resize( aNodeBVH.size() );
  assert( ibvh < (int)aNodeBVH.size() );
  const int ichild0 = aNodeBVH[ibvh].ichild[0];
  const int ichild1 = aNodeBVH[ibvh].ichild[1];
  if( ichild1 == -1 ){ // leaf node
    assert( ichild0 >= 0 );
    const unsigned int ip = ichild0;
    assert( ip < nXYZ );
    BBOX& bb = aBB[ibvh];
    return;
  }
  // branch node is the bounding volume of child nodes
  assert( aNodeBVH[ichild0].iroot == ibvh );
  assert( aNodeBVH[ichild1].iroot == ibvh );
  BVH_BuildBVHGeometry_Points(aBB,  ichild0,aNodeBVH, margin, aXYZ,nXYZ);
  BVH_BuildBVHGeometry_Points(aBB,  ichild1,aNodeBVH, margin, aXYZ,nXYZ);
  BBOX& bb = aBB[ibvh];
  bb  = aBB[ichild0];
  bb += aBB[ichild1];
  return;
}

template <typename BBOX>
void delfem2::BVH_BuildGeometryBranch(
    std::vector<BBOX>& aBB,
    int ibvh,
    const std::vector<delfem2::CNodeBVH2>& aNodeBVH)
{
  aBB.resize( aNodeBVH.size() );
  assert( ibvh < (int)aNodeBVH.size() );
  const int ichild0 = aNodeBVH[ibvh].ichild[0];
  const int ichild1 = aNodeBVH[ibvh].ichild[1];
  if( ichild1 == -1 ){ // leaf node
    return;
  }
  // branch node is the bounding volume of child nodes
  assert( aNodeBVH[ichild0].iroot == ibvh );
  assert( aNodeBVH[ichild1].iroot == ibvh );
  BVH_BuildGeometryBranch(aBB, ichild0,aNodeBVH);
  BVH_BuildGeometryBranch(aBB, ichild1,aNodeBVH);
  BBOX& bb = aBB[ibvh];
  bb  = aBB[ichild0];
  bb += aBB[ichild1];
  return;
}

/*
// build Bounding Box for AABB
template <typename BBOX>
void BuildBoundingBoxesBVH
(int ibvh,
 double delta,
 const std::vector<double>& aXYZ,
 const std::vector<unsigned int>& aElemInd,
 const std::vector<unsigned int>& aElem,
 const std::vector<CNodeBVH>& aNodeBVH,
 std::vector<BBOX>& aBB)
{
  aBB.resize( aNodeBVH.size() );
  assert( ibvh < aNodeBVH.size() );
  int ichild0 = aNodeBVH[ibvh].ichild[0];
  int ichild1 = aNodeBVH[ibvh].ichild[1];
  if( ichild1 == -1 ){ // leaf node
    const int ielem = ichild0;
    assert( ielem < aElemInd.size()-1 );
    T& bb = aBB[ibvh];
    bb.is_active = false;
    for(int iip=aElemInd[ielem];iip<aElemInd[ielem+1];++iip){
      const int ino0 = aElem[iip];
      bb.AddPoint(aXYZ[ino0*3+0], aXYZ[ino0*3+1], aXYZ[ino0*3+2], delta*0.5);
    }
    return;
  }
  // branch node is the bounding volume of child nodes
  assert( aNodeBVH[ichild0].iroot == ibvh );
  assert( aNodeBVH[ichild1].iroot == ibvh );
  BuildBoundingBoxesBVH(ichild0,delta, aXYZ,aElemInd,aElem,aNodeBVH,aBB);
  BuildBoundingBoxesBVH(ichild1,delta, aXYZ,aElemInd,aElem,aNodeBVH,aBB);
  T& bb = aBB[ibvh];
  bb.is_active = false;
  bb  = aBB[ichild0];
  bb += aBB[ichild1];
  return;
}
 */

template <typename BBOX>
void delfem2::BuildBoundingBoxesBVH_Dynamic
(int ibvh,
 double dt,
 const std::vector<double>& aXYZ,
 const std::vector<double>& aUVW,
 const std::vector<unsigned int>& aTri,
 const std::vector<delfem2::CNodeBVH2>& aNodeBVH,
 std::vector<BBOX>& aBB)
{
  double eps = 1.0e-10;
  assert( ibvh < aNodeBVH.size() );
  int ichild0 = aNodeBVH[ibvh].ichild[0];
  int ichild1 = aNodeBVH[ibvh].ichild[1];
  if( ichild1 == -1 ){ // leaf
    const int itri = ichild0;
    assert( itri < aTri.size() );
    const int ino0 = aTri[itri*3+0];
    const int ino1 = aTri[itri*3+1];
    const int ino2 = aTri[itri*3+2];
    BBOX& bb = aBB[ibvh];
    bb.bbmin[0] = +1; bb.bbmax[0] = -1;
    bb.AddPoint(aXYZ[ino0*3+0],aXYZ[ino0*3+1],aXYZ[ino0*3+2], eps);
    bb.AddPoint(aXYZ[ino1*3+0],aXYZ[ino1*3+1],aXYZ[ino1*3+2], eps);
    bb.AddPoint(aXYZ[ino2*3+0],aXYZ[ino2*3+1],aXYZ[ino2*3+2], eps);
    bb.AddPoint(aXYZ[ino0*3+0]+dt*aUVW[ino0*3+0], aXYZ[ino0*3+1]+dt*aUVW[ino0*3+1], aXYZ[ino0*3+2]+dt*aUVW[ino0*3+2], eps);
    bb.AddPoint(aXYZ[ino1*3+0]+dt*aUVW[ino1*3+0], aXYZ[ino1*3+1]+dt*aUVW[ino1*3+1], aXYZ[ino1*3+2]+dt*aUVW[ino1*3+2], eps);
    bb.AddPoint(aXYZ[ino2*3+0]+dt*aUVW[ino2*3+0], aXYZ[ino2*3+1]+dt*aUVW[ino2*3+1], aXYZ[ino2*3+2]+dt*aUVW[ino2*3+2], eps);
    return;
  }
  // internal node,内部ノードは子ノードのBounding Volume
  assert( aNodeBVH[ichild0].iroot == ibvh );
  assert( aNodeBVH[ichild1].iroot == ibvh );
  BuildBoundingBoxesBVH_Dynamic(ichild0,dt, aXYZ,aUVW,aTri,aNodeBVH,aBB);
  BuildBoundingBoxesBVH_Dynamic(ichild1,dt, aXYZ,aUVW,aTri,aNodeBVH,aBB);
  BBOX& bb = aBB[ibvh];
  bb.bbmin[0] = +1; bb.bbmax[0] = -1;
  bb  = aBB[ichild0];
  bb += aBB[ichild1];
  return;
}

// -------------------------------------------------


template <typename BBOX>
void delfem2::BVH_GetIndElem_IncludePoint
(std::vector<int>& aIndElem,
 //
 double px, double py, double pz,
 int ibvh,
 const std::vector<delfem2::CNodeBVH2>& aBVH,
 const std::vector<BBOX>& aBB)
{
  if( !aBB[ibvh].isInclude_Point(px,py,pz) ){ return; }
  const int ichild0 = aBVH[ibvh].ichild[0];
  const int ichild1 = aBVH[ibvh].ichild[1];
  if( ichild1 == -1 ){ // leaf
    aIndElem.push_back(ichild0);
    return;
  }
  BVH_GetIndElem_IncludePoint(aIndElem, px,py,pz, ichild0,  aBVH,aBB);
  BVH_GetIndElem_IncludePoint(aIndElem, px,py,pz, ichild1,  aBVH,aBB);
}


// potential maximum distance of the nearest point
template <typename BBOX>
void delfem2::BVH_Range_DistToNearestPoint
(double& min, double& max,
 //
 double px, double py, double pz,
 int ibvh,
 const std::vector<delfem2::CNodeBVH2>& aBVH,
 const std::vector<BBOX>& aBB)
{
  double min0, max0;
  aBB[ibvh].Range_DistToPoint(min0,max0, px,py,pz);
  assert( min0 >= 0 && max0 >= min0 );
  //
  if( max>=0 && min0>max ){ return; }
  const int ichild0 = aBVH[ibvh].ichild[0];
  const int ichild1 = aBVH[ibvh].ichild[1];
  if( ichild1 == -1 ){ // leaf
    if( max<0 ){
      max = max0;
      min = min0;
      return;
    }
    if( max0 < max ){ max = max0; }
    if( min0 < min ){ min = min0; }
    return;
  }
  //
  BVH_Range_DistToNearestPoint(min,max, px,py,pz, ichild0,aBVH,aBB);
  BVH_Range_DistToNearestPoint(min,max, px,py,pz, ichild1,aBVH,aBB);
}

template <typename BBOX>
void delfem2::BVH_GetIndElem_InsideRange
(std::vector<int>& aIndElem,
 //
 double min, double max,
 double px, double py, double pz,
 int ibvh,
 const std::vector<delfem2::CNodeBVH2>& aBVH,
 const std::vector<BBOX>& aBB)
{
  assert( min < max );
  {
    double min0, max0;
    aBB[ibvh].Range_DistToPoint(min0,max0, px,py,pz);
    if( max0<min ){ return; }
    if( min0>max ){ return; }
  }
  const int ichild0 = aBVH[ibvh].ichild[0];
  const int ichild1 = aBVH[ibvh].ichild[1];
  if( ichild1 == -1 ){ // leaf
    assert( ichild0 >= 0 && ichild0 < aBB.size() );
    aIndElem.push_back(ichild0);
    return;
  }
  /////
  BVH_GetIndElem_InsideRange(aIndElem, min,max, px,py,pz, ichild0,aBVH,aBB);
  BVH_GetIndElem_InsideRange(aIndElem, min,max, px,py,pz, ichild1,aBVH,aBB);
}

template <typename BBOX>
void delfem2::BVH_GetIndElem_IntersectRay
(std::vector<int>& aIndElem,
 //
 const double src[3], const double dir[3],
 int ibvh,
 const std::vector<delfem2::CNodeBVH2>& aBVH,
 const std::vector<BBOX>& aBB)
{
  assert( ibvh >= 0 && ibvh < (int)aBVH.size() );
  bool is_intersect = aBB[ibvh].IsIntersectRay(src,dir);
  if( !is_intersect ) return;
  //
  const int ichild0 = aBVH[ibvh].ichild[0];
  const int ichild1 = aBVH[ibvh].ichild[1];
  if( ichild1 == -1 ){ // leaf
    assert( ichild0 >= 0 && ichild0 < (int)aBB.size() );
    aIndElem.push_back(ichild0);
    return;
  }
  BVH_GetIndElem_IntersectRay(aIndElem, src,dir, ichild0,aBVH,aBB);
  BVH_GetIndElem_IntersectRay(aIndElem, src,dir, ichild1,aBVH,aBB);
}

template <typename BBOX>
void delfem2::BVH_GetIndElem_IntersectLine
(std::vector<int>& aIndElem,
 //
 const double src[3], const double dir[3],
 int ibvh,
 const std::vector<delfem2::CNodeBVH2>& aBVH,
 const std::vector<BBOX>& aBB)
{
  assert( ibvh >= 0 && ibvh < (int)aBVH.size() );
  bool is_intersect = aBB[ibvh].IsIntersectLine(src,dir);
  if( !is_intersect ) return;
  //
  const int ichild0 = aBVH[ibvh].ichild[0];
  const int ichild1 = aBVH[ibvh].ichild[1];
  if( ichild1 == -1 ){ // leaf
    assert( ichild0 >= 0 && ichild0 < aBB.size() );
    aIndElem.push_back(ichild0);
    return;
  }
  BVH_GetIndElem_IntersectLine(aIndElem, src,dir, ichild0,aBVH,aBB);
  BVH_GetIndElem_IntersectLine(aIndElem, src,dir, ichild1,aBVH,aBB);
}
  



#endif
