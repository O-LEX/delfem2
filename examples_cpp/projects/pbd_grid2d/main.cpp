#include <iostream>
#include <math.h>

#if defined(__APPLE__) && defined(__MACH__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "delfem2/msh.h"
#include "delfem2/mshtopo.h"
#include "delfem2/vec2.h"
#include "delfem2/funcs_gl.h"
#include "Griddeformer.h"



/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////


void ConstraintProjection2D_Rigid
(std::vector<double>& aXYt,
 double stiffness,
 const std::vector<int>& clstr_ind,
 const std::vector<int>& clstr,
 const std::vector<double>& aXY0)
{
  const int nclstr = clstr_ind.size()-1;
  for(int iclstr=0;iclstr<nclstr;++iclstr){
    CVector2 pc(0, 0), qc(0, 0);
    for (int iip=clstr_ind[iclstr];iip<clstr_ind[iclstr+1]; iip++){
      const int ip = clstr[iip];
      qc += CVector2(aXY0[ip*2+0],aXY0[ip*2+1]);
      pc += CVector2(aXYt[ip*2+0],aXYt[ip*2+1]);
    }
    qc /= (clstr_ind[iclstr+1]-clstr_ind[iclstr]);
    pc /= (clstr_ind[iclstr+1]-clstr_ind[iclstr]);
    
    double A[4] = { 0, 0, 0, 0 };
    for (int iip=clstr_ind[iclstr];iip<clstr_ind[iclstr+1]; iip++){
      const int ip = clstr[iip];
      const CVector2 dq = CVector2(aXY0[ip*2+0],aXY0[ip*2+1])-qc; // undeform
      const CVector2 dp = CVector2(aXYt[ip*2+0],aXYt[ip*2+1])-pc; // deform
      A[0*2+0] += dp[0]*dq[0];
      A[0*2+1] += dp[0]*dq[1];
      A[1*2+0] += dp[1]*dq[0];
      A[1*2+1] += dp[1]*dq[1];
    }
    double R[4]; RotationalComponentOfMatrix2(R,A);
    
    for (int iip=clstr_ind[iclstr];iip<clstr_ind[iclstr+1]; iip++){
      const int ip = clstr[iip];
      CVector2 dq = CVector2(aXY0[ip*2+0],aXY0[ip*2+1])-qc;
      CVector2 pg = pc+matVec(R, dq); // goal position
      CVector2 pg2 = stiffness*pg+(1-stiffness)*CVector2(aXYt[ip*2+0],aXYt[ip*2+1]);
      aXYt[ip*2+0] = pg2.x;
      aXYt[ip*2+1] = pg2.y;
    }
  }
}

void stepTime
(std::vector<double>& aXY1,
 std::vector<double>& aUV1,
 std::vector<double>& aTmp,
 double dt,
 int nitr,
 const std::vector<int>& clstr_ind,
 const std::vector<int>& clstr,
 const std::vector<int>& aQuad,
 const std::vector<double>& aXY0)
{
  const int ndof = aXY0.size();
  for (int idof=0; idof<ndof; idof++){
    aTmp[idof] = aXY1[idof]+aUV1[idof]*dt;
  }
  { // deform
    for (int itr=0; itr<nitr; itr++){
      ConstraintProjection2D_Rigid(aTmp,
                 0.5, clstr_ind, clstr, aXY0);
    }
  }
  for (int idof=0; idof<ndof; ++idof){
    aUV1[idof] = (aTmp[idof]-aXY1[idof])*(1.0/dt);
  }
  for (int idof=0; idof<ndof; idof++){
    aXY1[idof] = aTmp[idof];
  }
}

const int nX = 5;
const int nY = 5;
std::vector<double> aXY0;
std::vector<double> aXY1;
std::vector<double> aUV1;
std::vector<double> aXYt;
std::vector<int> aQuad;
std::vector<int> clstr_ind, clstr;

void myGlutDisplay(void)
{
  //	::glClearColor(0.2f, 0.7f, 0.7f ,1.0f);
	::glClearColor(1.0f, 1.0f, 1.0f ,1.0f);
  ::glClearStencil(0);
	::glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
	::glEnable(GL_DEPTH_TEST);
  
	::glEnable(GL_POLYGON_OFFSET_FILL );
  ::glPolygonOffset(1.1f, 4.0f);

  {
    int viewport[8];
    glGetIntegerv(GL_VIEWPORT, viewport);
    double w = (double)viewport[2];
    double h = (double)viewport[3];
    double asp = w/h;
    const double win_size = 10.0;
    ::glMatrixMode(GL_PROJECTION);
    ::glLoadIdentity();
    ::glOrtho(-asp*win_size, +asp*win_size, -win_size, +win_size, -10, +10);
    ::glMatrixMode(GL_MODELVIEW);
    ::glLoadIdentity();
  }
  
  DrawMeshQuad2D_Edge(aXY1.data(), aXY1.size()/2,
                      aQuad.data(), aQuad.size()/4);
    
  ::glutSwapBuffers();
}

void myGlutIdle(){
  double dt = 0.1;
  static double t = 0;
  t += dt;
  aXY1[0] = 5*sin(t*0.5);
  aXY1[1] = 0;
  stepTime(aXY1, aUV1, aXYt,
           dt, 1,
           clstr_ind, clstr,
           aQuad, aXY0);
  ::glutPostRedisplay();
}


void myGlutResize(int w, int h)
{
  ::glViewport(0,0,w,h);
	::glutPostRedisplay();
}

void myGlutMotion(int x, int y)
{
	::glutPostRedisplay();
}

void myGlutMouse(int button, int state, int x, int y)
{
	::glutPostRedisplay();
}


void myGlutKeyboard(unsigned char key, int x, int y)
{
  switch (key) {
    case 'q':
    case 'Q':
    case '\033':  /* '\033' ÇÕ ESC ÇÃ ASCII ÉRÅ[Éh */
      exit(0);
      break;
    case 'a':
      break;
  }
}

int main(int argc,char* argv[])
{
  glutInit(&argc, argv);
  
	// Initialize GLUT window 3D
  glutInitWindowPosition(200,200);
	glutInitWindowSize(400, 300);
 	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH|GLUT_STENCIL);
  glutCreateWindow("3D View");
	glutDisplayFunc(myGlutDisplay);
	glutIdleFunc(myGlutIdle);
	glutReshapeFunc(myGlutResize);
	glutMotionFunc(myGlutMotion);
	glutMouseFunc(myGlutMouse);
  glutKeyboardFunc(myGlutKeyboard);
  
  ////////////////////////
  
  MeshQuad2D_Grid(aXY0, aQuad, nX, nY);
  aXY1 = aXY0;
  aXYt = aXY0;
  aUV1.resize(aXY0.size());
  
  std::vector<int> psup_ind, psup;
  JaggedArray_MeshOneRingNeighborhood(psup_ind, psup,
                                      aQuad.data(), aQuad.size()/4, 4,
                                      aXY0.size()/2);
//  Print_IndexedArray(psup_ind, psup);
  JaggedArray_AddDiagonal(clstr_ind, clstr,
                          psup_ind, psup);
  JaggedArray_Print(clstr_ind, clstr);
  
  
  glutMainLoop();
	return 0;
}

