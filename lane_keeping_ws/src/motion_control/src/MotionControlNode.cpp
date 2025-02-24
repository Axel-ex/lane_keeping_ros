#include "MotionControlNode.hpp"
#include "eigen3/Eigen/Dense"

using namespace rclcpp;

MotionControlNode::MotionControlNode() : Node("motion_control_node")
{
    lane_pos_sub_ = this->create_subscription<lane_msgs::msg::LanePositions>(
        "lane_position", 10,
        [this](lane_msgs::msg::LanePositions::SharedPtr lane_msg)
        { MotionControlNode::processLanePosition(lane_msg); });
    velocity_pub_ =
        this->create_publisher<geometry_msgs::msg::Twist>("cmd_vel", 10);
    status_pub_ =
        create_publisher<std_msgs::msg::String>("motion_control_status", 10);

    declare_parameter("kp", 0.5);
    declare_parameter("ki", 0.1);
    declare_parameter("kd", 0.2);
    declare_parameter("base_speed", 0.2);
    declare_parameter("lookahead_index", 10);
    declare_parameter("bucket_size", 10);
    pid_controller_.initializePID(get_parameter("kp").as_double(),
                                  get_parameter("ki").as_double(),
                                  get_parameter("kd").as_double());
}

void MotionControlNode::processLanePosition(
    lane_msgs::msg::LanePositions::SharedPtr lane_msg)
{

    auto left_lane_points = filterLanePositions(lane_msg->left_lane);
    auto right_lane_points = filterLanePositions(lane_msg->right_lane);

    // TODO: Take action if no line is detected
    if (left_lane_points.empty() || right_lane_points.empty())
    {
        stopVehicle();
        return;
    }

    auto left_buckets = extractBuckets(left_lane_points);
    auto right_buckets = extractBuckets(right_lane_points);

    // TODO:
    // Filter out points
    // Estimate position and steering action
    // Integrate the error.
}

std::vector<geometry_msgs::msg::Point32> MotionControlNode::filterLanePositions(
    std::vector<geometry_msgs::msg::Point32>& points)
{
}

std::vector<std::vector<geometry_msgs::msg::Point32>>
MotionControlNode::extractBuckets(
    std::vector<geometry_msgs::msg::Point32>& points)
{
    int bucket_size = get_parameter("bucket_size").as_int();
    std::vector<std::vector<geometry_msgs::msg::Point32>> result;

    if (points.empty())
        return result;

    std::vector<geometry_msgs::msg::Point32> bucket;
    int upper_limit = points.front().y + bucket_size;

    for (const auto& point : points)
    {
        if (point.y > upper_limit)
        {
            result.push_back(std::move(bucket));
            bucket.clear();
            upper_limit += bucket_size;
        }
        bucket.push_back(point);
    }
    // Last bucket
    if (!bucket.empty())
        result.push_back(std::move(bucket));

    return result;
}

void MotionControlNode::stopVehicle()
{
    geometry_msgs::msg::Twist msg;

    msg.linear.x = 0.0;
    msg.angular.z = 0.0;
    velocity_pub_->publish(msg);
}
