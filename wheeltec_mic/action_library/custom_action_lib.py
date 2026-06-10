from time import sleep
# from control_robot_arms import control_robot_arms
from action_library.control_robot_hands import make_hands_gesture
from action_library.control_robot_arms import control_robot_arms

def shake_hand():
    # 定义关节角度
    left = [0.0651, 0.1441, -0.0695, -0.1913, -0.0048, 0.0022, -0.0020]
    right = [-0.1486, 0.0380, -0.0839, -0.5303, 0.2176, -0.8474, 0.0054]

    # 控制双臂运动
    control_robot_arms(left, right, speed=0.5)
    make_hands_gesture("shake")
    sleep(2)
    # make_hands_gesture("open")

def original_pose():
    # 定义关节角度
    left = [0.0651, 0.1441, -0.0695, -0.1913, -0.0048, 0.0022, -0.0020]
    right = [0.0502, -0.1420, 0.0737, -0.2079, 0.0116, -0.0026, -0.0058]
    make_hands_gesture("open")
    # 控制双臂运动
    control_robot_arms(left, right, speed=0.5)

def make_gesture(gesture_name):

    if gesture_name == "shake":
        shake_hand()
        sleep(1)
        original_pose()

# if __name__ == "__main__":
#     make_gesture("shake")
