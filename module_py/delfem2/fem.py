import numpy, os
import OpenGL.GL as gl

from .libdelfem2 import MatrixSquareSparse, PreconditionerILU
from .libdelfem2 import addMasterSlavePattern, matrixSquareSparse_setPattern, matrixSquareSparse_ScaleLeftRight
from .libdelfem2 import precond_ilu0, linearSystem_setMasterSlave, linsys_solve_pcg, masterSlave_distributeValue, linsys_solve_bicgstab, pointFixBC
from .libdelfem2 import \
  mergeLinSys_poission, \
  mergeLinSys_cloth, \
  mergeLinSys_massPoint, \
  mergeLinSys_contact, \
  mergeLinSys_linearSolidStatic, \
  mergeLinSys_diffuse, \
  mergeLinSys_linearSolidDynamic, \
  mergeLinSys_storksStatic2D, \
  mergeLinSys_storksDynamic2D, \
  mergeLinSys_navierStorks2D
from .libdelfem2 import proj_rigid2d, proj_rigid3d
from .libdelfem2 import matrixSquareSparse_setFixBC
from .libdelfem2 import drawField_colorMap, drawField_disp, drawField_hedgehog
from .libdelfem2 import ColorMap
from .libdelfem2 import elemQuad_dihedralTri, jarray_mesh_psup, jarray_add_diagonal, jarray_sort
from .libdelfem2 import map_value
from .libdelfem2 import write_vtk_meshpoint, write_vtk_meshelem, write_vtk_pointscalar, write_vtk_pointvector
from .libdelfem2 import MathExpressionEvaluator, mass_lumped

from .cadmsh import SDF
from .cadmsh import Mesh



def normalize_rigmsh(rigmsh):
  aabb = rigmsh.aabb()
  center = aabb.center()
  rigmsh.translate(-center[0],-center[1],-center[2])
  rigmsh.scale(1.0/aabb.max_length())

#####################################################

class VisFEM_Hedgehog():
  def __init__(self, fem,
               name_vector=""):
    self.fem = fem
    self.name_vector = name_vector

  def draw(self):
    mesh = self.fem.mesh
    if hasattr(self.fem, self.name_vector):
      npVector = getattr(self.fem, self.name_vector)
      assert type(npVector) == numpy.ndarray
      assert npVector.ndim  == 2
      ndim = self.fem.mesh.np_pos.shape[1]
      gl.glDisable(gl.GL_LIGHTING)
      gl.glColor3d(0, 0, 0)
      drawField_hedgehog(self.fem.mesh.np_pos, npVector[:,0:ndim], 1.0)

class VisFEM_ColorContour():
  def __init__(self, fem,
               name_color="",
               name_disp="",
               idim = 0):
    self.fem = fem
    ####
    self.name_color = name_color
    self.idim = idim
    self.color_mode = 'bcgyr'
    self.color_min = 0.0
    self.color_max = 0.3
    ####
    self.name_disp = name_disp
    self.disp_mode = 'disp'

  def minmax_xyz(self):
    return self.fem.mesh.minmax_xyz()

  def set_color_minmax(self):
    if hasattr(self.fem, self.name_color):
      npColor = getattr(self.fem, self.name_color)
      assert type(npColor) == numpy.ndarray
      assert npColor.ndim == 2 and self.idim < npColor.shape[1]
      self.color_min = npColor[:,self.idim].min()
      self.color_max = npColor[:,self.idim].max()

  def draw(self):
    mesh = self.fem.mesh
    if hasattr(self.fem, self.name_color):
      npColor = getattr(self.fem, self.name_color)
      assert type(npColor) == numpy.ndarray
      assert npColor.ndim == 2 and self.idim < npColor.shape[1]
      self.color_map = ColorMap(self.color_min,self.color_max,self.color_mode)
      drawField_colorMap(mesh.np_pos, mesh.np_elm,
                         npColor[:,self.idim],
                         self.color_map)

    if hasattr(self.fem, self.name_disp):
      npDisp = getattr(self.fem, self.name_disp)
      assert type(npDisp) == numpy.ndarray
      if self.disp_mode == 'disp':
        drawField_disp(mesh.np_pos, mesh.np_elm,
                       npDisp)
      if self.disp_mode == 'hedgehog':
        gl.glDisable(gl.GL_LIGHTING)
        gl.glColor3d(0,0,0)
        drawField_hedgehog(mesh.np_pos, self.val_disp, 1.0)

  def write_vtk(self, path_vtk, message=""):
    mesh = self.fem.mesh
    write_vtk_meshpoint(path_vtk,"foobar", mesh.np_pos)
    write_vtk_meshelem(path_vtk, mesh.np_elm, mesh.elem_type)
    open(path_vtk, "a+").write("POINT_DATA {0}\n".format(mesh.np_pos.shape[0]))
    if hasattr(self.fem, self.name_color):
      npColor = getattr(self.fem, self.name_color)
      assert type(npColor) == numpy.ndarray
      if npColor.ndim == 1 or (npColor.ndim == 2 and npColor.shape[1] == 1):
        write_vtk_pointscalar(path_vtk, npColor)
    elif hasattr(self.fem, self.name_disp):
      npDisp = getattr(self.fem, self.name_disp)
      assert type(npDisp) == numpy.ndarray
      if npDisp.ndim == 2 and npDisp.shape[1] == mesh.np_pos.shape[1]:
        write_vtk_pointvector(path_vtk, npDisp)


