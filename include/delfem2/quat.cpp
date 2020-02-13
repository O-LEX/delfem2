/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */


#include <cmath>
#include "delfem2/quat.h"

#ifndef M_PI
#  define M_PI 3.141592
#endif

namespace dfm2 = delfem2;

// ----------------------------------

template <typename T>
void dfm2::Normalize_Quat(T q[])
{
  const double len = sqrt(q[0]*q[0]+q[1]*q[1]+q[2]*q[2]+q[3]*q[3]);
  double invlen = 1.0/len;
  q[0] *= invlen;
  q[1] *= invlen;
  q[2] *= invlen;
  q[3] *= invlen;
}
template void dfm2::Normalize_Quat(float q[]);
template void dfm2::Normalize_Quat(double q[]);

// -----------------------------------

template <typename T>
void dfm2::Quat_Identity(T q[4]){
  q[0] = 1;
  q[1] = 0;
  q[2] = 0;
  q[3] = 0;
}
template void dfm2::Quat_Identity(float q[4]);
template void dfm2::Quat_Identity(double q[4]);

// ----------------------------------

template <typename T>
void dfm2::QuatVec(
    T vo[3],
    const T q[4],
    const T vi[3])
{
  const T x2 = q[1] * q[1] * 2;
  const T y2 = q[2] * q[2] * 2;
  const T z2 = q[3] * q[3] * 2;
  const T xy = q[1] * q[2] * 2;
  const T yz = q[2] * q[3] * 2;
  const T zx = q[3] * q[1] * 2;
  const T xw = q[1] * q[0] * 2;
  const T yw = q[2] * q[0] * 2;
  const T zw = q[3] * q[0] * 2;
  vo[0] = (1.0 - y2 - z2)*vi[0] + (xy + zw      )*vi[1] + (zx - yw      )*vi[2];
  vo[1] = (xy - zw      )*vi[0] + (1.0 - z2 - x2)*vi[1] + (yz + xw      )*vi[2];
  vo[2] = (zx + yw      )*vi[0] + (yz - xw      )*vi[1] + (1.0 - x2 - y2)*vi[2];
}
template void dfm2::QuatVec(float vo[3], const float q[4], const float vi[3]);
template void dfm2::QuatVec(double vo[3], const double q[4], const double vi[3]);

// -------------------------------------

// multiply two quaternion
void dfm2::QuatQuat(double r[], const double p[], const double q[])
{
  r[0] = p[0] * q[0] - p[1] * q[1] - p[2] * q[2] - p[3] * q[3];
  r[1] = p[0] * q[1] + p[1] * q[0] + p[2] * q[3] - p[3] * q[2];
  r[2] = p[0] * q[2] - p[1] * q[3] + p[2] * q[0] + p[3] * q[1];
  r[3] = p[0] * q[3] + p[1] * q[2] - p[2] * q[1] + p[3] * q[0];
}



// transform vector with conjugate of quaternion
void dfm2::QuatConjVec(double vo[], const double q[], const double vi[])
{
  double x2 = q[1] * q[1] * 2.0;
  double y2 = q[2] * q[2] * 2.0;
  double z2 = q[3] * q[3] * 2.0;
  double xy = q[1] * q[2] * 2.0;
  double yz = q[2] * q[3] * 2.0;
  double zx = q[3] * q[1] * 2.0;
  double xw = q[1] * q[0] * 2.0;
  double yw = q[2] * q[0] * 2.0;
  double zw = q[3] * q[0] * 2.0;
  
  vo[0] = (1.0 - y2 - z2)*vi[0] + (xy - zw      )*vi[1] + (zx + yw      )*vi[2];
  vo[1] = (xy + zw      )*vi[0] + (1.0 - z2 - x2)*vi[1] + (yz - xw      )*vi[2];
  vo[2] = (zx - yw      )*vi[0] + (yz + xw      )*vi[1] + (1.0 - x2 - y2)*vi[2];
}

