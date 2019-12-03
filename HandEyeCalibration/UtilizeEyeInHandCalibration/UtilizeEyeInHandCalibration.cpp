/*
Utilize the result of eye-in-hand calibration to transform (picking) point
coordinates from the camera frame to the robot base frame.
*/

#include <Zivid/CloudVisualizer.h>
#include <Zivid/Zivid.h>

#include <Eigen/Core>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <cmath>
#include <iostream>

Eigen::MatrixXd cvToEigen(const cv::Mat &cvMat)
{
    if(cvMat.dims > 2)
    {
        throw std::invalid_argument("Invalid matrix dimensions. Expected 2D.");
    }

    Eigen::MatrixXd eigenMat(cvMat.rows, cvMat.cols);

    for(int i = 0; i < cvMat.rows; i++)
    {
        for(int j = 0; j < cvMat.cols; j++)
        {
            eigenMat(i, j) = cvMat.at<double>(i, j);
        }
    }

    return eigenMat;
}

cv::Mat readTransform(const std::string &file_name)
{
    auto fileStorage = cv::FileStorage();

    if(!fileStorage.open(file_name, cv::FileStorage::Mode::READ))
    {
        throw std::invalid_argument("Could not open " + file_name);
    }
    try
    {
        const auto poseStateNode = fileStorage["PoseState"];

        if(poseStateNode.empty())
        {
            throw std::invalid_argument("PoseState not found in file " + file_name);
        }

        const auto rows = poseStateNode.mat().rows;
        const auto cols = poseStateNode.mat().cols;
        if(rows != 4 || cols != 4)
        {
            throw std::invalid_argument("Expected 4x4 matrix in " + file_name + ", but got " + std::to_string(cols)
                                        + "x" + std::to_string(rows));
        }

        const auto poseState = poseStateNode.mat();
        fileStorage.release();
        return poseState;
    }
    catch(...)
    {
        fileStorage.release();
        throw;
    }
}

enum class Axis
{
    X,
    Y,
    Z
};

template<Axis axis>
float getValue(const Zivid::Point &p);

template<>
float getValue<Axis::X>(const Zivid::Point &p)
{
    return p.x;
}

template<>
float getValue<Axis::Y>(const Zivid::Point &p)
{
    return p.y;
}

template<>
float getValue<Axis::Z>(const Zivid::Point &p)
{
    return p.z;
}

template<Axis axis>
bool isLesserOrNan(const Zivid::Point &a, const Zivid::Point &b)
{
    return getValue<axis>(a) < getValue<axis>(b) ? true : std::isnan(getValue<axis>(a));
}

template<Axis axis>
bool isGreaterOrNaN(const Zivid::Point &a, const Zivid::Point &b)
{
    return getValue<axis>(a) > getValue<axis>(b) ? true : std::isnan(getValue<axis>(a));
}

