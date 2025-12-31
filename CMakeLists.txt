cmake_minimum_required(VERSION 3.10)
project(RaylibCppTemplate VERSION 1.0)

# 设置C++标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 针对 Emscripten (WebAssembly) 的特殊配置
if(PLATFORM STREQUAL "Web")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    # 设置 Raylib 路径
    set(RAYLIB_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/third_party/raylib/src)
    set(RAYLIB_LIBRARY ${CMAKE_SOURCE_DIR}/third_party/raylib/src/libraylib.web.a)
    set(raylib_FOUND TRUE)
    
    # 添加 Emscripten 编译链接选项
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 -s ASYNCIFY -s TOTAL_MEMORY=67108864 -s FORCE_FILESYSTEM=1 --preload-file ${CMAKE_SOURCE_DIR}/assets@assets --shell-file ${CMAKE_SOURCE_DIR}/third_party/raylib/src/shell.html")
    
    # 定义 WEB 宏以便在代码中进行平台特定处理
    add_definitions(-DPLATFORM_WEB)
else()
    # 定义raylib可能的安装路径
    set(RAYLIB_PATHS 
        /opt/homebrew 
        /usr/local 
        /opt/homebrew/opt/raylib 
        /usr/local/opt/raylib
        ${CMAKE_SOURCE_DIR}/third_party/raylib
    )
    
    # 尝试查找raylib头文件
    find_path(RAYLIB_INCLUDE_DIR 
        NAMES raylib.h 
        PATHS ${RAYLIB_PATHS}
        PATH_SUFFIXES include include/raylib
    )
    
    # 尝试查找raylib库文件
    find_library(RAYLIB_LIBRARY 
        NAMES raylib 
        PATHS ${RAYLIB_PATHS}
        PATH_SUFFIXES lib lib/raylib
    )
endif()

# 检查是否找到了raylib
if(RAYLIB_INCLUDE_DIR AND RAYLIB_LIBRARY)
    set(raylib_FOUND TRUE)
    message(STATUS "Found raylib: ${RAYLIB_LIBRARY}")
    message(STATUS "Raylib include directory: ${RAYLIB_INCLUDE_DIR}")
else()
    # 尝试使用项目中的third_party/raylib-cpp
    set(RAYLIB_CPP_DIR ${CMAKE_SOURCE_DIR}/third_party/raylib-cpp)
    if(EXISTS ${RAYLIB_CPP_DIR})
        message(STATUS "Using raylib-cpp from third_party directory")
        # 为raylib-cpp添加子目录，但不强制构建
        add_subdirectory(${RAYLIB_CPP_DIR} raylib-cpp-build EXCLUDE_FROM_ALL)
        
        # 设置raylib-cpp的包含目录
        set(RAYLIB_CPP_INCLUDE_DIR ${RAYLIB_CPP_DIR}/include)
        
        # 尝试在third_party中查找raylib本身
        set(THIRD_PARTY_RAYLIB_DIR ${CMAKE_SOURCE_DIR}/third_party/raylib)
        if(EXISTS ${THIRD_PARTY_RAYLIB_DIR}/include/raylib.h)
            set(RAYLIB_INCLUDE_DIR ${THIRD_PARTY_RAYLIB_DIR}/include)
            find_library(RAYLIB_LIBRARY 
                NAMES raylib 
                PATHS ${THIRD_PARTY_RAYLIB_DIR}/lib
            )
            set(raylib_FOUND TRUE)
            message(STATUS "Found raylib in third_party directory")
        else()
            # 如果还是找不到raylib，我们需要告诉用户安装
            message(FATAL_ERROR "Could not find raylib. Please install it using 'brew install raylib' on macOS or download it from https://www.raylib.com/")
        endif()
    else()
        message(FATAL_ERROR "Could not find raylib or raylib-cpp. Please install raylib using 'brew install raylib' on macOS.")
    endif()
endif()

# 添加raylib和raylib-cpp的包含目录
include_directories(${RAYLIB_INCLUDE_DIR})
if(EXISTS ${CMAKE_SOURCE_DIR}/third_party/raylib-cpp)
    include_directories(${CMAKE_SOURCE_DIR}/third_party/raylib-cpp/include)
    message(STATUS "Including raylib-cpp from third_party directory")
endif()

# 添加源文件目录到包含路径
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/third_party/rapidjson/include)

# 源文件 - 包含所有源文件
set(SOURCES 
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/core/ResourceManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/UIHelper.cpp
    ${CMAKE_SOURCE_DIR}/src/core/StartScreen.cpp
    ${CMAKE_SOURCE_DIR}/src/core/SettingsMenu.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Meowdex.cpp
    ${CMAKE_SOURCE_DIR}/src/core/GifPlayer.cpp
    ${CMAKE_SOURCE_DIR}/src/core/StatusIndicator.cpp
    ${CMAKE_SOURCE_DIR}/src/core/CatCollection.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Player.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Cat.cpp
    ${CMAKE_SOURCE_DIR}/src/entities/Catnip.cpp
    ${CMAKE_SOURCE_DIR}/src/systems/MapLoader.cpp
)



# 创建主程序
add_executable(${PROJECT_NAME} ${SOURCES})

# 链接raylib库
if(TARGET raylib-cpp)
    target_link_libraries(${PROJECT_NAME} raylib-cpp)
else()
    target_include_directories(${PROJECT_NAME} PRIVATE ${RAYLIB_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${RAYLIB_LIBRARY})
endif()

# 针对macOS的特殊配置
if(APPLE)
    # 设置macOS应用程序属性
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER com.raylibcpp.template
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    )
    
    # 确保在macOS上正确链接
    target_link_libraries(${PROJECT_NAME} "-framework IOKit" "-framework Cocoa" "-framework OpenGL")
endif()

# 安装规则
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
