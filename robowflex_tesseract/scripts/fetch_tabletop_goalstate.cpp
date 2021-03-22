/* Author: Carlos Quintero Pena*/

#include <robowflex_library/util.h>
#include <robowflex_library/log.h>
#include <robowflex_library/detail/fetch.h>
#include <robowflex_library/io.h>
#include <robowflex_library/io/visualization.h>
#include <robowflex_library/scene.h>
#include <robowflex_library/yaml.h>
#include <robowflex_tesseract/trajopt_planner.h>

using namespace robowflex;

/* \file fetch_tabletop_goalstate.cpp
 * A simple script that shows how to use TrajOpt to plan in a manipulation task. The scene and request are
 * loaded from yaml files. An RVIZ Helper object is used to visualize the start/goal states and the computed
 * trajectory.
 */

static const std::string GROUP = "arm";

int main(int argc, char **argv)
{
    // Startup ROS
    ROS ros(argc, argv);

    // Create the default Fetch robot.
    auto fetch = std::make_shared<FetchRobot>();
    fetch->initialize(false);

    // Load tabletop scene.
    auto scene = std::make_shared<Scene>(fetch);
    scene->fromYAMLFile("package://robowflex_tesseract/scenes/table/scene.yaml");

    // Attach object to end effector.
    scene->attachObject(*fetch->getScratchState(), "Can1");
    fetch->getScratchState() =
        std::make_shared<robot_state::RobotState>(scene->getScene()->getCurrentState());

    // Create a TrajOpt planner for Fetch.
    auto planner = std::make_shared<TrajOptPlanner>(fetch, GROUP);
    planner->initialize("torso_lift_link", "gripper_link");

    // Set planner parameters.
    planner->options.num_waypoints = 8;  // Select number of waypoints in trajectory
    planner->setInitType(trajopt::InitInfo::Type::JOINT_INTERPOLATED);  // Initialize using a straight-line
                                                                        // between start and goal in C-space.

    // Load request.
    const auto &request = std::make_shared<MotionRequestBuilder>(fetch);
    request->fromYAMLFile("package://robowflex_tesseract/scenes/table/request.yaml");

    // RVIZ helper.
    const auto &rviz = std::make_shared<IO::RVIZHelper>(fetch);
    rviz->updateScene(scene);
    rviz->visualizeState(request->getStartConfiguration());

    RBX_INFO("Visualizing start state");
    RBX_INFO("Press Enter to continue");
    std::cin.ignore();

    // Do motion planning.
    const auto &res = planner->plan(scene, request->getRequest());
    if (res.error_code_.val == moveit_msgs::MoveItErrorCodes::SUCCESS)
        rviz->updateTrajectory(res);

    rviz->visualizeState(request->getGoalConfiguration());

    RBX_INFO("Visualizing goal state");
    RBX_INFO("Press Enter to exit");
    std::cin.ignore();

    return 0;
}
