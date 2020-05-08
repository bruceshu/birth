CMAKE_MINIMUM_REQUIRED(VERSION 3.15)

set(LIBRARY_OUTPUT_PATH "${PROJECT_BINARY_DIR}/lib")

# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(${PROJECT_SOURCE_DIR}/src/helloLib HELLOlib_src)

INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/src/helloLib")

ADD_LIBRARY(hello SHARED ${HELLOlib_src})
ADD_LIBRARY(hello_static STATIC ${HELLOlib_src})
SET_TARGET_PROPERTIES(hello_static PROPERTIES OUTPUT_NAME "hello")
#一般我们希望同时编译出静态库和动态库，并且命名一致，但是cmake的target不允许重名，所以我们把静态库命名为hello_static，然后通过上述语句修改其输出的名字，为hello，这样就可以在lib目录下生成libhello.a和lobhello.so。
