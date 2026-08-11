import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import (
    IncludeLaunchDescription,
    TimerAction
)
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration


def generate_launch_description():

    maze_solver_dir   = get_package_share_directory('maze_solver')
    turtlebot3_gz_dir = get_package_share_directory('turtlebot3_gazebo')
    pkg_gazebo_ros    = get_package_share_directory('gazebo_ros')

    world_file = os.path.join(maze_solver_dir, 'worlds', 'maze_arena.world')
    use_sim_time = LaunchConfiguration('use_sim_time', default='true')

    # 1. Start Gazebo server with our maze world
    gzserver_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(pkg_gazebo_ros, 'launch', 'gzserver.launch.py')
        ),
        launch_arguments={'world': world_file}.items() # type: ignore
    )

    # 2. Start Gazebo client (GUI)
    gzclient_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(pkg_gazebo_ros, 'launch', 'gzclient.launch.py')
        )
    )

    # 3. Robot state publisher
    robot_state_publisher_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(turtlebot3_gz_dir, 'launch', 'robot_state_publisher.launch.py')
        ),
        launch_arguments={'use_sim_time': use_sim_time}.items() # type: ignore
    )

    # 4. Spawn TurtleBot3 — delayed 10s so gzserver factory plugin is fully up
    spawn_turtlebot_cmd = TimerAction(
        period=10.0,
        actions=[
            IncludeLaunchDescription(
                PythonLaunchDescriptionSource(
                    os.path.join(turtlebot3_gz_dir, 'launch', 'spawn_turtlebot3.launch.py')
                ),
                launch_arguments={
                    'x_pose': '-1.6',
                    'y_pose': '1.6',
                    'Y_pose': '0.0',
                }.items() # type: ignore
            )
        ]
    )

    ld = LaunchDescription()
    ld.add_action(gzserver_cmd)
    ld.add_action(gzclient_cmd)
    ld.add_action(robot_state_publisher_cmd)
    ld.add_action(spawn_turtlebot_cmd)

    return ld
