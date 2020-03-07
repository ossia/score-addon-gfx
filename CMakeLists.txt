project(score_addon_gfx LANGUAGES C CXX)

find_package(Qt5 5.14 OPTIONAL_COMPONENTS ShaderTools)
if(NOT TARGET Qt5::ShaderTools)
  return()
endif()

# General initialization
score_common_setup()


find_package(Qt5Gui 5.14 REQUIRED Private)

if(NOT TARGET avformat)
  find_package(FFmpeg COMPONENTS  AVCODEC AVFORMAT AVUTIL)
endif()

add_subdirectory(3rdparty/libisf)

# Source files
set(HDRS
    Gfx/CommandFactory.hpp

    Gfx/Filter/Executor.hpp
    Gfx/Filter/Inspector.hpp
    Gfx/Filter/LocalTree.hpp
    Gfx/Filter/Metadata.hpp
    Gfx/Filter/Presenter.hpp
    Gfx/Filter/Process.hpp
    Gfx/Filter/View.hpp
    Gfx/Filter/Layer.hpp

    Gfx/Video/Executor.hpp
    Gfx/Video/Inspector.hpp
    Gfx/Video/LocalTree.hpp
    Gfx/Video/Metadata.hpp
    Gfx/Video/Presenter.hpp
    Gfx/Video/Process.hpp
    Gfx/Video/View.hpp
    Gfx/Video/Layer.hpp

    Gfx/Graph/main.cpp
    Gfx/Graph/window.hpp
    Gfx/Graph/renderstate.hpp
    Gfx/Graph/scene.hpp
    Gfx/Graph/nodes.hpp
    Gfx/Graph/node.hpp
    Gfx/Graph/filternode.hpp
    Gfx/Graph/isfnode.hpp
    Gfx/Graph/graph.hpp
    Gfx/Graph/uniforms.hpp
    Gfx/Graph/mesh.hpp
    Gfx/Graph/renderer.hpp
    Gfx/Graph/videodecoder.hpp
    Gfx/Graph/videonode.hpp

    Gfx/GfxApplicationPlugin.hpp
    Gfx/GfxContext.hpp
    Gfx/GfxExec.hpp
    Gfx/GfxDevice.hpp
    Gfx/TexturePort.hpp

    # Gfx/graph/hap/source/hap.h

    score_addon_gfx.hpp
)

set(SRCS
    Gfx/Filter/Executor.cpp
    Gfx/Filter/Inspector.cpp
    Gfx/Filter/LocalTree.cpp
    Gfx/Filter/Presenter.cpp
    Gfx/Filter/Process.cpp
    Gfx/Filter/View.cpp

    Gfx/Video/Executor.cpp
    Gfx/Video/Inspector.cpp
    Gfx/Video/LocalTree.cpp
    Gfx/Video/Presenter.cpp
    Gfx/Video/Process.cpp
    Gfx/Video/View.cpp

    Gfx/Graph/node.cpp
    Gfx/Graph/graph.cpp
    Gfx/Graph/renderer.cpp

    Gfx/GfxApplicationPlugin.cpp
    Gfx/GfxDevice.cpp
    Gfx/GfxExec.cpp
    Gfx/TexturePort.cpp

    # Gfx/graph/hap/source/hap.c

    score_addon_gfx.cpp
)


# Creation of the library
add_library(${PROJECT_NAME} ${SRCS} ${HDRS})

# Code generation
score_generate_command_list_file(${PROJECT_NAME} "${HDRS}")

# Link
target_link_libraries(${PROJECT_NAME} PUBLIC
  score_lib_base  score_lib_localtree score_lib_process score_plugin_dataflow score_plugin_engine
   Qt5::ShaderTools  Qt5::GuiPrivate
)
target_link_libraries(${PROJECT_NAME} PRIVATE
  avcodec avformat swresample avutil isf)

# Target-specific options
setup_score_plugin(${PROJECT_NAME})

target_compile_options(${PROJECT_NAME} PRIVATE -std=c++2a)
