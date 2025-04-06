cmake_minimum_required(VERSION 3.0)
project(mysql_connection_pool)

# 配置编译选项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGE} -g)

# 配置最终可执行文件的输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 配置头文件的搜索路径
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/include)


# 加载指定搜索的子目录
aux_source_directory(src/. SRC_LIST)
aux_source_directory(. PROJECT_LIST)


add_executable(CONNECTION ${SRC_LIST} ${PROJECT_LIST})
target_link_libraries(CONNECTION mysqlclient)