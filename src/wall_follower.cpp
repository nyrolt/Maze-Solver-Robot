// wall_follower.cpp — C++ port of wall_follower.py
// Right-hand rule maze solver for TurtleBot3 in Gazebo (ROS 2)

#include <cmath>
#include <vector>
#include <algorithm>
#include <limits>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include "geometry_msgs/msg/twist.hpp"

class WallFollower : public rclcpp::Node
{
public:
  WallFollower()
  : rclcpp::Node("wall_follower"),
    desired_distance_(0.28),
    Kp_(2.0),
    Kd_(0.4),
    prev_error_(0.0),
    max_speed_(0.22),
    min_speed_(0.08)
  {
    publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);

    subscription_ = this->create_subscription<sensor_msgs::msg::LaserScan>(
      "/scan", 10,
      std::bind(&WallFollower::scan_callback, this, std::placeholders::_1));
  }

private:
  // ── helpers ──────────────────────────────────────────────────────────────

  /// Return the minimum finite, positive value from a range slice.
  /// Returns 10.0 if all values are nan/inf/<=0 (i.e. "nothing nearby").
  float safe_min(const std::vector<float> & values,
                 std::size_t start, std::size_t end)
  {
    float best = 10.0f;
    for (std::size_t i = start; i < end && i < values.size(); ++i) {
      float v = values[i];
      if (std::isfinite(v) && v > 0.0f) {
        best = std::min(best, v);
      }
    }
    return best;
  }

  // ── main callback ─────────────────────────────────────────────────────────

  void scan_callback(const sensor_msgs::msg::LaserScan::SharedPtr msg)
  {
    const auto & r = msg->ranges;
    std::size_t  n = r.size();

    // Lidar sectors  (same indices as Python version)
    // front: first 20 + last 20 rays
    float front = 10.0f;
    for (std::size_t i = 0; i < 20 && i < n; ++i) {
      if (std::isfinite(r[i]) && r[i] > 0.0f) front = std::min(front, r[i]);
    }
    for (std::size_t i = (n > 20 ? n - 20 : 0); i < n; ++i) {
      if (std::isfinite(r[i]) && r[i] > 0.0f) front = std::min(front, r[i]);
    }

    float right       = safe_min(r, 260, 280);
    float front_right = safe_min(r, 300, 330);

    // stronger right opening detection
    bool right_open = (right > 0.75f) && (front_right > 0.75f);

    geometry_msgs::msg::Twist twist;

    // ── RIGHT HAND RULE (fixed priority) ─────────────────────────────────

    // 1️⃣  FRONT BLOCKED → TURN LEFT
    if (front < 0.35f) {
      twist.linear.x  = 0.0;
      twist.angular.z = 1.2;
    }
    // 2️⃣  RIGHT OPEN → TURN RIGHT
    else if (right_open) {
      twist.linear.x  = 0.05;
      twist.angular.z = -1.0;
    }
    // 3️⃣  FOLLOW WALL (PD controller)
    else {
      float error   = right - desired_distance_;
      float d_error = error - prev_error_;
      prev_error_   = error;

      float angular = -(Kp_ * error + Kd_ * d_error);

      // clamp oscillations
      if (std::abs(angular) < 0.05f) { angular = 0.0f; }
      angular = std::max(-1.0f, std::min(1.0f, angular));

      float speed = max_speed_ * (front / 1.2f);
      speed = std::max(min_speed_, std::min(max_speed_, speed));

      twist.linear.x  = speed;
      twist.angular.z = angular;
    }

    publisher_->publish(twist);

    RCLCPP_INFO_THROTTLE(
      this->get_logger(), *this->get_clock(), 500,
      "F:%.2f R:%.2f FR:%.2f  vx:%.2f wz:%.2f",
      front, right, front_right,
      twist.linear.x, twist.angular.z);
  }

  // ── members ───────────────────────────────────────────────────────────────

  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr        publisher_;
  rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr   subscription_;

  const float desired_distance_;
  const float Kp_;
  const float Kd_;
  float       prev_error_;
  const float max_speed_;
  const float min_speed_;
};

// ── entry point ───────────────────────────────────────────────────────────────

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<WallFollower>());
  rclcpp::shutdown();
  return 0;
}
