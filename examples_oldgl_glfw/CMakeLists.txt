cmake_minimum_required(VERSION 3.12)

option(USE_STATIC_LIB "compile delfem2 with static library?" OFF)

IF (USE_STATIC_LIB)
  message("$$$$$$$$$$$$$$$$$$$$$")
  message("generate project as STATIC library: USE_STATIC_LIB=${USE_STATIC_LIB}")
  project(examples_oldgl_glfw_static)
  add_subdirectory(dfm2_static_glfwold)
ELSE ()
  message("$$$$$$$$$$$$$$$$$$$$$")
  message("generate project as HEADER-ONLY library: USE_STATIC_LIB=${USE_STATIC_LIB}")
  project(examples_oldgl_glfw_hdronly)
ENDIF ()

# fem
add_subdirectory(613_FemRod3Darboux)
add_subdirectory(616_FemRod3StraightStatic)
add_subdirectory(617_FemRod3HairDarbouxSelfcollision)
add_subdirectory(622_FemHairDarbouxStatic)
add_subdirectory(623_FemHairDarbouxDynamic)