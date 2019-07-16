####################################################################
# Copyright (c) 2019 Nobuyuki Umetani                              #
#                                                                  #
# This source code is licensed under the MIT license found in the  #
# LICENSE file in the root directory of this source tree.          #
####################################################################

import sys, math

from PySide2.QtCore import Qt, QSize, Signal
from PySide2.QtWidgets import QApplication, QWidget, QSizePolicy
from PySide2.QtWidgets import QVBoxLayout, QHBoxLayout, QCheckBox, QButtonGroup, QRadioButton

sys.path.append("..")
import PyDelFEM2 as dfm2
import PyDelFEM2.gl
import PyDelFEM2.qt
import PyDelFEM2.cadmshsim


class Window_Poisson(dfm2.qt.QW_CadMshFem):
  def __init__(self):
    super(Window_Poisson, self).__init__()
    self.setWindowTitle("CAD_Mesh_Poisson")

    self.cadmsh = dfm2.cadmshsim.CadMesh2D_FEMPoisson(edge_length=0.05)
    self.cadmsh.add_polygon([-1, -1, +1, -1, +1, +1, -1, +1])

    self.ui_fem = dfm2.qt.QW_FEMPoisson(self.cadmsh.fem)

    super().init_UI()


if __name__ == '__main__':
  app = QApplication(sys.argv)
  window = Window_Poisson()
  window.show()
  sys.exit(app.exec_())