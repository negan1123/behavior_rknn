cmake_minimum_required (VERSION 2.8)

# 项目信息
project(behavior_new)

set(CMAKE_INCLUDE_CURRENT_DIR ON)



# 添加要编译的cpp文件
file(GLOB DIR_SRCS
    ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/src/classify/*.cpp)

#添加头文件路径
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/./vehicleHpp)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/./libcpptoolkit/include)
INCLUDE_DIRECTORIES(/usr/local/include/opencv4)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/lib)

add_definitions("-DRKNN_HAVE_MEAN_DEAL")
add_definitions("-DASIO_STANDALONE")

#添加链接库
set (EXTRA_LIBS ${EXTRA_LIBS}  paho-mqtt3as paho-mqttpp3 rknn_api rockx opencv_highgui opencv_imgproc opencv_imgcodecs opencv_core  pthread avformat avdevice avfilter avcodec avutil swresample swscale cpptoolkit)

# 指定生成目标
add_executable(behavior ${DIR_SRCS})
target_link_libraries(behavior ${EXTRA_LIBS})
INSTALL(TARGETS behavior
       RUNTIME DESTINATION bin
)


#安装配置文件
INSTALL(FILES ../conf/behavior.json DESTINATION conf)
INSTALL(FILES ../conf/behavior.json DESTINATION defaultConf)
#安装模型
INSTALL(DIRECTORY ../models/ DESTINATION model)
INSTALL(DIRECTORY ${PROJECT_SOURCE_DIR}/detector/
           DESTINATION detector
)
