// Copyright 2021 ros2_control Development Team
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "boots/carlikebot_system.hpp"

#include <chrono>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <limits>
#include <memory>
#include <sstream>
#include <vector>

#include "hardware_interface/types/hardware_interface_type_values.hpp"
#include "rclcpp/rclcpp.hpp"

namespace boots
{
hardware_interface::CallbackReturn CarlikeBotSystemHardware::on_init(
  const hardware_interface::HardwareInfo & info)
{
  if (
    hardware_interface::SystemInterface::on_init(info) !=
    hardware_interface::CallbackReturn::SUCCESS)
  {
    return hardware_interface::CallbackReturn::ERROR;
  }
  logger_ = std::make_shared<rclcpp::Logger>(
    rclcpp::get_logger("controller_manager.resource_manager.hardware_component.system.CarlikeBot"));
  clock_ = std::make_shared<rclcpp::Clock>(rclcpp::Clock());

  // Check if the number of joints is correct based on the mode of operation
  if (info_.joints.size() != 2)
  {
    RCLCPP_ERROR(
      get_logger(),
      "CarlikeBotSystemHardware::on_init() - Failed to initialize, "
      "because the number of joints %ld is not 2.",
      info_.joints.size());
    return hardware_interface::CallbackReturn::ERROR;
  }

  for (const hardware_interface::ComponentInfo & joint : info_.joints)
  {
    bool joint_is_steering = joint.name.find("front") != std::string::npos;

    // Steering joints have a position command interface and a position state interface
    if (joint_is_steering)
    {
      RCLCPP_INFO(get_logger(), "Joint '%s' is a steering joint.", joint.name.c_str());

      if (joint.command_interfaces.size() != 1)
      {
        RCLCPP_FATAL(
          get_logger(), "Joint '%s' has %zu command interfaces found. 1 expected.",
          joint.name.c_str(), joint.command_interfaces.size());
        return hardware_interface::CallbackReturn::ERROR;
      }

      if (joint.command_interfaces[0].name != hardware_interface::HW_IF_POSITION)
      {
        RCLCPP_FATAL(
          get_logger(), "Joint '%s' has %s command interface. '%s' expected.", joint.name.c_str(),
          joint.command_interfaces[0].name.c_str(), hardware_interface::HW_IF_POSITION);
        return hardware_interface::CallbackReturn::ERROR;
      }

      // if (joint.state_interfaces.size() != 1)
      // {
      //   RCLCPP_FATAL(
      //     get_logger(), "Joint '%s' has %zu state interface. 1 expected.", joint.name.c_str(),
      //     joint.state_interfaces.size());
      //   return hardware_interface::CallbackReturn::ERROR;
      // }

      // if (joint.state_interfaces[0].name != hardware_interface::HW_IF_POSITION)
      // {
      //   RCLCPP_FATAL(
      //     get_logger(), "Joint '%s' has %s state interface. '%s' expected.", joint.name.c_str(),
      //     joint.state_interfaces[0].name.c_str(), hardware_interface::HW_IF_POSITION);
      //   return hardware_interface::CallbackReturn::ERROR;
      // }
    }
    else
    {
      RCLCPP_INFO(get_logger(), "Joint '%s' is a drive joint.", joint.name.c_str());

      // Drive joints have a velocity command interface and a velocity state interface
      if (joint.command_interfaces.size() != 1)
      {
        RCLCPP_FATAL(
          get_logger(), "Joint '%s' has %zu command interfaces found. 1 expected.",
          joint.name.c_str(), joint.command_interfaces.size());
        return hardware_interface::CallbackReturn::ERROR;
      }

      if (joint.command_interfaces[0].name != hardware_interface::HW_IF_VELOCITY)
      {
        RCLCPP_FATAL(
          get_logger(), "Joint '%s' has %s command interface. '%s' expected.", joint.name.c_str(),
          joint.command_interfaces[0].name.c_str(), hardware_interface::HW_IF_VELOCITY);
        return hardware_interface::CallbackReturn::ERROR;
      }

      // if (joint.state_interfaces.size() != 2)
      // {
      //   RCLCPP_FATAL(
      //     get_logger(), "Joint '%s' has %zu state interface. 2 expected.", joint.name.c_str(),
      //     joint.state_interfaces.size());
      //   return hardware_interface::CallbackReturn::ERROR;
      // }

      // if (joint.state_interfaces[0].name != hardware_interface::HW_IF_VELOCITY)
      // {
      //   RCLCPP_FATAL(
      //     get_logger(), "Joint '%s' has %s state interface. '%s' expected.", joint.name.c_str(),
      //     joint.state_interfaces[1].name.c_str(), hardware_interface::HW_IF_VELOCITY);
      //   return hardware_interface::CallbackReturn::ERROR;
      // }

      // if (joint.state_interfaces[1].name != hardware_interface::HW_IF_POSITION)
      // {
      //   RCLCPP_FATAL(
      //     get_logger(), "Joint '%s' has %s state interface. '%s' expected.", joint.name.c_str(),
      //     joint.state_interfaces[1].name.c_str(), hardware_interface::HW_IF_POSITION);
      //   return hardware_interface::CallbackReturn::ERROR;
      // }
    }
  }

  // // BEGIN: This part here is for exemplary purposes - Please do not copy to your production
  // code
  // hw_start_sec_ = std::stod(info_.hardware_parameters["example_param_hw_start_duration_sec"]);
  // hw_stop_sec_ = std::stod(info_.hardware_parameters["example_param_hw_stop_duration_sec"]);
  // // // END: This part here is for exemplary purposes - Please do not copy to your production code

  hw_interfaces_["steering"] = Joint("virtual_front_wheel_joint");

  hw_interfaces_["traction"] = Joint("virtual_rear_wheel_joint");

  return hardware_interface::CallbackReturn::SUCCESS;
}

std::vector<hardware_interface::StateInterface> CarlikeBotSystemHardware::export_state_interfaces()
{
  std::vector<hardware_interface::StateInterface> state_interfaces;

  for (auto & joint : hw_interfaces_)
  {
    state_interfaces.emplace_back(hardware_interface::StateInterface(
      joint.second.joint_name, hardware_interface::HW_IF_POSITION, &steering_angle_));

    if (joint.first == "traction")
    {
      state_interfaces.emplace_back(hardware_interface::StateInterface(
        joint.second.joint_name, hardware_interface::HW_IF_VELOCITY, &rear_wheel_speed_));
    }
  }

  RCLCPP_INFO(get_logger(), "Exported %zu state interfaces.", state_interfaces.size());

  for (auto s : state_interfaces)
  {
    RCLCPP_INFO(get_logger(), "Exported state interface '%s'.", s.get_name().c_str());
  }

  return state_interfaces;
}

std::vector<hardware_interface::CommandInterface>
CarlikeBotSystemHardware::export_command_interfaces()
{
  std::vector<hardware_interface::CommandInterface> command_interfaces;

  // for (auto & joint : hw_interfaces_)
  // {
  //   if (joint.first == "steering")
  //   {
  //     command_interfaces.emplace_back(hardware_interface::CommandInterface(
  //       joint.second.joint_name, hardware_interface::HW_IF_POSITION,
  //       &rear_wheel_speed_));
  //       // &joint.second.command.position));
  //   }
  //   else if (joint.first == "traction")
  //   {
  //     command_interfaces.emplace_back(hardware_interface::CommandInterface(
  //       joint.second.joint_name, hardware_interface::HW_IF_VELOCITY,
  //       &steering_angle_));
  //       // &joint.second.command.velocity));
  //   }
  // }

  // RCLCPP_INFO(get_logger(), "Exported %zu command interfaces.", command_interfaces.size());

  // for (auto i = 0u; i < command_interfaces.size(); i++)
  // {
  //   RCLCPP_INFO(
  //     get_logger(), "Exported command interface '%s'.", command_interfaces[i].get_name().c_str());
  // }
  command_interfaces.emplace_back(hardware_interface::CommandInterface("virtual_rear_wheel_joint", hardware_interface::HW_IF_VELOCITY, &rear_wheel_speed_));
  command_interfaces.emplace_back(hardware_interface::CommandInterface("virtual_front_wheel_joint", hardware_interface::HW_IF_POSITION, &steering_angle_));
  return command_interfaces;
}

void CarlikeBotSystemHardware::update_commands()
{
  std::string command = std::to_string(rear_wheel_speed_) + " " + std::to_string(steering_angle_) + "\n";
  RCLCPP_INFO(get_logger(), "Sending command: %s", command.c_str());
  arduino_.Write(command);
}


hardware_interface::CallbackReturn CarlikeBotSystemHardware::on_activate(
  const rclcpp_lifecycle::State & /*previous_state*/)
{
  
  RCLCPP_INFO(get_logger(), "Activating ...please wait...");


  

  try{
    arduino_.Open("/dev/ttyUSB1");
    arduino_.SetBaudRate(LibSerial::BaudRate::BAUD_115200);
  } catch(...){
    RCLCPP_FATAL_STREAM(get_logger(),"something went wrong while interacting with the port");
  }
  RCLCPP_INFO(get_logger(),"hardware started and ready to take commands");
  RCLCPP_INFO(get_logger(), "Successfully activated!");

  // return CallbackReturn::SUCCESS;

  // auto node = rclcpp::Node::make_shared("carlikebot_system_hardware");
  // timer_ = node->create_wall_timer(
  //   500ms, std::bind(&CarlikeBotSystemHardware::update_commands, this));


  return hardware_interface::CallbackReturn::SUCCESS;
}




hardware_interface::CallbackReturn CarlikeBotSystemHardware::on_deactivate(
  const rclcpp_lifecycle::State & /*previous_state*/)
{

  RCLCPP_INFO(get_logger(),"hardware started and ready to take commands");
    if (arduino_.IsOpen()){
        try{
            arduino_.Close();
        } catch(...){
            RCLCPP_FATAL_STREAM(rclcpp::get_logger("BumperbotInterface"),"something went wrong while CLOSING THE port");
            return CallbackReturn::FAILURE;
        }
  }
  // BEGIN: This part here is for exemplary purposes - Please do not copy to your production code
  // RCLCPP_INFO(get_logger(), "Deactivating ...please wait...");

  // for (auto i = 0; i < hw_stop_sec_; i++)
  // {
  //   rclcpp::sleep_for(std::chrono::seconds(1));
  //   RCLCPP_INFO(get_logger(), "%.1f seconds left...", hw_stop_sec_ - i);
  // }
  // END: This part here is for exemplary purposes - Please do not copy to your production code
  RCLCPP_INFO(get_logger(), "Successfully deactivated!");
  // timer_.reset();
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::return_type CarlikeBotSystemHardware::read(
  const rclcpp::Time & /*time*/, const rclcpp::Duration & period)
{
  // BEGIN: This part here is for exemplary purposes - Please do not copy to your production code

  hw_interfaces_["steering"].state.position = steering_angle_;

  hw_interfaces_["traction"].state.velocity = rear_wheel_speed_;
  hw_interfaces_["traction"].state.position +=
    rear_wheel_speed_ * period.seconds();

  // std::stringstream ss;
  // ss << "Reading states:";

  // ss << std::fixed << std::setprecision(2) << std::endl
  //    << "\t"
  //    << "position: " << hw_interfaces_["steering"].state.position << " for joint '"
  //    << hw_interfaces_["steering"].joint_name.c_str() << "'" << std::endl
  //    << "\t"
  //    << "position: " << hw_interfaces_["traction"].state.position << " for joint '"
  //    << hw_interfaces_["traction"].joint_name.c_str() << "'" << std::endl
  //    << "\t"
  //    << "velocity: " << hw_interfaces_["traction"].state.velocity << " for joint '"
  //    << hw_interfaces_["traction"].joint_name.c_str() << "'";

  // RCLCPP_INFO_THROTTLE(get_logger(), *get_clock(), 500, "%s", ss.str().c_str());

  // END: This part here is for exemplary purposes - Please do not copy to your production code

  return hardware_interface::return_type::OK;
}

hardware_interface::return_type boots ::CarlikeBotSystemHardware::write(
  const rclcpp::Time & /*time*/, const rclcpp::Duration & /*period*/)
{
  // arduino_.FlushIOBuffers();
  double steering_angle_degrees = steering_angle_ * (180.0 / M_PI);
  std::string command = std::to_string(rear_wheel_speed_) + " " + std::to_string(steering_angle_degrees) + "\n";
  RCLCPP_INFO(get_logger(), command.c_str());
  // std::string command = "0 0\n";

  arduino_.Write(command);
  return hardware_interface::return_type::OK;
  // BEGIN: This part here is for exemplary purposes - Please do not copy to your production code
  // std::stringstream ss;
  // ss << "Writing commands:";

  // ss << std::fixed << std::setprecision(2) << std::endl
  //    << "\t"
  //    << "position: " << hw_interfaces_["steering"].command.position << " for joint '"
  //    << hw_interfaces_["steering"].joint_name.c_str() << "'" << std::endl
  //    << "\t"
  //    << "velocity: " << hw_interfaces_["traction"].command.velocity << " for joint '"
  //    << hw_interfaces_["traction"].joint_name.c_str() << "'";

  // RCLCPP_INFO_THROTTLE(get_logger(), *get_clock(), 500, "%s", ss.str().c_str());
  // // END: This part here is for exemplary purposes - Please do not copy to your production code

  // return hardware_interface::return_type::OK;
}



}  // namespace boots

#include "pluginlib/class_list_macros.hpp"
PLUGINLIB_EXPORT_CLASS(
  boots::CarlikeBotSystemHardware, hardware_interface::SystemInterface)
