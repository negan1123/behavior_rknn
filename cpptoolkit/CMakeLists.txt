cmake_minimum_required (VERSION 2.8)

# 项目信息
project (cpptoolkit)

# 查找依赖包OpenSSL
#find_package(OpenSSL)
include_directories(${OPENSSL_INCLUDE_DIR})

set (CMAKE_INCLUDE_CURRENT_DIR ON)

# 添加头文件路径
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(/usr/local/include/opencv4)

# 添加要编译的cpp文件
#AUX_SOURCE_DIRECTORY(. DIR_SRCS)
file(GLOB_RECURSE DIR_SRCS
       ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/src/*.c ${PROJECT_SOURCE_DIR}/src/*.cc)
# 添加链接库
set (EXTRA_LIBS ${EXTRA_LIBS} pthread paho-mqtt3as paho-mqttpp3 i2c rockchip_mpp avcodec opencv_core ssl crypto swresample avutil postproc)

# 指定生成目标
add_library (cpptoolkit SHARED ${DIR_SRCS})
target_link_libraries(cpptoolkit ${OPENSSL_LIBRARIES} ${EXTRA_LIBS})

INSTALL(TARGETS cpptoolkit
       LIBRARY DESTINATION lib
)

