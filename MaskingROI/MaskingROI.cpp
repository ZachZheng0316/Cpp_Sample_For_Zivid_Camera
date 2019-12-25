/*
Import a ZDF point cloud, mask it using OpenCV, and convert it to PCL format.
*/

#include <Zivid/CloudVisualizer.h>
#include <Zivid/Zivid.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <pcl/visualization/cloud_viewer.h>

#include <algorithm>
#include <cmath>
#include <iostream>

int main()
{
    try
    {
        std::string filenameZDF = "Zivid3D.zdf";

        Zivid::Application zivid;

        std::cout << "Reading " + filenameZDF << std::endl;
        Zivid::Frame frame(filenameZDF);

        const auto pointCloud = frame.getPointCloud();

        std::cout << "Setting up visualization" << std::endl;
        Zivid::CloudVisualizer vis;
        zivid.setDefaultComputeDevice(vis.computeDevice());

        std::cout << "Displaying the point cloud" << std::endl;
        vis.showMaximized();
        vis.show(frame);
        vis.resetToFit();
        std::cout << "Running the visualizer. Blocking until the window closes" << std::endl;
        vis.run();

        std::cout << "Converting ZDF point cloud to OpenCV format" << std::endl;

        // Creating OpenCV structure
        cv::Mat rgb((int)pointCloud.height(), (int)pointCloud.width(), CV_8UC3, cv::Scalar(0, 0, 0));
        cv::Mat x((int)pointCloud.height(), (int)pointCloud.width(), CV_32F, cv::Scalar(0));
        cv::Mat y((int)pointCloud.height(), (int)pointCloud.width(), CV_32F, cv::Scalar(0));
        cv::Mat z((int)pointCloud.height(), (int)pointCloud.width(), CV_32F, cv::Scalar(0));

        // Filling in OpenCV matrices with the cloud data
        for(size_t i = 0; i < pointCloud.height(); i++)
        {
            for(size_t j = 0; j < pointCloud.width(); j++)
            {
                cv::Vec3b &color = rgb.at<cv::Vec3b>(i, j);
                color[0] = pointCloud(i, j).blue();
                color[1] = pointCloud(i, j).green();
                color[2] = pointCloud(i, j).red();
                x.at<float>(i, j) = pointCloud(i, j).x;
                y.at<float>(i, j) = pointCloud(i, j).y;
                z.at<float>(i, j) = pointCloud(i, j).z;
            }
        }

        // Masking the point cloud
        cv::Mat mask = cv::Mat::zeros(x.size(), CV_8U);

        //Circle
        //cv::circle(mask, cv::Point(mask.cols / 2, mask.rows / 2), 200, cv::Scalar(255, 0, 0), -1, 8, 0);

        //Rectangle
        /*int rectangle_width = 600;
        int rectangle_height = 400;
        cv::rectangle(mask,
                      cv::Point((mask.cols - rectangle_width) / 2, (mask.rows - rectangle_height) / 2),
                      cv::Point((mask.cols + rectangle_width) / 2, (mask.rows + rectangle_height) / 2),
                      cv::Scalar(255, 0, 0),
                      -1,
                      8,
                      0);*/

        //Rectangle2
        cv::Rect rect(660, 400, 600, 400);
        cv::rectangle(mask, rect, cv::Scalar(255, 0, 0), -1, 8, 0);

        cv::Mat xMasked = cv::Mat::zeros(x.size(), x.type());
        cv::Mat yMasked = cv::Mat::zeros(y.size(), y.type());
        cv::Mat zMasked = cv::Mat::zeros(z.size(), z.type());
        cv::Mat rgbMasked = cv::Mat::zeros(rgb.size(), rgb.type());

        x.copyTo(xMasked, mask);
        y.copyTo(yMasked, mask);
        z.copyTo(zMasked, mask);
        rgb.copyTo(rgbMasked, mask);

        // Creating a PointCloud structure
        pcl::PointCloud<pcl::PointXYZRGB> cloud;

        // Filling in the cloud data
        cloud.width = pointCloud.width();
        cloud.height = pointCloud.height();
        cloud.is_dense = false;
        cloud.points.resize(cloud.width * cloud.height);
        for(size_t i = 0; i < cloud.points.size(); ++i)
        {
            cloud.points[i].x = xMasked.at<float>(i);
            cloud.points[i].y = yMasked.at<float>(i);
            cloud.points[i].z = zMasked.at<float>(i);
            cloud.points[i].r = (uint8_t)rgb.at<cv::Vec3b>(i)[2];
            cloud.points[i].g = (uint8_t)rgb.at<cv::Vec3b>(i)[1];
            cloud.points[i].b = (uint8_t)rgb.at<cv::Vec3b>(i)[0];
        }

        //Saving to a .PCD file format
        std::string filenamePCD = "Zivid3D.pcd";
        std::cerr << "Saving " << cloud.points.size() << " data points to " + filenamePCD << std::endl;
        pcl::io::savePCDFileBinary(filenamePCD, cloud);

        pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloudPTR;
        cloudPTR.reset(new pcl::PointCloud<pcl::PointXYZRGB>(cloud));

        //Simple Cloud Visualization
        pcl::visualization::CloudViewer viewer("Simple Cloud Viewer");
        viewer.showCloud(cloudPTR);
        std::cout << "Press r to centre and zoom the viewer so that the entire cloud is visible" << std::endl;
        std::cout << "Press q to me exit the viewer application" << std::endl;
        while(!viewer.wasStopped())
        {
        }

        // Displaying the masked RGB image
        cv::namedWindow("Masked RGB image", cv::WINDOW_AUTOSIZE);
        cv::imshow("Masked RGB image", rgbMasked);
        cv::waitKey(0);

        // Saving the masked RGB image
        cv::imwrite("Masked RGB image.jpg", rgbMasked);
    }
    catch(const std::exception &e)
    {
        std::cerr << "Error: " << Zivid::toString(e) << std::endl;
        return EXIT_FAILURE;
    }
}
