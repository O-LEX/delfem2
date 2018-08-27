from OpenGL.GL import *
import glfw
from .dfm2_gl import Camera
from ._dfm2 import *

class WindowManagerGLFW:
  def __init__(self,view_height):
    self.camera = Camera(view_height)
    self.modifier = 0
    self.mouse_x = 0.0
    self.mouse_y = 0.0
    self.button = -1
    self.isClose = False

  def keyinput(self,win_glfw,key,scancode,action,mods):
    if key == glfw.KEY_Q and action == glfw.PRESS:
      self.isClose = True
    if key == glfw.KEY_PAGE_UP:
      self.camera.scale *= 1.03
    if key == glfw.KEY_PAGE_DOWN:
      self.camera.scale /= 1.03

  def mouse(self,win_glfw,btn,action,mods):
    (win_w, win_h) = glfw.get_window_size(win_glfw)
    (x, y) = glfw.get_cursor_pos(win_glfw)
    self.mouse_x = (2.0 * x - win_w) / win_w
    self.mouse_y = (win_h - 2.0 * y) / win_h
    self.modifier = mods
    if action == glfw.PRESS:
      self.button = btn
    elif action == glfw.RELEASE:
      self.button = -1

  def motion(self,win_glfw, x, y):
    (win_w, win_h) = glfw.get_window_size(win_glfw)
    if self.button == glfw.MOUSE_BUTTON_LEFT:
      if self.modifier == glfw.MOD_ALT:  # shift
        self.mouse_x, self.mouse_y = self.camera.rotation(x, y, self.mouse_x, self.mouse_y, win_w, win_h)
      if self.modifier == glfw.MOD_SHIFT:
        self.mouse_x, self.mouse_y = self.camera.translation(x, y, self.mouse_x, self.mouse_y, win_w, win_h)


class WindowGLFW:
  def __init__(self,view_height,winsize=(400,300)):
    glfw.init()
    self.win = glfw.create_window(winsize[0], winsize[1], '3D Window', None, None)
    glfw.make_context_current(self.win)
    ###
    self.wm = WindowManagerGLFW(view_height)
    self.draw_func = None
    glEnable(GL_DEPTH_TEST)

  def draw_loop(self,render):
    glfw.set_mouse_button_callback(self.win, self.mouse)
    glfw.set_cursor_pos_callback(self.win, self.motion)
    glfw.set_key_callback(self.win, self.keyinput)
    while not glfw.window_should_close(self.win):
      glClearColor(1, 1, 1, 1)
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
      self.wm.camera.set_gl_camera()
      render()
      glfw.swap_buffers(self.win)
      glfw.poll_events()
      if self.wm.isClose:
        break
    glfw.destroy_window(self.win)
    glfw.terminate()

  def mouse(self, win0,btn,action,mods):
    self.wm.mouse(win0,btn,action,mods)

  def motion(self,win0,x,y):
    self.wm.motion(win0,x,y)

  def keyinput(self,win0,key,scancode,action,mods):
    self.wm.keyinput(win0,key,scancode,action,mods)

def winDraw3d(obj,winsize=(400,300)):
  window = WindowGLFW(1.0,winsize);
  setSomeLighting()  
  glEnable(GL_POLYGON_OFFSET_FILL );
  glPolygonOffset( 1.1, 4.0 );
  window.draw_loop(obj.draw)