// copy quaternion
void dfm2::QuatCopy(double r[], const double p[])
{
  r[0] = p[0];
  r[1] = p[1];
  r[2] = p[2];
  r[3] = p[3];
}

void dfm2::Quat_Bryant
 (double q[4],
  double x, double y, double z)
{
  const double dqx[4] = { cos(x*0.5), sin(x*0.5), 0.0, 0.0 };
  const double dqy[4] = { cos(y*0.5), 0.0, sin(y*0.5), 0.0 };
  const double dqz[4] = { cos(z*0.5), 0.0, 0.0, sin(z*0.5) };
  double qtmp_yx[4]; dfm2::QuatQuat(qtmp_yx, dqy, dqx);
  dfm2::QuatQuat(q, dqz, qtmp_yx);
}

void dfm2::Mat4_Quat
 (double r[], const double q[])
{
  double x2 = q[1] * q[1] * 2.0;
  double y2 = q[2] * q[2] * 2.0;
  double z2 = q[3] * q[3] * 2.0;
  double xy = q[1] * q[2] * 2.0;
  double yz = q[2] * q[3] * 2.0;
  double zx = q[3] * q[1] * 2.0;
  double xw = q[1] * q[0] * 2.0;
  double yw = q[2] * q[0] * 2.0;
  double zw = q[3] * q[0] * 2.0;
  
  r[ 0] = 1.0 - y2 - z2;
  r[ 1] = xy - zw;
  r[ 2] = zx + yw;
  r[ 4] = xy + zw;
  r[ 5] = 1.0 - z2 - x2;
  r[ 6] = yz - xw;
  r[ 8] = zx - yw;
  r[ 9] = yz + xw;
  r[10] = 1.0 - x2 - y2;
  r[ 3] = r[ 7] = r[11] = r[12] = r[13] = r[14] = 0.0;
  r[15] = 1.0;
}

// return transpose matrix of Mat4_Quat
void Mat4_QuatConj(double r[], const double q[])
{
  double x2 = q[1] * q[1] * 2.0;
  double y2 = q[2] * q[2] * 2.0;
  double z2 = q[3] * q[3] * 2.0;
  double xy = q[1] * q[2] * 2.0;
  double yz = q[2] * q[3] * 2.0;
  double zx = q[3] * q[1] * 2.0;
  double xw = q[1] * q[0] * 2.0;
  double yw = q[2] * q[0] * 2.0;
  double zw = q[3] * q[0] * 2.0;
  
  r[ 0] = 1.0 - y2 - z2;
  r[ 1] = xy + zw;
  r[ 2] = zx - yw;
  r[ 4] = xy - zw;
  r[ 5] = 1.0 - z2 - x2;
  r[ 6] = yz + xw;
  r[ 8] = zx + yw;
  r[ 9] = yz - xw;
  r[10] = 1.0 - x2 - y2;
  r[ 3] = r[ 7] = r[11] = r[12] = r[13] = r[14] = 0.0;
  r[15] = 1.0;
}

void dfm2::Mat4_ScaleRotTrans
(double m[16],
 double scale,
 const double quat[4],
 const double trans[3])
{
  dfm2::Mat4_Quat(m, quat);
  for(int i=0;i<3;++i){
    for(int j=0;j<3;++j){
      m[i*4+j] *= scale;
    }
  }
  m[0*4+3] = trans[0];
  m[1*4+3] = trans[1];
  m[2*4+3] = trans[2];
}

void dfm2::MatMat4(double m01[16],
             const double m0[16], const double m1[16])
{
  for(int i=0;i<4;++i){
    for(int j=0;j<4;++j){
      m01[i*4+j] = m0[i*4+0]*m1[0*4+j] + m0[i*4+1]*m1[1*4+j] + m0[i*4+2]*m1[2*4+j] + m0[i*4+3]*m1[3*4+j];
    }
  }
}

void dfm2::Copy_Mat4(double m1[16], const double m0[16])
{
  for(int i=0;i<16;++i){ m1[i] = m0[i]; }
}


