cmake_minimum_required(VERSION 3.8)
project(maze_solver)

# ── compiler warnings ──────────────────────────────────────────────────────
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ── find dependencies ─────────────────────────────────────────────────────
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)

# ── build the C++ node ────────────────────────────────────────────────────
add_executable(wall_follower src/wall_follower.cpp)
target_compile_features(wall_follower PUBLIC cxx_std_17)
ament_target_dependencies(wall_follower
  rclcpp
  sensor_msgs
  geometry_msgs
)

# ── install rules ─────────────────────────────────────────────────────────

# C++ binary
install(TARGETS wall_follower
  DESTINATION lib/${PROJECT_NAME}
)

# Python launch file
install(DIRECTORY launch/
  DESTINATION share/${PROJECT_NAME}/launch
)

# Gazebo world file
install(DIRECTORY worlds/
  DESTINATION share/${PROJECT_NAME}/worlds
)

# ── testing ───────────────────────────────────────────────────────────────
if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
