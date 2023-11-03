cmake_minimum_required (VERSION 2.6)

# 项目信息
project (VoiceService)

set (CMAKE_INCLUDE_CURRENT_DIR ON)

# 添加头文件路径
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/./vehicleHpp)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/./libcpptoolkit/include)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)
# INCLUDE_DIRECTORIES(/usr/include/jsoncpp)
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/./lib)

# 添加要编译的cpp文件
#AUX_SOURCE_DIRECTORY(. DIR_SRCS)
file(GLOB_RECURSE DIR_SRCS
       ${PROJECT_SOURCE_DIR}/src/*.cpp)
# 添加链接库
set (EXTRA_LIBS ${EXTRA_LIBS} asound pthread paho-mqtt3as paho-mqttpp3 curl crypto jsoncpp sqlite3 cpptoolkit)

# 指定生成目标
add_executable(voiceService ${DIR_SRCS})
target_link_libraries(voiceService ${EXTRA_LIBS})

# 安装文件
INSTALL(TARGETS voiceService
       RUNTIME DESTINATION bin
)
# 安装配置文件
INSTALL(FILES ${PROJECT_SOURCE_DIR}/conf/voiceService.json
           DESTINATION conf
)
INSTALL(FILES ${PROJECT_SOURCE_DIR}/conf/voiceService.json
           DESTINATION defaultConf
)

# 目录名不以/结尾，包含目录本身，目录名以 / 结尾，目录下的内容，不包括目录本身, 创建db和voice空目录，注意工程中db和voice文件夹须为空
INSTALL(DIRECTORY ${PROJECT_SOURCE_DIR}/db/
           DESTINATION db
)
INSTALL(DIRECTORY ${PROJECT_SOURCE_DIR}/voice/
           DESTINATION voice
)