# Hand Eye Calibration

要充分了解手眼校准，请参阅 [tutorial](https://zivid.atlassian.net/wiki/spaces/ZividKB/pages/72450049) in our Knowledge Base.

-----------------

[SampleHandEyeCalibration]([SampleHandEyeCalibration-url]):

* 通过收集标定姿态的应用程序
   1. 为应用程序提供姿态(手动输入);
   2. 应用程序获取校准对象的图片并计算姿态;
   3. 移动机器人到新的位置, 输入指令添加新姿态;
   4. 重复1.-3.直到收集 10-20 个姿势对;
   5. 输入指令执行手眼标定并返回一个 **Transformation Matrix**;

[ZividHandEyeCalibration](C:\Program Files\Zivid\bin\ZividHandEyeCalibration.exe): (no source)

* 采取了一组姿态对的应用程序(e.g. output of steps 1.-3. in [[SampleHandEyeCalibration]([SampleHandEyeCalibration-url])) 并返回一个 **Transformation Matrix**])

-----------------
以下程序假设存在一个 **Transformation Matrix**

[**UtilizeEyeInHandCalibration**]([UtilizeEyeInHandCalibration-url]):

* 演示如何将摄像机坐标系中的位置和旋转(姿态)转换为机器人坐标系
* 示范用例 - "Bin Picking":
   1. 用Zivid相机获取目标点云;
   2. 为目标获取最优的拾取姿态并**transform to robot co-ordinate system**;
   3. 利用变换后的姿态计算机器人路径并执行拾取;

--------------------
[**PoseConversions**]([PoseConversions-url]):

* Zivid主要使用一个(4x4)变换矩阵(旋转矩阵+平移向量). 这个例子展示如何使用Eigen在 AxisAngle、Rotation Vector、Roll-Pitch-Yaw 和 Quaternion之间相互转化.

--------------------

[SampleHandEyeCalibration-url](https://www.zivid.com/hubfs/softwarefiles/releases/1.6.0+7a245bbe-26/doc/cpp/zivid_sample_code.html#autotoc_md10)
[UtilizeEyeInHandCalibration-url](https://github.com/zivid/cpp-extra-samples/blob/master/Applications/Advanced/HandEyeCalibration/UtilizeEyeInHandCalibration/UtilizeEyeInHandCalibration.cpp)
[PoseConversions-url](https://github.com/zivid/cpp-extra-samples/blob/master/Applications/Advanced/HandEyeCalibration/PoseConversions/PoseConversions.cpp)