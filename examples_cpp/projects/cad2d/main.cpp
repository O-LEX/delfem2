#include <iostream>
#include <sstream>
#include <math.h>

#include <complex>
#include <set>
#include <stack>

#if defined(__APPLE__) && defined(__MACH__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "delfem2/dyntri_v2.h"
#include "delfem2/cad2d.h"

#include "delfem2/funcs_glut.h"
#include "delfem2/funcs_gl.h"
#include "delfem2/v23q_gl.h"

#ifndef M_PI
#define M_PI 3.141592653589793
#endif



///////////////////////////////////////////////////////////////////////////////////////////////


CGlutWindowManager win;
const double view_height = 2.0;
bool is_animation = false;
int imode_draw = 0;

CCad2D cad;

////////////////////////////////////////////////////////////////////////////////////////////////

void myGlutDisplay(void)
{
//  	::glClearColor(0.2f, 0.7f, 0.7f ,1.0f);
	::glClearColor(1.0f, 1.0f, 1.0f ,1.0f);
//	::glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  ::glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
  ::glEnable(GL_DEPTH_TEST);
	::glEnable(GL_POLYGON_OFFSET_FILL );
	::glPolygonOffset( 1.1f, 4.0f );
  
  win.SetGL_Camera();
  
  cad.Draw();
  
  ::glColor3d(0,0,0);
  ShowFPS();
  
  ::glutSwapBuffers();
}

void myGlutIdle(){
  if( is_animation ){
  }
  ::glutPostRedisplay();
}


void myGlutResize(int w, int h)
{
  glViewport(0, 0, w, h);
	::glutPostRedisplay();
}

void myGlutSpecial(int Key, int x, int y)
{
  win.glutSpecial(Key,x,y);
}

void myGlutMotion( int x, int y ){
  win.glutMotion(x,y);
  if( win.imodifier != 0){ return; }
  float mMV[16]; glGetFloatv(GL_MODELVIEW_MATRIX, mMV);
  float mPj[16]; glGetFloatv(GL_PROJECTION_MATRIX, mPj);
  CVector2 sp0(win.mouse_x-win.dx, win.mouse_y-win.dy);
  CVector2 sp1(win.mouse_x, win.mouse_y);
  const CVector3 src_pick0 = screenUnProjection(CVector3(sp0.x,sp0.y, 0.0), mMV,mPj);
  const CVector3 src_pick1 = screenUnProjection(CVector3(sp1.x,sp1.y, 0.0), mMV,mPj);
  const CVector3 dir_pick = screenUnProjectionDirection(CVector3(0.0,  0, -1.0 ), mMV,mPj);
  /////
  cad.Motion(src_pick0.stlvec(), src_pick1.stlvec(), dir_pick.stlvec());
}

void myGlutMouse(int button, int state, int x, int y)
{
  win.glutMouse(button,state,x,y);
  if( win.imodifier == GLUT_ACTIVE_SHIFT || win.imodifier == GLUT_ACTIVE_ALT ) return;
  float mMV[16]; glGetFloatv(GL_MODELVIEW_MATRIX, mMV);
  float mPj[16]; glGetFloatv(GL_PROJECTION_MATRIX, mPj);
  CVector2 sp0(win.mouse_x, win.mouse_y);
  const CVector3 src_pick = screenUnProjection(CVector3(sp0.x,sp0.y, 0.0), mMV,mPj);
  const CVector3 dir_pick = screenUnProjectionDirection(CVector3(0.0,  0, -1.0 ), mMV,mPj);
  if( state == GLUT_DOWN ){
    cad.Mouse(0,1,0,
               src_pick.stlvec(),
               dir_pick.stlvec(),
               view_height);
  }
  if( state == GLUT_UP ){
    cad.Mouse(0,0,0,
              src_pick.stlvec(),
              dir_pick.stlvec(),
              view_height);
  }
}

void myGlutKeyboard(unsigned char Key, int x, int y)
{
	switch(Key)
	{
    case 'q':
    case 'Q':
    case '\033':
      exit(0);  /* '\033' ? ESC ? ASCII ??? */
      break;
    case 'a':
    {
      is_animation = !is_animation;
      break;
    }
    case 'd':
    {
      imode_draw = (imode_draw+1)%3;
      break;
    }
  }
  ::glutPostRedisplay();
}

int main(int argc,char* argv[])
{
  glutInit(&argc, argv);
  
	// Initialize GLUT window 3D
  glutInitWindowPosition(200,200);
	glutInitWindowSize(400, 300);
// 	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH);
  glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH|GLUT_STENCIL);
  glutCreateWindow("3D View");
	glutDisplayFunc(myGlutDisplay);
	glutIdleFunc(myGlutIdle);
	glutReshapeFunc(myGlutResize);
	glutMotionFunc(myGlutMotion);
	glutMouseFunc(myGlutMouse);
	glutKeyboardFunc(myGlutKeyboard);
	glutSpecialFunc(myGlutSpecial);
  
  ////////////////////////
  win.camera.view_height = view_height;
//  win.camera.camera_rot_mode = CAMERA_ROT_TBALL;
  win.camera.camera_rot_mode = CAMERA_ROT_YTOP;
//    win.camera.camera_rot_mode = CAMERA_ROT_ZTOP;
  
  setSomeLighting();
  
  const double poly[8] = {-1,-1, +1,-1, +1,+1, -1,+1};
  cad.AddPolygon(std::vector<double>(poly,poly+8));
  
  glutMainLoop();
	return 0;
}