int main()
{
    try
    {
        // define (picking) point in camera frame
        //const Eigen::Vector4d pointInCameraFrame(81.2, 18.0, 594.6, 1);
        //std::cout << "Point coordinates in camera frame: " << pointInCameraFrame.segment(0, 3).transpose() << std::endl;

        // Read camera pose in end-effector frame (result of eye-in-hand calibration)
        const auto eyeInHandTransformation = readTransform("handEyeTransform.yaml");

        // Read end-effector pose in robot base frame
        const auto endEffectorPose = readTransform("robotTransform.yaml");

        // convert to Eigen matrices for easier computation
        const auto transformEndEffectorToCamera = cvToEigen(eyeInHandTransformation);
        const auto transformBaseToEndEffector = cvToEigen(endEffectorPose);

        // Compute camera pose in robot base frame
        const auto transform_base_to_camera = transformBaseToEndEffector * transformEndEffectorToCamera;

		Zivid::Application zivid;

		std::string Filename = "zividgem.zdf";
        std::cout << "Reading " << Filename << " point cloud" << std::endl;
        Zivid::Frame frame = Zivid::Frame(Filename);

		std::cout << "Setting up visualization" << std::endl;
        Zivid::CloudVisualizer vis;
        zivid.setDefaultComputeDevice(vis.computeDevice());

		std::cout << "Displaying the frame" << std::endl;
        vis.showMaximized();
        vis.show(frame);
        vis.resetToFit();

        std::cout << "Running the visualizer. Blocking until the window closes" << std::endl;
        vis.run();

        // compute (picking) point in robot base frame
        /*const auto pointInBaseFrame = transform_base_to_camera * pointInCameraFrame;
        std::cout << "Point coordinates in robot base frame: " << pointInBaseFrame.segment(0, 3).transpose()
                  << std::endl;*/

		Eigen::Vector4d pointInCameraFrame(0, 0, 0, 1);
        Eigen::Vector4d pointInBaseFrame;
        std::cout << "Point coordinates in robot base frame: " << pointInCameraFrame << std::endl;
        std::cout << "Point coordinates in robot base frame: " << pointInBaseFrame << std::endl;
        pointInCameraFrame(0) = 11;
        pointInCameraFrame(1) = 22;
        pointInCameraFrame(2) = 33;
        pointInBaseFrame = transform_base_to_camera * pointInCameraFrame;
        std::cout << "Point coordinates in robot base frame: " << pointInCameraFrame << std::endl;
        std::cout << "Point coordinates in robot base frame: " << pointInBaseFrame << std::endl;

		// Extracting point cloud from the frame
        const auto pointCloud = frame.getPointCloud();

		Eigen::MatrixXf xe(pointCloud.height(), pointCloud.width());
        Eigen::MatrixXf ye(pointCloud.height(), pointCloud.width());
        Eigen::MatrixXf ze(pointCloud.height(), pointCloud.width());
        Eigen::MatrixXi re(pointCloud.height(), pointCloud.width());
        Eigen::MatrixXi ge(pointCloud.height(), pointCloud.width());
        Eigen::MatrixXi be(pointCloud.height(), pointCloud.width());
        Eigen::MatrixXf contraste(pointCloud.height(), pointCloud.width());

        for(size_t i = 0; i < pointCloud.height(); i++)
        {
            for(size_t j = 0; j < pointCloud.width(); j++)
            {
                pointInCameraFrame(0) = pointCloud(i, j).x;
                pointInCameraFrame(1) = pointCloud(i, j).y;
                pointInCameraFrame(2) = pointCloud(i, j).z;
                pointInBaseFrame = transform_base_to_camera * pointInCameraFrame;

                xe(i, j) = pointInCameraFrame(0);
                ye(i, j) = pointInCameraFrame(1);
                ze(i, j) = pointInCameraFrame(2);
                re(i, j) = pointCloud(i, j).red();
                ge(i, j) = pointCloud(i, j).green();
                be(i, j) = pointCloud(i, j).blue();
                contraste(i, j) = pointCloud(i, j).contrast;
            }
        }

        std::cout << "Point cloud information:" << std::endl;
        std::cout << "Number of points: " << pointCloud.size() << "\n"
                  << "Height: " << pointCloud.height() << ", Width: " << pointCloud.width() << std::endl;

		

        // Iterating over the point cloud and displaying (X, Y, Z, R, G, B, Contrast)
        for(int i = 0; i < pointCloud.height(); i++)
        {
            for(int j = 0; j < pointCloud.width(); j++)
            {
                //const auto &point = pointCloud(i, j);

                /*std::cout << "Values at pixel (" << i << ", " << j << "):"
                          << "    X:" << point.x << "  Y:" << point.y << "  Z:" << point.z
                          << "    R:" << (int)point.red() << "  G:" << (int)point.green() << "  B:" << (int)point.blue()
                          << "    Contrast:" << point.contrast << std::endl;*/

            }
        }

		std::cout << "Converting ZDF point cloud to OpenCV format" << std::endl;

        // Creating OpenCV structure
        
        cv::Mat rgb((int)pointCloud.height(), (int)pointCloud.width(), CV_8UC3, cv::Scalar(0, 0, 0));
        cv::Mat x((int)pointCloud.height(), (int)pointCloud.width(), CV_8UC1, cv::Scalar(0));
        cv::Mat y((int)pointCloud.height(), (int)pointCloud.width(), CV_8UC1, cv::Scalar(0));
        cv::Mat z((int)pointCloud.height(), (int)pointCloud.width(), CV_8UC1, cv::Scalar(0));

        // Getting min and max values for X, Y, Z images
        auto maxX =
            std::max_element(pointCloud.dataPtr(), pointCloud.dataPtr() + pointCloud.size(), isLesserOrNan<Axis::X>);
        auto minX =
            std::max_element(pointCloud.dataPtr(), pointCloud.dataPtr() + pointCloud.size(), isGreaterOrNaN<Axis::X>);
        auto maxY =
            std::max_element(pointCloud.dataPtr(), pointCloud.dataPtr() + pointCloud.size(), isLesserOrNan<Axis::Y>);
        auto minY =
            std::max_element(pointCloud.dataPtr(), pointCloud.dataPtr() + pointCloud.size(), isGreaterOrNaN<Axis::Y>);
        auto maxZ =
            std::max_element(pointCloud.dataPtr(), pointCloud.dataPtr() + pointCloud.size(), isLesserOrNan<Axis::Z>);
        auto minZ =
            std::max_element(pointCloud.dataPtr(), pointCloud.dataPtr() + pointCloud.size(), isGreaterOrNaN<Axis::Z>);

		

        // Filling in OpenCV matrices with the cloud data
        for(size_t i = 0; i < pointCloud.height(); i++)
        {
            for(size_t j = 0; j < pointCloud.width(); j++)
            {
                cv::Vec3b &color = rgb.at<cv::Vec3b>(i, j);
                color[0] = pointCloud(i, j).blue();
                color[1] = pointCloud(i, j).green();
                color[2] = pointCloud(i, j).red();

                if(std::isnan(pointCloud(i, j).z))
                {
                    x.at<uchar>(i, j) = 0;
                    y.at<uchar>(i, j) = 0;
                    z.at<uchar>(i, j) = 0;
                }
                else
                {
                    pointInCameraFrame(0) = pointCloud(i, j).x;
                    pointInCameraFrame(1) = pointCloud(i, j).y;
                    pointInCameraFrame(2) = pointCloud(i, j).z;
                    pointInBaseFrame = transform_base_to_camera * pointInCameraFrame;

					size_t XXX = pointInBaseFrame(0);
                    size_t YYY = pointInBaseFrame(1);
                    size_t ZZZ = pointInBaseFrame(2);



                    x.at<uchar>(i, j) =
                        static_cast<unsigned char>((255.0f * (XXX - minX->x) / (maxX->x - minX->x)));
                    y.at<uchar>(i, j) =
                        static_cast<unsigned char>((255.0f * (YYY - minY->y) / (maxY->y - minY->y)));
                    z.at<uchar>(i, j) =
                        static_cast<unsigned char>((255.0f * (ZZZ - minZ->z) / (maxZ->z - minZ->z)));
                }
            }
        }

        // Applying color map
        cv::Mat xJetColorMap, yJetColorMap, zJetColorMap;
        cv::applyColorMap(x, xJetColorMap, cv::COLORMAP_JET);
        cv::applyColorMap(y, yJetColorMap, cv::COLORMAP_JET);
        cv::applyColorMap(z, zJetColorMap, cv::COLORMAP_JET);

        // Setting nans to black
        for(size_t i = 0; i < pointCloud.height(); i++)
        {
            for(size_t j = 0; j < pointCloud.width(); j++)
            {
                if(std::isnan(pointCloud(i, j).z))
                {
                    cv::Vec3b &xRGB = xJetColorMap.at<cv::Vec3b>(i, j);
                    xRGB[0] = 0;
                    xRGB[1] = 0;
                    xRGB[2] = 0;

                    cv::Vec3b &yRGB = yJetColorMap.at<cv::Vec3b>(i, j);
                    yRGB[0] = 0;
                    yRGB[1] = 0;
                    yRGB[2] = 0;

                    cv::Vec3b &zRGB = zJetColorMap.at<cv::Vec3b>(i, j);
                    zRGB[0] = 0;
                    zRGB[1] = 0;
                    zRGB[2] = 0;
                }
            }
        }

        // Displaying the Depth image
        cv::namedWindow("Depth map", cv::WINDOW_AUTOSIZE);
        cv::imshow("Depth map", zJetColorMap);
        cv::waitKey(0);

        // Saving the Depth map
        cv::imwrite("Depth map.jpg", zJetColorMap);
    
    }

    catch(const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}