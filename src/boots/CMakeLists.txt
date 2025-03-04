cmake_minimum_required(VERSION 3.16)
project(boots LANGUAGES CXX)

if(CMAKE_CXX_COMPILER_ID MATCHES "(GNU|Clang)")
  add_compile_options(-Wall -Wextra)
endif()

# set the same behavior for windows as it is on linux
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# find dependencies
set(THIS_PACKAGE_INCLUDE_DEPENDS
  hardware_interface
  pluginlib
  rclcpp
  rclcpp_lifecycle
)

# find dependencies
find_package(backward_ros REQUIRED)
find_package(ament_cmake REQUIRED)
find_package(PkgConfig)
pkg_check_modules(SERIAL libserial)
foreach(Dependency IN ITEMS ${THIS_PACKAGE_INCLUDE_DEPENDS})
  find_package(${Dependency} REQUIRED)
endforeach()

## COMPILE
add_library(
  boots
  SHARED
  hardware/carlikebot_system.cpp
)
target_compile_features(boots PUBLIC cxx_std_17)
target_include_directories(boots PUBLIC
$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/hardware/include>
$<INSTALL_INTERFACE:include/boots>
)

target_link_libraries(boots PUBLIC serial)
ament_target_dependencies(
  boots PUBLIC
  ${THIS_PACKAGE_INCLUDE_DEPENDS}
)

# Export hardware plugins
pluginlib_export_plugin_description_file(hardware_interface boots.xml)

# INSTALL
install(
  DIRECTORY hardware/include/
  DESTINATION include/boots
)
install(
  DIRECTORY description/launch description/ros2_control description/urdf
  DESTINATION share/boots
)
install(
  DIRECTORY bringup/launch bringup/config
  DESTINATION share/boots
)
install(TARGETS boots
  EXPORT export_boots
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)


if(BUILD_TESTING)
  find_package(ament_cmake_pytest REQUIRED)

  ament_add_pytest_test(example_11_urdf_xacro test/test_urdf_xacro.py)
  ament_add_pytest_test(view_example_11_launch test/test_view_robot_launch.py)
  ament_add_pytest_test(run_example_11_launch test/test_carlikebot_launch.py)
  ament_add_pytest_test(run_example_11_launch_remapped test/test_carlikebot_launch_remapped.py)
endif()

## EXPORTS
ament_export_targets(export_boots HAS_LIBRARY_TARGET)
ament_export_dependencies(${THIS_PACKAGE_INCLUDE_DEPENDS})
ament_package()
