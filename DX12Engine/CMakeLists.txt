cmake_minimum_required(VERSION 3.20)
project(DX12Engine)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(Src)

add_executable(DX12Engine
        Src/CPUPerformance.cpp
        Src/CPUPerformance.h
        Src/d3dx12.h
        Src/DataContainerParser.cpp
        Src/DataContainerParser.h
        Src/DataContainerSection.cpp
        Src/DataContainerSection.h
        Src/DataFileContainer.cpp
        Src/DataFileContainer.h
        Src/Direct3D.cpp
        Src/Direct3D.h
        Src/FPSTimer.cpp
        Src/FPSTimer.h
        Src/Graphics.cpp
        Src/Graphics.h
        Src/Input.cpp
        Src/Input.h
        Src/Logger.h
        Src/main.cpp
        Src/MaterialManager.cpp
        Src/MaterialManager.h
        Src/Singleton.h
        Src/System.cpp
        Src/System.h
        Src/Texture.cpp
        Src/Texture.h
        Src/TextureManager.cpp
        Src/TextureManager.h
        Src/Timer.cpp
        Src/Timer.h
        Src/Transform.cpp
        Src/Transform.h
        SceneGraph.cpp
        SceneGraph.h
        StaticMesh.cpp
        StaticMesh.h
        StaticModel.cpp
        StaticModel.h)
