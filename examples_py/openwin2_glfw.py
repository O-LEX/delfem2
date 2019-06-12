from OpenGL.GL import *

import sys
sys.path.append("../module_py")
import dfm2
import dfm2.glfw


def draw_func():
  glEnable(GL_LIGHTING)
  msh.draw()

msh = dfm2.Mesh()
msh.read("../test_inputs/bunny_2k.ply")
msh.scale_xyz(0.03)

win = dfm2.glfw.WindowGLFW(1.0,winsize=(400,300))
win.list_func_draw.append(draw_func)
dfm2.setSomeLighting()
win.draw_loop()