class FieldValueSetter():
  def __init__(self,
               mathexp: str,
               val:numpy.ndarray,
               idim:int,
               mesh:Mesh,
               npIdP: numpy.ndarray,
               dt:float):
    self.mesh = mesh
    self.val = val
    self.idim = idim
    self.mathexp = mathexp
    self.npIdP = npIdP
    self.eval = MathExpressionEvaluator()
    self.dt = dt
    self.time_cur = 0.0
    #####
    self.eval.set_key("x",0.0)
    self.eval.set_key("y",0.0)
    self.eval.set_key("z",0.0)
    self.eval.set_key("t",0.0)
    self.eval.set_expression(self.mathexp)

  def step_time(self):
    self.time_cur += self.dt
    for ip in self.npIdP:
      self.eval.set_key("t",self.time_cur)
      if self.mesh.np_pos.shape[1] >= 2:
        self.eval.set_key("x",self.mesh.np_pos[ip,0])
        self.eval.set_key("y",self.mesh.np_pos[ip,1])
      if self.mesh.np_pos.shape[1] == 3:
        self.eval.set_key("z",self.mesh.np_pos[ip,2])
      val = self.eval.eval()
      self.val[ip,self.idim] = self.mesh.np_pos[ip,self.idim] + val

######################################################

class FEM_LinSys():
  def __init__(self,
               np:int, ndimval:int):
    # vectors
    self.np = np
    self.ndimval = ndimval
    self.f = numpy.zeros((np,ndimval), dtype=numpy.float64)
    self.x = numpy.zeros((np,ndimval), dtype=numpy.float64)
    self.bc = numpy.zeros((np,ndimval), dtype=numpy.int32)
    self.ms = None

    self.mat = None
    self.mat_prec = None

    self.nitr = 1000
    self.conv_ratio = 1.0e-4
    self.conv_hist = []

  def set_pattern(self, pattern:tuple, master_slave_ptn=None):
    self.vec_ms = master_slave_ptn
    # matrix
    self.mat = MatrixSquareSparse()
    self.mat.initialize(self.np, self.ndimval, True)
    psup_ind,psup = pattern[0],pattern[1]
    if self.vec_ms is not None:
      psup_ind1,psup1 = addMasterSlavePattern(self.vec_ms,psup_ind,psup)
    else:
      psup_ind1,psup1 = psup_ind,psup
    jarray_sort(psup_ind1, psup1)
    matrixSquareSparse_setPattern(self.mat, psup_ind1, psup1)
    # preconditioner
    self.mat_prec = PreconditionerILU()
    precond_ilu0(self.mat_prec, self.mat)

  def set_zero(self):
    self.mat.set_zero()
    self.f[:,:] = 0.0

  def set_bc_ms(self):
    #### setting bc
    self.f[self.bc != 0] = 0.0
    matrixSquareSparse_setFixBC(self.mat, self.bc)
    if self.vec_ms is not None:
      linearSystem_setMasterSlave(self.mat,self.f, self.vec_ms)

  def set_precond(self):
    self.mat_prec.set_value(self.mat)
    self.mat_prec.ilu_decomp()

  def solve_iteration(self,is_asymmetric=False):
    #### solving matrix
    if not is_asymmetric:
      self.conv_hist = linsys_solve_pcg(self.f, self.x,
                                        self.conv_ratio, self.nitr,
                                        self.mat, self.mat_prec)
    else:
      self.conv_hist = linsys_solve_bicgstab(self.f, self.x,
                                             self.conv_ratio, self.nitr,
                                             self.mat, self.mat_prec)
    self.x[self.bc != 0] = 0.0

