/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */


#ifndef DEPTH_H
#define DEPTH_H

#include <stdio.h>
#include <vector>
#include "delfem2/opengl/glnew_mshcolor.h"


class CInputDepth
{
public:
  virtual void Draw() const = 0;
};

class CGPUSampler
{
public:
  CGPUSampler(){
    nResX = 0;
    nResY = 0;
    id_tex_color = 0;
    pointSize = 3;
    isDrawTex = true;
    // ------------------
    color.resize(4);  color[0] = 1;  color[1] = 0;  color[2] = 0;  color[3] = 1;
    bgcolor.resize(4);  bgcolor[0] = 1;  bgcolor[1] = 1;  bgcolor[2] = 1;  bgcolor[3] = 1;
    lengrid = 0.01;
    origin[  0]=0; origin[  1]=0; origin[  2]=0;
    z_axis[  0]=0; z_axis[  1]=0; z_axis[  2]=1;
    x_axis[0]=1; x_axis[1]=0; x_axis[2]=0;
    draw_len_axis = 1.0;
  }
  CGPUSampler(int nw, int nh, std::string sFormatPixelColor, bool isDepth){
    this->Init(nw,nh,sFormatPixelColor,isDepth);
  }
  void Init(int nw, int nh, std::string sFormatPixelColor, bool isDepth);

  void InitGL();
//  void LoadTex();
  // --------------
  void Draw(float mP[16], float mMV[16]) const;
  std::vector<double> MinMaxXYZ() const {
    std::vector<double> mm(6);
    mm[0] = +1;
    mm[1] = -1;
    return mm;
  }
  void Matrix_MVP(float mMV[16], float p[16]) const;
  std::vector<double> getGPos(int ix, int iy) const;
  // ------------
  void SetColor(double r, double g, double b);
  void SaveDepthCSV(const std::string& path) const;
  void SetCoord(double elen, double depth_max,
                const std::vector<double>& orgPrj,
                const std::vector<double>& dirPrj,
                const std::vector<double>& dirWidth);
  void Start();
  void End();
  void SetZeroToDepth(){ for(unsigned int i=0;i<aZ.size();++i){ aZ[i] = 0.0; } }
public:
  unsigned int id_tex_color;
  unsigned int id_tex_depth;
  unsigned int id_framebuffer;
  // -----
  int nResX;
  int nResY;
  double lengrid;
  double z_range;
  double z_axis[3];
  double x_axis[3];
  double origin[3];
  // ------
  std::vector<float> aZ;
  // ------
  std::vector<double> bgcolor;
  std::vector<double> color;
  double draw_len_axis;
  unsigned int pointSize;
  bool isDrawTex;

  CShader_LineMesh shdr0;
  CShader_TriMesh_Tex shdr1;
  CShader_Points shdr2;
private:
  int view[4];
};

#endif /* depth_hpp */
