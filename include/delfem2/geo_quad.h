/*
* Copyright (c) 2019 Nobuyuki Umetani
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/


#ifndef DFM2_GEO_QUAD_H
#define DFM2_GEO_QUAD_H

#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

#include "delfem2/vec3.h"
#include "delfem2/dfm2_inline.h"

#define NEARLY_ZERO 1.e-16

namespace delfem2 {

template<typename T>
DFM2_INLINE CVec3<T> Nearst_Origin3_Quad3(
  double &s0,
  double &s1,
  const CVec3<T> &q0,
  const CVec3<T> &q1,
  const CVec3<T> &q2,
  const CVec3<T> &q3);

template<typename T>
bool intersection_Point_Quad(
    CVec3<T> &psec,
    double &s0,
    double &s1,
    const CVec3<T> &src,
    const CVec3<T> &dir,
    const CVec3<T> &q0,
    const CVec3<T> &q1,
    const CVec3<T> &q2,
    const CVec3<T> &q3);

template<typename T>
void iteration_intersection_Line_Quad(
    double &t0, double &t1,
    const CVec3<T> &src,
    const CVec3<T> &u,
    const CVec3<T> &v,
    const CVec3<T> &q0,
    const CVec3<T> &q1,
    const CVec3<T> &q2,
    const CVec3<T> &q3);

} // end namespace delfem2

#ifndef DFM2_STATIC_LIBRARY
#  include "delfem2/geo_quad.cpp"
#endif

#endif // DFM2_GEO_NEAREST3_H