##########################################################################



class FEM_Poisson():
  def __init__(self,
               source=0.0,
               alpha=1.0):
    self.source = source
    self.alpha = alpha

  def updated_topology(self,mesh:Mesh,mapper=None,master_slave_pattern=None):
    self.mesh = mesh
    np = self.mesh.np_pos.shape[0]
    ndimval = 1
    val_new = numpy.zeros((np,ndimval), dtype=numpy.float64)  # initial guess is zero
    if mapper is not None and hasattr(self,"value"):
      val_new[:self.value.shape[0],:] = self.value
      map_value(val_new,mapper)
    self.value = val_new
    self.ls = FEM_LinSys(np,ndimval)
    self.ls.set_pattern(self.mesh.psup(),master_slave_ptn=master_slave_pattern)

  def solve(self):
    assert self.ls.mat is not None
    self.ls.set_zero()
    mergeLinSys_poission(self.ls.mat, self.ls.f,
                         self.alpha, self.source,
                         self.mesh.np_pos, self.mesh.np_elm, self.mesh.elem_type,
                         self.value)
    self.ls.set_bc_ms()
    self.ls.set_precond()
    self.ls.solve_iteration()
    self.value += self.ls.x
    if self.ls.vec_ms is not None:
      masterSlave_distributeValue(self.value, self.ls.vec_ms)


class FEM_Diffuse():
  def __init__(self,
               source=0.0):
    self.dt = 0.01
    self.gamma_newmark = 0.6
    self.alpha = 1.0
    self.rho = 1.0
    self.source = source

  def updated_topology(self,mesh:Mesh):
    self.mesh = mesh
    np = self.mesh.np_pos.shape[0]
    ndimval = 1
    self.value = numpy.zeros((np,ndimval), dtype=numpy.float64)  # initial guess is zero
    self.velocity = numpy.zeros((np,ndimval), dtype=numpy.float64)  # initial guess is zero
    self.ls = FEM_LinSys(np,ndimval)
    self.ls.set_pattern(self.mesh.psup())

  def solve(self):
    assert self.ls.mat is not None
    self.ls.set_zero()
    mergeLinSys_diffuse(self.ls.mat, self.ls.f,
                        self.alpha, self.rho, self.source,
                        self.dt, self.gamma_newmark,
                        self.mesh.np_pos, self.mesh.np_elm, self.mesh.elem_type,
                        self.value, self.velocity)
    self.ls.set_bc_ms()
    self.ls.set_precond()
    self.ls.solve_iteration()
    self.value += (self.ls.x)*(self.dt*self.gamma_newmark) + (self.velocity)*self.dt
    self.velocity += self.ls.x

  def step_time(self):
    self.solve()


