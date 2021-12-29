//
// Created by Nobuyuki Umetani on 2021/12/01.
//

#include <random>

#include "gtest/gtest.h"

#include "delfem2/geo_curve_cubic.h"
#include "delfem2/geo_polyline.h"

TEST(bezier_cubic, test0) {
  namespace dfm2 = delfem2;
  std::mt19937 rndeng(std::random_device{}());
  std::uniform_real_distribution<double> dist_01(0, 1);
  for (int itr = 0; itr < 1000; ++itr) {
    dfm2::CVec2d p0, p1, p2, p3;
    p0 = dfm2::CVec2d(dist_01(rndeng), dist_01(rndeng));
    p1 = dfm2::CVec2d(dist_01(rndeng), dist_01(rndeng));
    p2 = dfm2::CVec2d(dist_01(rndeng), dist_01(rndeng));
    p3 = dfm2::CVec2d(dist_01(rndeng), dist_01(rndeng));
    std::vector<dfm2::CVec2d> polyline;
    dfm2::Polyline_BezierCubic(polyline, 1000, p0, p1, p2, p3);
    {
      double v0 = dfm2::Length_Polyline<dfm2::CVec2d>(polyline);
      double v2 = dfm2::Length_CubicBezierCurve_Quadrature<dfm2::CVec2d>(p0, p1, p2, p3, 5);
      EXPECT_NEAR(v0, v2, 0.07);
      double v3 = dfm2::Length_CubicBezierCurve_QuadratureSubdivision<dfm2::CVec2d,11>(
        p0, p1, p2, p3, 1.0e-3, 12);
      EXPECT_NEAR(v0, v3, 4.e-3);
    }
    {
      double a0 = dfm2::Area_CubicBezierCurve2(p0, p1, p2, p3);
      double a1 = dfm2::Area_Polyline2(polyline);
      EXPECT_NEAR(a0, a1, 1.0e-5);
    }
    {  // nearest
      dfm2::CVec2d scr(0.5, 0.5);
      const auto[i0, l0] = dfm2::FindNearestPointInPolyline(polyline, scr);
      const dfm2::CVec2d v0 = dfm2::PositionInPolyline(polyline, i0, l0);
      //
      const double t1 = dfm2::Nearest_CubicBezierCurve<dfm2::CVec2d>(
        scr,
        p0, p1, p2, p3, 50, 3);
      const dfm2::CVec2d v1 = dfm2::PointOnCubicBezierCurve(t1, p0, p1, p2, p3);
      EXPECT_NEAR((v0 - scr).norm(), (v1-scr).norm(), 1.2e-2);
      //
      const double t2 = dfm2::Nearest_CubicBezierCurve_Strum<dfm2::CVec2d>(
        scr,
        p0, p1, p2, p3, 16);
      const dfm2::CVec2d v2 = dfm2::PointOnCubicBezierCurve(t2, p0, p1, p2, p3);
      EXPECT_NEAR((v0 - scr).norm(), (v2-scr).norm(), 1.0e-6);
    }
    {  // bb
      auto bb0 = dfm2::AABB_CubicBezierCurve<2>(p0, p1, p2, p3);
      auto bb1 = dfm2::AABB_Polyline<2>(polyline);
      EXPECT_NEAR(bb0[0], bb1[0], 1.0e-3);
      EXPECT_NEAR(bb0[1], bb1[1], 1.0e-3);
      EXPECT_NEAR(bb0[2], bb1[2], 1.0e-3);
      EXPECT_NEAR(bb0[3], bb1[3], 1.0e-3);
    }
    {  // bezier split
      std::vector<dfm2::CVec2d> poly0;
      {
        std::array<dfm2::CVec2d,8> r = dfm2::Split_CubicBezierCurve(p0,p1,p2,p3, 0.3);
        delfem2::Polyline_BezierCubic(
          poly0, 4,
          r[0], r[1], r[2], r[3]);
        std::vector<dfm2::CVec2d> poly1;
        delfem2::Polyline_BezierCubic(
          poly1, 8,
          r[4], r[5], r[6], r[7]);
        poly0.resize(poly0.size() - 1);
        poly0.insert(poly0.end(), poly1.begin(), poly1.end());
      }
      std::vector<dfm2::CVec2d> poly2;
      delfem2::Polyline_BezierCubic(
        poly2, 11,
        p0, p1, p2,p3);
      EXPECT_EQ(poly0.size(),poly2.size());
      for(unsigned int ip=0;ip<poly0.size();++ip){
        EXPECT_LT((poly0[ip]-poly2[ip]).norm(),1.0e-10);
      }
    }
  }
}
