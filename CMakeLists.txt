cmake_minimum_required(VERSION 3.5)
project(CGF)

function(get_mingw_exception_dll)
    # determina a dll de exceção usada pelo mingw
    # essa função é posteriormente usada para copiar a dll para a pasta dos exemplos
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        SET(ARCH64_BITS TRUE)
    else ()
        SET(ARCH64_BITS FALSE)
    endif ()
    if (MINGW AND ARCH64_BITS)
        get_filename_component(MINGW_BIN_PATH ${CMAKE_CXX_COMPILER} PATH)
        file(GLOB allddls ${MINGW_BIN_PATH}/*.dll)
        set(MINGW_GCC_THREAD "")
        set(MINGW_GCC_EXCEPTION "")
        foreach (f ${allddls})
            if (${f} MATCHES "libgcc")
                if (${f} MATCHES "dw2")
                    set(MINGW_GCC_EXCEPTION_DLL ${f} PARENT_SCOPE)
                elseif (${f} MATCHES "seh")
                    set(MINGW_GCC_EXCEPTION_DLL ${f} PARENT_SCOPE)
                elseif (${f} MATCHES "sjlj")
                    set(MINGW_GCC_EXCEPTION_DLL ${f} PARENT_SCOPE)
                endif ()
            endif ()
        endforeach ()
    endif ()
endfunction()


function(cgf_dlls_to_target target)
    if (WIN32)
        # copia as dlls para a pasta de saida se necessario
        if (BUILD_SHARED_LIBS)
            # DLLs geradas pelo CMAKE
            set(DLL_TARGETS
                sfml-system
                sfml-window
                sfml-graphics
                sfml-audio
                ${BOX2D_LIB}
                CGF
                )
            foreach (target_dll ${DLL_TARGETS})
                add_custom_command(
                        TARGET ${target}
                        POST_BUILD
                        COMMAND ${CMAKE_COMMAND}
                        ARGS -E copy_if_different $<TARGET_FILE:${target_dll}> $<TARGET_FILE_DIR:${target}>
                )
            endforeach ()

            # copia as DLLs do mingw
            get_filename_component(MINGW_BIN_PATH ${CMAKE_CXX_COMPILER} PATH)
            get_mingw_exception_dll()
            set(MINGW_DLLS ${PROJECT_SOURCE_DIR}/bin/zlib/zlib1.dll ${MINGW_BIN_PATH}/libstdc++-6.dll ${MINGW_BIN_PATH}/libwinpthread-1.dll ${MINGW_GCC_EXCEPTION_DLL})

            foreach (mingw_dll ${MINGW_DLLS})
                if (EXISTS ${mingw_dll})
                    add_custom_command(
                            TARGET ${target}
                            POST_BUILD
                            COMMAND ${CMAKE_COMMAND}
                            ARGS -E copy_if_different ${mingw_dll} $<TARGET_FILE_DIR:${target}>
                    )
                else ()
                    message(WARNING "arquivo ${mingw_dll} não existe")
                endif ()
            endforeach ()
        endif ()
    endif ()

endfunction()

function(cgf_add_example example)
    # adiciona um examplo, criadno um executable, linkando com a biblioteca CGF
    # e coloca o executavel, junto com os arquivos necessários (pasta bin/data), dentro
    # de um diretório na pasta bin/examples
    cmake_parse_arguments(THIS "EXAMPLE" "" "SOURCES" ${ARGN})
    source_group("" FILES ${THIS_SOURCES})
    add_executable(${example} ${THIS_SOURCES})
    target_link_libraries(${example} CGF)
    set_target_properties(${example}
                          PROPERTIES
                          RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/examples/${example}"
                          )

    # o comando é POST_BUILD de maneira que só é executado após a successful compilation
    # do exemplo
    if (UNIX)
        add_custom_command(TARGET ${example} POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E create_symlink
                           ${CMAKE_SOURCE_DIR}/bin/data $<TARGET_FILE_DIR:${example}>/data)
    else ()
        add_custom_command(TARGET ${example} POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy_directory
                           ${CMAKE_SOURCE_DIR}/bin/data $<TARGET_FILE_DIR:${example}>/data)
    endif ()

    # adiciona as dlls no windows, se for necessário
    cgf_dlls_to_target(${example})

endfunction()

set(CGF_ROOT ${PROJECT_SOURCE_DIR})


set(CGF_HEADERS
    ${CGF_ROOT}/lib/CGF/include/Collision.h
    ${CGF_ROOT}/lib/CGF/include/Anim.h
    ${CGF_ROOT}/lib/CGF/include/InputManager.h
    ${CGF_ROOT}/lib/CGF/include/Game.h
    ${CGF_ROOT}/lib/CGF/include/VectorUtils.h
    ${CGF_ROOT}/lib/CGF/include/pugixml/pugiconfig.hpp
    ${CGF_ROOT}/lib/CGF/include/pugixml/pugixml.hpp
    ${CGF_ROOT}/lib/CGF/include/TexRect.h
    ${CGF_ROOT}/lib/CGF/include/ClockHUD.h
    ${CGF_ROOT}/lib/CGF/include/FrameClock.h
    ${CGF_ROOT}/lib/CGF/include/EEDebugDraw3.h
    ${CGF_ROOT}/lib/CGF/include/GameState.h
    ${CGF_ROOT}/lib/CGF/include/Physics.h
    ${CGF_ROOT}/lib/CGF/include/Sprite.h
    ${CGF_ROOT}/lib/CGF/include/tmx/QuadTreeNode.h
    ${CGF_ROOT}/lib/CGF/include/tmx/MapLoader.h
    ${CGF_ROOT}/lib/CGF/include/tmx/Helpers.h
    ${CGF_ROOT}/lib/CGF/include/tmx/MapObject.h
    ${CGF_ROOT}/lib/CGF/include/tmx/zconf.h
    ${CGF_ROOT}/lib/CGF/include/TextureManager.h
    lib/CGF/include/Debug.h)

set(CGF_SOURCES
    ${CGF_ROOT}/lib/CGF/source/TextureManager.cpp
    ${CGF_ROOT}/lib/CGF/source/EEDebugDraw3.cpp
    ${CGF_ROOT}/lib/CGF/source/Physics.cpp
    ${CGF_ROOT}/lib/CGF/source/tmxloader/MapLoaderPublic.cpp
    ${CGF_ROOT}/lib/CGF/source/tmxloader/QuadTreeNode.cpp
    ${CGF_ROOT}/lib/CGF/source/tmxloader/MapLoaderPrivate.cpp
    ${CGF_ROOT}/lib/CGF/source/pugixml/pugixml.cpp
    ${CGF_ROOT}/lib/CGF/source/Sprite.cpp
    ${CGF_ROOT}/lib/CGF/source/Game.cpp
    ${CGF_ROOT}/lib/CGF/source/Collision.cpp
    ${CGF_ROOT}/lib/CGF/source/InputManager.cpp
    )


#=====================SFML=====================
set(SFML_ROOT lib/SFML-2.4.0)
add_subdirectory(lib/SFML-2.4.0)
#=====================SFML=====================


#=====================BOX2D=====================
set(BOX2D_ROOT ${CGF_ROOT}/lib/Box2D-2.3.0)
if (BUILD_SHARED_LIBS)
    set(BOX2D_BUILD_SHARED TRUE)
    set(BOX2D_LIB Box2D_shared)
else ()
    set(BOX2D_BUILD_STATIC TRUE)
    set(BOX2D_LIB Box2D)

endif ()
add_subdirectory(${BOX2D_ROOT}/include/Box2D)

#=====================BOX2D=====================

#=====================CGF=====================
add_library(CGF ${CGF_HEADERS} ${CGF_SOURCES})

# LTO
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-flto GXX_HAS_LTO_FLAG)
message(STATUS "Suporta LTO = ${GXX_HAS_LTO_FLAG}")

target_compile_options(CGF PUBLIC
                       -std=c++11 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-ignored-qualifiers -Wno-reorder
                       $<$<CONFIG:Debug>:-g -Og>
                       $<$<CONFIG:Release>:-g0 -O2 -s $<$<BOOL:${GXX_HAS_LTO_FLAG}>:-flto>>)

if (WIN32)
    # no windows procura a zlib nesta pasta
    set(${ZLIB_ROOT} ${CGF_ROOT}/bin/zlib/)
endif ()

find_package(ZLIB REQUIRED)

target_include_directories(CGF PUBLIC ${ZLIB_INCLUDE_DIRS})
target_include_directories(CGF PUBLIC ${CGF_ROOT}/lib/CGF/include)
target_include_directories(CGF PUBLIC ${SFML_ROOT}/include)
target_include_directories(CGF PUBLIC ${BOX2D_ROOT}/include)
target_include_directories(CGF PUBLIC ${CGF_ROOT}/lib/glew-1.10/include)
target_link_libraries(CGF ${BOX2D_LIB} sfml-system sfml-window sfml-graphics sfml-audio ${ZLIB_LIBRARIES})

# desabilita/habilita loggin

set(CGF_DISABLE_LOGGING FALSE CACHE BOOL "Desabilita mensagens de debug")
if (CGF_DISABLE_LOGGING)
    message(STATUS "Desabilitando mensagens de debug")
    target_compile_definitions(CGF PRIVATE -DDISABLE_LOGGING)
endif ()


#=====================CGF=====================

######################EXEMPLOS######################

set(EXAMPLE_OUTPUT ${PROJECT_SOURCE_DIR}/bin/example CACHE PATH "Aonde vão os binários dos exemplos")
set(CGF_BUILD_EXAMPLES FALSE CACHE BOOL "Compilar os exemplos")

if (CGF_BUILD_EXAMPLES)
    set(E1
        examples/01_Movement/main.cpp
        examples/01_Movement/MenuState.cpp
        examples/01_Movement/MenuState.h
        examples/01_Movement/PlayState.cpp
        examples/01_Movement/PlayState.h
        )

    set(E2
        examples/02_Sprites/main.cpp
        examples/02_Sprites/PlayState.cpp
        examples/02_Sprites/PlayState.h
        )

    set(E3
        examples/03_Tiling/main.cpp
        examples/03_Tiling/PlayState_Solution.cpp
        examples/03_Tiling/PlayState_Solution.h
        )

    set(E4
        examples/04_Collisions/main.cpp
        examples/04_Collisions/PlayState_Solution.cpp
        examples/04_Collisions/PlayState_Solution.h
        )

    set(E5
        examples/05_Physics/main.cpp
        examples/05_Physics/PlayState_Solution.cpp
        examples/05_Physics/PlayState_Solution.h
        )

    set(E6
        examples/06_Audio/main.cpp
        examples/06_Audio/PlayState_Solution.cpp
        examples/06_Audio/PlayState_Solution.h
        )

    set(E7
        examples/07_IA/main.cpp
        examples/07_IA/PlayState.cpp
        examples/07_IA/PlayState.h
        )


    cgf_add_example(01_Movement EXAMPLE SOURCES ${E1})
    cgf_add_example(02_Sprites EXAMPLE SOURCES ${E2})
    cgf_add_example(03_Tiling EXAMPLE SOURCES ${E3})
    cgf_add_example(04_Collisions EXAMPLE SOURCES ${E4})
    cgf_add_example(05_Physics EXAMPLE SOURCES ${E5})
    cgf_add_example(06_Audio EXAMPLE SOURCES ${E6})
    cgf_add_example(07_IA EXAMPLE SOURCES ${E7})
endif ()