//////////////////////////////////////////////////////////////////////
// �\�z/����
//////////////////////////////////////////////////////////////////////

/*
CQuat::CQuat(const CVector3D& axis ){
	AxisToQuat( axis );
}

CQuat::CQuat(double real, const CVector3D& vector){
	this->real = real;
	this->vector = vector;
}

CQuat::CQuat(const CVector3D& a_vector, const CVector3D& b_vector){
	VectorTrans(a_vector, b_vector);
}
 */

// functions for Quaternion above
// --------------------------------------------------------------------------------------------
// CQuat from here

template class dfm2::CQuat<double>;
template class dfm2::CQuat<float>;

namespace delfem2{
  
template <typename T>
CQuat<T> operator + (const CQuat<T>& lhs, const CQuat<T>& rhs)
{
  return CQuat<T>(lhs.q[0]+rhs.q[0],
                        lhs.q[1]+rhs.q[1],
                        lhs.q[2]+rhs.q[2],
                        lhs.q[3]+rhs.q[3]);
}
template CQuat<double> operator + (const CQuat<double>& lhs, const CQuat<double>& rhs);
template CQuat<float> operator + (const CQuat<float>& lhs, const CQuat<float>& rhs);
  
  
} // end namespace delfem2

/*
CQuat operator-(const CQuat& lhs, const CQuat& rhs){
    CQuat temp = lhs;
	temp -= rhs;
    return temp;
}

CQuat operator*(const CQuat& quat, double d){
    CQuat temp = quat;
	temp *= d;
    return temp;
}

CQuat operator*(double d, const CQuat& quat){
    CQuat temp = quat;
    temp *= d;
	return temp;
}

CQuat operator*(const CQuat& lhs, const CQuat& rhs){
    CQuat temp = lhs;
    temp *= rhs;
	return temp;
}

CVector3D Rotate(const CQuat& quat, const CVector3D& vec){
	CQuat tmp(0, vec);
	tmp = quat *  (tmp * quat.GetConjugate());  
//	tmp = quat.GetConjugate() *  tmp * quat ;   
	return tmp.GetVector();
}

CVector3D UnRotate(const CQuat& quat, const CVector3D& vec){
	CQuat tmp(0, vec);
	tmp = (quat.GetConjugate() *  tmp) * quat ; 
//	tmp = quat *  tmp * quat.GetConjugate();    
	return tmp.GetVector();
}


//////////////////////////////////////////////////////////////////////
// �����o�֐��̃t�����h�֐�
//////////////////////////////////////////////////////////////////////


bool operator==(const CQuat& lhs, const CQuat& rhs){
	if( fabs(lhs.real - rhs.real) < NEARLY_ZERO && lhs == rhs )	return true;
	else return false;
}

bool operator!=(const CQuat& lhs, const CQuat& rhs){
	if( lhs == rhs ) return false;
	else return true;
}

//////////////////////////////////////////////////////////////////////
// �����o�֐��̔�t�����h�֐��@
//////////////////////////////////////////////////////////////////////

CQuat& CQuat::operator = (const CQuat& rhs){
	if( this != &rhs ){
		real = rhs.real;
		vector = rhs.vector;
	}
	return *this;
}

CQuat& CQuat::operator *= (const CQuat& rhs){
	const double last_real = real;
	real = real * rhs.real - Dot( vector, rhs.vector );
	vector = ( (rhs.vector * last_real) + (vector * rhs.real) ) + Cross( vector, rhs.vector );
	return *this;
}

CQuat& CQuat::operator *= (double d){
	real *= d;
	vector *= d;
	return *this;
}

CQuat& CQuat::operator += (const CQuat& rhs){
	real += rhs.real;
	vector += rhs.vector;
	return *this;
}

CQuat& CQuat::operator -= (const CQuat& rhs){
	real -= rhs.real;
	vector -= rhs.vector;
	return *this;
}

CQuat CQuat::GetInverse() const
{
  double invslen = 1.0/SquareLength();
	CQuat tmp(real*invslen, vector*(-invslen));
	return tmp;
}

CQuat CQuat::GetConjugate() const{
	CQuat tmp(real, vector*(-1.0));
	return tmp;
}

void CQuat::RotMatrix44(double* m) const
{
  
	double vx=vector.x, vy=vector.y, vz=vector.z;
  
	m[ 0] = 1.0 - 2.0 * ( vy * vy + vz * vz );
	m[ 4] = 2.0 * ( vx * vy - vz * real );
	m[ 8] = 2.0 * ( vz * vx + vy * real );
	m[12] = 0;
  
	m[ 1] = 2.0 * ( vx * vy + vz * real );
	m[ 5] = 1.0 - 2.0 * ( vz * vz + vx * vx );
	m[ 9] = 2.0 * ( vy * vz - vx * real );
	m[13] = 0.0;
  
	m[ 2] = 2.0 * ( vz * vx - vy * real );
	m[ 6] = 2.0 * ( vy * vz + vx * real );
	m[10] = 1.0 - 2.0 * ( vy * vy + vx * vx );    
	m[14] = 0.0;  
  
  m[ 3] = 0.0;
  m[ 7] = 0.0;
  m[11] = 0.0;
  m[15] = 1.0;  
}



void CQuat::RotMatrix33(double* m) const
{
	double vx=vector.x, vy=vector.y, vz=vector.z;

	m[0] = 1.0 - 2.0 * ( vy * vy + vz * vz );
	m[1] = 2.0 * ( vx * vy - vz * real );
	m[2] = 2.0 * ( vz * vx + vy * real );

	m[3] = 2.0 * ( vx * vy + vz * real );
	m[4] = 1.0 - 2.0 * ( vz * vz + vx * vx );
	m[5] = 2.0 * ( vy * vz - vx * real );

	m[6] = 2.0 * ( vz * vx - vy * real );
	m[7] = 2.0 * ( vy * vz + vx * real );
	m[8] = 1.0 - 2.0 * ( vy * vy + vx * vx );
}

void CQuat::Normalize()
{	
	const double len = ( real * real + vector.DLength() );
	real /= len;
	vector /= len;
}

void CQuat::AxisToQuat(const CVector3D &axis )
{
	const double phi = axis.Length();
	if( phi < 1.0e-30 ){
		vector = CVector3D(0,0,0);
		real = 1.0;
		return;
	}
	vector = axis;
	vector.SetNormalizedVector();
	vector *= sin( phi * 0.5 );
	real = cos( phi * 0.5 );
}

void CQuat::VectorTrans(const CVector3D& a_vector, const CVector3D& b_vector){
	vector = Cross(a_vector, b_vector);
	if( vector.DLength() < NEARLY_ZERO ){
		real = 1.0;
		vector.SetZero();
		return;
	}
	vector.SetNormalizedVector();
	double cos_theta = Dot(a_vector, b_vector) / ( a_vector.Length() * b_vector.Length() );
	real = sqrt( 0.5*(1+cos_theta) );
	vector *= sqrt( 0.5*(1-cos_theta) );
}

double CQuat::SquareLength() const
{
	return real*real + vector.DLength();
}


double CQuat::Length() const
{
	return sqrt( real*real + vector.DLength() );
}

CQuat SphericalLinearInterp(const CQuat& q0, const CQuat& q1, double t)
{  
  
  double qr = q0.real * q1.real + Dot(q0.vector,q1.vector);
  double ss = 1.0 - qr * qr;
  
  if (ss == 0.0) { return q0; }
  double sp = sqrt(ss);
  double ph = acos(qr);
  double pt = ph * t;
  double t1 = sin(pt) / sp;
  double t0 = sin(ph - pt) / sp;    
  CQuat q;
  q.real = t0*q0.real + t1*q1.real;
  q.vector = t0*q0.vector + t1*q1.vector;
  return q;
}

*/
