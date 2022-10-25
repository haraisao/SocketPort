cmake_minimum_required(VERSION 3.2)

project(SockPort)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

#
#
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# if use ROS2
#find_package(ament_cmake REQUIRED)
#find_package(rclcpp REQUIRED)
#find_package(rclcpp_components REQUIRED)
#find_package(rcutils REQUIRED)
#find_package(rmw REQUIRED)
#find_package(std_msgs REQUIRED)

find_package(yaml-cpp REQUIRED)

set(CMAKE_CXX_FLAGS -DLinux)

include_directories(include)

set(SOCKETPORT socketport)
set(LIB_SOCKETPORT ${SOCKETPORT} yaml-cpp)

add_library(${SOCKETPORT} SHARED src/SockPort.cpp src/param.cpp)
target_link_libraries(${LIB_SOCKETPORT})

##
# Examples
add_executable(server examples/server.cpp)
add_executable(client examples/client.cpp) 
target_link_libraries(client ${LIB_SOCKETPORT})
target_link_libraries(server ${LIB_SOCKETPORT})
####


#ament_target_dependencies(server rclcpp std_msgs)
#ament_target_dependencies(client rclcpp std_msgs)

install(
  TARGETS server client
  DESTINATION lib/${PROJECT_NAME}
)


#rclcpp_component_register_node()

#ament_package()