class FEM_SolidLinearStatic():
  def __init__(self,
               gravity = (0,0,0)):
    self.gravity = gravity
    self.myu = 1.0
    self.lmd = 0.0
    self.rho = 1.0

  def updated_topology(self,mesh:Mesh):
    self.mesh = mesh
    np = self.mesh.np_pos.shape[0]
    ndimval = self.mesh.np_pos.shape[1]
    self.vec_val = numpy.zeros((np,ndimval), dtype=numpy.float64)  # initial guess is zero
    self.ls = FEM_LinSys(np, ndimval)
    self.ls.set_pattern(self.mesh.psup())

  def solve(self):
    assert self.ls.mat is not None
    self.ls.set_zero()
    mergeLinSys_linearSolidStatic(self.ls.mat, self.ls.f,
                                  self.myu, self.lmd, self.rho, self.gravity,
                                  self.mesh.np_pos, self.mesh.np_elm, self.mesh.elem_type,
                                  self.vec_val)
    self.ls.set_bc_ms()
    self.ls.set_precond()
    self.ls.solve_iteration()
    self.vec_val += self.ls.x


class FEM_SolidLinearEigen():
  def __init__(self):
    self.rho = 1.0
    self.mesh = None

  def updated_topology(self,mesh:Mesh):
    self.mesh = mesh
    np = self.mesh.np_pos.shape[0]
    ndimval = self.mesh.np_pos.shape[1]
    self.mode = numpy.zeros((np,ndimval), dtype=numpy.float64)  # initial guess is zero
    self.ker = numpy.zeros((6,np,ndimval), dtype=numpy.float64)  # initial guess is zero
    self.mass_lumped_sqrt_inv = numpy.zeros((np,), dtype=numpy.float64)
    self.ls = FEM_LinSys(np, ndimval)
    if self.ls.mat is None:
      self.ls.set_pattern(self.mesh.psup())
    self.updated_geometry()

  def updated_geometry(self):
    mass_lumped(self.mass_lumped_sqrt_inv,
                self.rho, self.mesh.np_pos, self.mesh.np_elm, self.mesh.elem_type)
    self.mass_lumped_sqrt_inv = numpy.sqrt(self.mass_lumped_sqrt_inv)
    if self.mesh.np_pos.shape[1] == 3:
      self.ker = self.ker.reshape((6,-1,3))
      self.ker[:,:,:] = 0.0
      self.ker[0,:,0] = + self.mass_lumped_sqrt_inv[:]
      self.ker[1,:,1] = + self.mass_lumped_sqrt_inv[:]
      self.ker[2,:,2] = + self.mass_lumped_sqrt_inv[:]
      self.ker[3,:,2] = - self.mass_lumped_sqrt_inv*self.mesh.np_pos[:,1]
      self.ker[3,:,1] = + self.mass_lumped_sqrt_inv*self.mesh.np_pos[:,2]
      self.ker[4,:,0] = - self.mass_lumped_sqrt_inv*self.mesh.np_pos[:,2]
      self.ker[4,:,2] = + self.mass_lumped_sqrt_inv*self.mesh.np_pos[:,0]
      self.ker[5,:,1] = - self.mass_lumped_sqrt_inv*self.mesh.np_pos[:,0]
      self.ker[5,:,0] = + self.mass_lumped_sqrt_inv*self.mesh.np_pos[:,1]
      self.ker = self.ker.reshape((6,-1))
      for i in range(6):
        self.ker[i] /= numpy.linalg.norm(self.ker[i])
        for j in range(i+1,6):
          self.ker[j] -= numpy.dot(self.ker[i], self.ker[j]) * self.ker[i]
      '''
      self.ker[0] /= numpy.linalg.norm(self.ker[0])
      self.ker[1] -= numpy.dot(self.ker[0],self.ker[1]) * self.ker[0]
      self.ker[2] -= numpy.dot(self.ker[0],self.ker[2]) * self.ker[0]
      self.ker[3] -= numpy.dot(self.ker[0],self.ker[3]) * self.ker[0]
      self.ker[4] -= numpy.dot(self.ker[0],self.ker[4]) * self.ker[0]
      self.ker[5] -= numpy.dot(self.ker[0],self.ker[5]) * self.ker[0]
      self.ker[1] /= numpy.linalg.norm(self.ker[1])
      self.ker[2] -= numpy.dot(self.ker[1],self.ker[2]) * self.ker[1]
      self.ker[3] -= numpy.dot(self.ker[1],self.ker[3]) * self.ker[1]
      self.ker[4] -= numpy.dot(self.ker[1],self.ker[4]) * self.ker[1]
      self.ker[5] -= numpy.dot(self.ker[1],self.ker[5]) * self.ker[1]
      self.ker[2] /= numpy.linalg.norm(self.ker[2])
      self.ker[3] -= numpy.dot(self.ker[2],self.ker[3]) * self.ker[2]
      self.ker[4] -= numpy.dot(self.ker[2],self.ker[4]) * self.ker[2]
      self.ker[5] -= numpy.dot(self.ker[2],self.ker[5]) * self.ker[2]
      self.ker[3] /= numpy.linalg.norm(self.ker[3])
      self.ker[4] -= numpy.dot(self.ker[3],self.ker[4]) * self.ker[3]
      self.ker[5] -= numpy.dot(self.ker[3],self.ker[5]) * self.ker[3]
      self.ker[4] /= numpy.linalg.norm(self.ker[4])
      self.ker[5] -= numpy.dot(self.ker[4],self.ker[5]) * self.ker[4]
      self.ker[5] /= numpy.linalg.norm(self.ker[5])
      '''
      '''
      for i in range(6):
        for j in range(i,6):
          print(i,j,numpy.dot(self.ker[i],self.ker[j]))
      '''
      self.ker = self.ker.reshape((6,-1,3))
      self.mass_lumped_sqrt_inv = numpy.reciprocal( self.mass_lumped_sqrt_inv )
    else:
      assert 0
      pass
    self.ls.set_zero()
    self.mode[:] = 0.0
    mergeLinSys_linearSolidStatic(self.ls.mat, self.ls.f,
                                  1.0, 0.1, 0.0, (0,0,0),
                                  self.mesh.np_pos, self.mesh.np_elm, self.mesh.elem_type,
                                  self.mode)
    matrixSquareSparse_ScaleLeftRight(self.ls.mat, self.mass_lumped_sqrt_inv)
    self.ls.mat.add_dia(1.0)
    ####
    self.ls.set_precond()

  def solve(self):
    self.mode[:] = self.ls.f
    self.ls.x[:] = 0.0
    self.ls.solve_iteration()
    ####
    x = self.ls.x.reshape((-1))
    self.ker = self.ker.reshape((6,-1))
    x -= numpy.dot(x,self.ker[0]) * self.ker[0]
    x -= numpy.dot(x,self.ker[1]) * self.ker[1]
    x -= numpy.dot(x,self.ker[2]) * self.ker[2]
    x -= numpy.dot(x,self.ker[3]) * self.ker[3]
    x -= numpy.dot(x,self.ker[4]) * self.ker[4]
    x -= numpy.dot(x,self.ker[5]) * self.ker[5]
    x /= numpy.linalg.norm(x)
    self.ker = self.ker.reshape((6,-1,3))
    self.ls.f[:] = self.ls.x
    ####
    self.mode[:,0] = self.mass_lumped_sqrt_inv*self.ls.x[:,0]*0.03
    self.mode[:,1] = self.mass_lumped_sqrt_inv*self.ls.x[:,1]*0.03
    self.mode[:,2] = self.mass_lumped_sqrt_inv*self.ls.x[:,2]*0.03

  def step_time(self):
    self.solve()


