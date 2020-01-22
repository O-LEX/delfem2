/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */


#ifndef DFM2_MAT3_H
#define DFM2_MAT3_H

#include <vector>
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits> // using NaN Check

#define NEARLY_ZERO 1.e-16

// -----------------------------

namespace delfem2 {

template <typename T>
void MatMat3(T* UL,
             const T* U, const T* L);
  
template <typename T>
void MatMatTrans3(T* ULUt,
                  const T* UL, const T* U);
  
template <typename T>
void MatTransMat3(T* C,
                  const T* A, const T* B);

template <typename T>
T Det_Mat3(const T U[9]);
  
/**
 * @brief compute eigen value & vector for symmmetric matrix
 * @details
 * sm[6] = (M_00,M_11,M_22,M_12,M_20,M_01)
 * M = ULU^T
 * u[9] = (U_00,U_01,U_02, U_10,U_11,U_12, U_20,U_21,U_22)
 */
bool eigenSym3(double u[9], double l[3],
               const double sm[6],
               int nitr);

void svd3(double U[9], double G[3], double V[9],
          const double m[9],
          int nitr);

void GetRotPolarDecomp(double R[9],
                       const double am[9],
                       int nitr);

  // ------------------------------------------------

void MatVec4(double v[4],
             const double A[16],
             const double x[4]);
void Affine3D(double y0[3],
              const double a[16],
              const double x0[3]);
void SetAffine_Scale(double A[16],
                     double s);
void SetAffine_Trans(double A[16],
                     double dx, double dy, double dz);
void SetAffine_Rotate_Rodriguez(double A[16],
                                double dx, double dy, double dz);


// --------------------------------

class CMat3; // this pre-definition is needed for following functions
CMat3 operator+ (const CMat3& lhs, const CMat3& rhs);
CMat3 operator- (const CMat3& lhs, const CMat3& rhs);
CMat3 operator* (double d, const CMat3& rhs);
CMat3 operator* (const CMat3& m, double d);
CMat3 operator* (const CMat3& lhs, const CMat3& rhs);
CMat3 operator/ (const CMat3& m, double d);
std::ostream &operator<<(std::ostream &output, const CMat3& m);
std::istream &operator>>(std::istream &output, CMat3& m);

static bool myIsNAN_Matrix3(double d){ return !(d > d-1); }

/**
 * @brief class of 3x3 matrix
 * @todo use template for this class 
 */
class CMat3
{
public:
  CMat3();
  CMat3(const double s);
  CMat3(double v00, double v01, double v02,
           double v10, double v11, double v12,
           double v20, double v21, double v22);
  CMat3(double x, double y, double z);
  CMat3(const double m[9]);
  // ---------------
  void GetElements(double m[9]) const { for(unsigned int i=0;i<9;i++){ m[i]=mat[i]; } }
  void AffineMatrixTrans(double m[16]) const {
    m[0*4+0] = mat[0];  m[1*4+0] = mat[1];  m[2*4+0] = mat[2];  m[3*4+0] = 0;
    m[0*4+1] = mat[3];  m[1*4+1] = mat[4];  m[2*4+1] = mat[5];  m[3*4+1] = 0;
    m[0*4+2] = mat[6];  m[1*4+2] = mat[7];  m[2*4+2] = mat[8];  m[3*4+2] = 0;
    m[0*4+3] = 0;       m[1*4+3] = 0;       m[2*4+3] = 0;       m[3*4+3] = 1;
  }
  double Get(int i, int j) const {
    return mat[i*3+j];
  }
  // ---------------
//  CVector3 MatVec(const CVector3& vec0) const;
  void MatVec(const double vec0[], double vec1[]) const;
  void MatVecTrans(const double vec0[], double vec1[]) const;
//  CVector3 MatVecTrans(const CVector3& vec0) const;
  CMat3 MatMat(const CMat3& mat0) const;
  CMat3 MatMatTrans(const CMat3& mat0) const;
  // ----------------
  CMat3 Sym() const{
    CMat3 m;
    for(unsigned int i=0;i<3;i++){
    for(unsigned int j=0;j<3;j++){
      m.mat[i*3+j] = (mat[i*3+j]+mat[j*3+i])*0.5;
    }
    }
    return m;
  }
  inline const CMat3 operator-() const{ return (*this)*(-1.0); }
  inline const CMat3 operator+() const{ return (*this); }
  inline CMat3& operator+=(const CMat3& rhs){
    for(unsigned int i=0;i<9;i++){ mat[i] += rhs.mat[i]; }
		return *this;
	}  
  inline CMat3& operator-=(const CMat3& rhs){
    for(unsigned int i=0;i<9;i++){ mat[i] -= rhs.mat[i]; }
		return *this;
	}    
	inline CMat3& operator*=(double d){
    for(unsigned int i=0;i<9;i++){ mat[i] *= d; }
		return *this;
	}
	inline CMat3& operator/=(double d){
    double invd = 1.0/d;
    for(unsigned int i=0;i<9;i++){ mat[i] *= invd; }
		return *this;
	}
  inline double operator[](int i) const{
    return this->mat[i];
  }  
  inline double& operator()(int i, int j){
    return this->mat[i*3+j];
  }
  // -------------------------
  CMat3 Inverse() const;
  // -------------------------
  void SetInverse();
  void SetSymetric(const double sm[6]);
  void SetZero();
  void SetRandom();
  void SetRotMatrix_Cartesian(const double vec[]);
  void SetRotMatrix_Cartesian(double x, double y, double z);
  void SetRotMatrix_Rodrigues(const double vec[]);
  void SetRotMatrix_CRV(const double crv[]);
  void SetRotMatrix_Quaternion(const double quat[]);
  void SetRotMatrix_BryantAngle(double rx, double ry, double rz);
  void SetIdentity(double scale = 1);
  // ------------------------
  void GetCRV_RotMatrix(double crv[]) const;
  void GetQuat_RotMatrix(double quat[]) const;
  // ------------------------
  CMat3 Trans() const {
    CMat3 m;
    m.mat[0] = mat[0]; m.mat[1] = mat[3]; m.mat[2] = mat[6];
    m.mat[3] = mat[1]; m.mat[4] = mat[4]; m.mat[5] = mat[7];
    m.mat[6] = mat[2]; m.mat[7] = mat[5]; m.mat[8] = mat[8];    
    return m;
  }
  bool isNaN() const{
    double s=mat[0]+mat[1]+mat[2]+mat[3]+mat[4]+mat[5]+mat[6]+mat[7]+mat[8];
    return myIsNAN_Matrix3( s ) != 0;
  }
  double Det() const {
    return
    + mat[0]*mat[4]*mat[8] + mat[3]*mat[7]*mat[2] + mat[6]*mat[1]*mat[5]
    - mat[0]*mat[7]*mat[5] - mat[6]*mat[4]*mat[2] - mat[3]*mat[1]*mat[8];
  }
  double SqNorm_Frobenius() const {
    double s = 0.0;
    for(int i=0;i<9;++i){
      s += mat[i]*mat[i];
    }
    return s;
  }
  double Trace() const {
    return mat[0]+mat[4]+mat[8];
  }
  double SecondInvarint() const {
    const CMat3& m2 = (*this)*(*this);
    const double tr = this->Trace();
    return 0.5*(tr*tr-m2.Trace());
  }
  void Print() const {
    std::cout << mat[0] << " " << mat[1] << " " << mat[2] << std::endl;
    std::cout << mat[3] << " " << mat[4] << " " << mat[5] << std::endl;
    std::cout << mat[6] << " " << mat[7] << " " << mat[8] << std::endl;
  }
  void PolerDecomp(CMat3& R, int nitr) const{
    GetRotPolarDecomp(R.mat,
                      mat, nitr);
  }
  // --------------------
  // static functions
  static CMat3 Identity(double scale = 1){
    CMat3 m;
    m.SetIdentity(scale);
    return m;
  }
  static CMat3 Zero(){
    CMat3 m;
    m.SetZero();
    return m;
  }

public:
  double mat[9];  
};


  
}

#endif
