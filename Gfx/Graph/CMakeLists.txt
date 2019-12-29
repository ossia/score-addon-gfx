cmake_minimum_required(VERSION 3.5)

    project(rhivideofilter LANGUAGES CXX)

        set(CMAKE_INCLUDE_CURRENT_DIR ON)

            set(CMAKE_AUTOUIC ON) set(CMAKE_AUTOMOC ON) set(CMAKE_AUTORCC ON)

                set(CMAKE_CXX_STANDARD 20) set(CMAKE_CXX_STANDARD_REQUIRED ON)

                    find_package(
                        Qt5 5.14 COMPONENTS Widgets ShaderTools REQUIRED)
                        find_package(Qt5Gui 5.14 REQUIRED Private)

                            add_executable(
                                rhivideofilter main.cpp window.hpp renderstate
                                    .hpp scene.hpp nodes.hpp node.hpp graph
                                    .hpp uniforms.hpp mesh.hpp renderer
                                    .hpp videonode
                                    .hpp

                                        node.cpp graph.cpp renderer.cpp

                                            hap
                                / source / hap.c hap / source / hap.h)

                                target_link_libraries(
                                    rhivideofilter PRIVATE Qt5::Widgets
                                            Qt5::ShaderTools Qt5::GuiPrivate
                                        - Wl,
                                    --start - group,
                                    -lavcodec,
                                    -lavdevice,
                                    -lavfilter,
                                    -lavutil,
                                    -lswscale,
                                    -lavformat,
                                    --end - group - pthread)