class FEM_SolidLinearDynamic():
  def __init__(self,
               gravity=(0, 0, 0)):
    self.mesh = None
    self.gravity = gravity
    self.dt = 0.1
    self.gamma_newmark = 0.6
    self.beta_newmark = 0.36

  def updated_topology(self,mesh:Mesh):
    self.mesh = mesh
    np = self.mesh.np_pos.shape[0]
    ndimval = self.mesh.np_pos.shape[1]
    self.vec_val = numpy.zeros((np,ndimval), dtype=numpy.float64)  # initial guess is zero
    self.vec_velo = numpy.zeros((np,ndimval), dtype=numpy.float64)  # initial guess is zero
    self.vec_acc = numpy.zeros((np,ndimval), dtype=numpy.float64)  # initial guess is zero
    self.ls = FEM_LinSys(np, ndimval)
    self.ls.set_pattern(self.mesh.psup())

  def solve(self):
    assert self.ls.mat is not None
    self.ls.set_zero()
    mergeLinSys_linearSolidDynamic(self.ls.mat, self.ls.f,
                                   1.0, 0.0, 1.0, self.gravity,
                                   self.dt, self.gamma_newmark, self.beta_newmark,
                                   self.mesh.np_pos, self.mesh.np_elm, self.mesh.elem_type,
                                   self.vec_val, self.vec_velo, self.vec_acc)
    self.ls.set_bc_ms()
    self.ls.set_precond()
    self.ls.solve_iteration()
    self.vec_val += (self.dt)*self.vec_velo + (0.5*self.dt*self.dt)*self.vec_acc + (self.dt*self.dt*self.beta_newmark)*self.ls.x
    self.vec_velo += (self.dt*self.gamma_newmark)*self.ls.x + (self.dt)*self.vec_acc
    self.vec_acc += self.ls.x

  def step_time(self):
    self.solve()


class FEM_Cloth():
  def __init__(self):
    self.dt = 0.1
    self.sdf = SDF()

  def updated_topology(self,mesh:Mesh,mapper=None):
    self.mesh = mesh
    np = self.mesh.np_pos.shape[0]
    ndimval = 3
    vec_val_new = numpy.zeros((np,ndimval), dtype=numpy.float64)  # initial guess is zero
    vec_velo_new = numpy.zeros((np,ndimval), dtype=numpy.float64)  # initial guess is zero
    if mapper is not None:
      vec_val_new[:self.vec_val.shape[0],:] = self.vec_val
      vec_velo_new[:self.vec_velo.shape[0],:] = self.vec_velo
      map_value(vec_val_new,mapper)
      map_value(vec_velo_new,mapper)
    else:
      vec_val_new[:,:2] = self.mesh.np_pos
    self.vec_val = vec_val_new
    self.vec_velo = vec_velo_new
    self.ls = FEM_LinSys(np,ndimval)
    self.np_quad = elemQuad_dihedralTri(self.mesh.np_elm, np)
    self.ls.set_pattern(jarray_mesh_psup(self.np_quad, np))

  def solve(self):
    assert self.ls.mat is not None
    self.ls.set_zero()
    mergeLinSys_cloth(self.ls.mat, self.ls.f,
                      10.0, 500.0, self.dt,
                      self.mesh.np_pos, self.mesh.np_elm,
                      self.np_quad,
                      self.vec_val)
    mergeLinSys_massPoint(self.ls.mat, self.ls.f,
                          1.0, self.dt,
                          [0,0,-1],
                          self.vec_val, self.vec_velo)
    mergeLinSys_contact(self.ls.mat, self.ls.f,
                        10000, 0.1,
                        self.sdf.list_sdf,
                        self.vec_val)
    self.ls.set_bc_ms()
    self.ls.set_precond()
    self.ls.solve_iteration()
    self.vec_val += self.ls.x
    self.vec_velo = (1.0/self.dt)*self.ls.x

  def step_time(self):
    self.solve()


##########################
## fluid from here

class FEM_StorksStatic2D():
  def __init__(self,
               mesh: Mesh):
    self.mesh = mesh
    self.updated_topology()

  def updated_topology(self):
    np = self.mesh.np_pos.shape[0]
    ndimval = 3
    self.ls = FEM_LinSys(np,ndimval)
    self.vec_val = numpy.zeros((np,ndimval), dtype=numpy.float64)  # initial guess is zero
    self.ls.set_pattern(self.mesh.psup())

  def solve(self):
    assert self.ls.mat is not None
    self.ls.set_zero()
    mergeLinSys_storksStatic2D(self.ls.mat, self.ls.f,
                               1.0, 0.0, 0.0,
                               self.mesh.np_pos, self.mesh.np_elm,
                               self.vec_val)
    self.ls.set_bc_ms()
    self.ls.set_precond()
    self.ls.solve_iteration()
    self.vec_val += self.ls.x

  def step_time(self):
    self.solve()


class FEM_StorksDynamic2D():
  def __init__(self,
               mesh: Mesh):
    self.dt = 0.005
    self.gamma_newmark = 0.6
    self.mesh = mesh
    self.updated_topology()

  def updated_topology(self):
    np = self.mesh.np_pos.shape[0]
    ndimval = 3
    self.ls = FEM_LinSys(np,ndimval)
    self.vec_val = numpy.zeros((np,ndimval), dtype=numpy.float64)  # initial guess is zero
    self.vec_velo = numpy.zeros((np,ndimval), dtype=numpy.float64)  # initial guess is zero
    self.ls.set_pattern(self.mesh.psup())

  def solve(self):
    assert self.ls.mat is not None
    self.ls.set_zero()
    mergeLinSys_storksDynamic2D(self.ls.mat, self.ls.f,
                                1.0, 1.0, 0.0, 0.0,
                                self.dt, self.gamma_newmark,
                                self.mesh.np_pos, self.mesh.np_elm,
                                self.vec_val, self.vec_velo)
    self.ls.set_bc_ms()
    self.ls.set_precond()
    self.ls.solve_iteration()
    self.vec_val += (self.ls.x)*(self.dt*self.gamma_newmark) + (self.vec_velo)*self.dt
    self.vec_velo += self.ls.x

  def step_time(self):
    self.solve()


class FEM_NavierStorks2D():
  def __init__(self,
               mesh: Mesh):
    self.dt = 0.1
    self.gamma_newmark = 0.6
    self.mesh = mesh
    self.updated_topology()

  def updated_topology(self):
    np = self.mesh.np_pos.shape[0]
    ndimval = 3
    self.vec_val = numpy.zeros((np,ndimval), dtype=numpy.float64)  # initial guess is zero
    self.vec_velo = numpy.zeros((np,ndimval), dtype=numpy.float64)  # initial guess is zero
    self.ls = FEM_LinSys(np,ndimval)
    self.ls.set_pattern(self.mesh.psup())

  def solve(self):
    assert self.ls.mat is not None
    self.ls.set_zero()
    mergeLinSys_navierStorks2D(self.ls.mat, self.ls.f,
                                1.0, 1000.0, 0.0, 0.0,
                                self.dt, self.gamma_newmark,
                                self.mesh.np_pos, self.mesh.np_elm,
                                self.vec_val, self.vec_velo)
    self.ls.set_bc_ms()
    self.ls.set_precond()
    self.ls.solve_iteration(is_asymmetric=True)
    self.vec_val += (self.ls.x)*(self.dt*self.gamma_newmark) + (self.vec_velo)*self.dt
    self.vec_velo += self.ls.x

  def step_time(self):
    self.solve()


class PBD():
  def __init__(self,
               mesh: Mesh):
    np = mesh.np_pos.shape[0]
    self.mesh = mesh
    self.vec_bc = numpy.zeros((np,), dtype=numpy.int32)
    self.vec_val = mesh.np_pos.copy()
    self.vec_velo = numpy.zeros_like(self.vec_val, dtype=numpy.float64)
    self.vec_tpos = mesh.np_pos.copy()
    self.dt = 0.1
    self.psup = mesh.psup()
    self.psup = jarray_add_diagonal(*self.psup)

  def step_time(self):
    self.vec_tpos[:] = self.vec_val + self.dt * self.vec_velo
    pointFixBC(self.vec_tpos, self.vec_bc, self.vec_val)
    for itr in range(1):
      if self.mesh.np_pos.shape[1] == 2:
        proj_rigid2d(self.vec_tpos,
                     0.5, self.psup[0], self.psup[1],
                     self.mesh.np_pos)
      if self.mesh.np_pos.shape[1] == 3:
        proj_rigid3d(self.vec_tpos,
                     0.5, self.psup[0], self.psup[1],
                     self.mesh.np_pos)
    pointFixBC(self.vec_tpos, self.vec_bc, self.vec_val)
    self.vec_velo[:] = (self.vec_tpos-self.vec_val)/self.dt
    self.vec_val[:] = self.vec_tpos
