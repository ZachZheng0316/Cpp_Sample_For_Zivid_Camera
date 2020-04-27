#include <Zivid/Application.h>
#include <Zivid/Exception.h>
#include <Zivid/HandEye/Calibrate.h>
#include <Zivid/HandEye/Detector.h>
#include <Zivid/HandEye/Pose.h>

#include <iostream>

namespace
{
	// 命令类型
    enum class CommandType
    {
        cmdAddPose,
        cmdCalibrate,
        cmdUnknown
    };

	// 获取输入
    std::string getInput()
    {
        std::string command;
        std::getline(std::cin, command);
        return command;
    }

	/* 函数意义: 输入操作指令; 
	 * 		- "p"/"P": 表示添加机器人姿态指令；
	 *		- "c"/"C": 表示执行手眼标定操作；
	 * 参数意义：无参数
	 * 返回值：返回操作的指令；
	 */
    CommandType enterCommand()
    {
		// p:表示添加机器人姿态; c: 表示执行手眼标定操作
        std::cout << "Enter command, p (to add robot pose) or c (to perform calibration): ";
        const auto command = getInput();

        if(command == "P" || command == "p")
        {
            return CommandType::cmdAddPose;
        }
        if(command == "C" || command == "c")
        {
            return CommandType::cmdCalibrate;
        }
        return CommandType::cmdUnknown;
    }

	/* 函数意义: 输入机器人位姿; 每个姿态带有一个id, 每个位姿用16个空格分开的值描述4*4-主矩阵的行;
	 * 参数意义：
	 *		- [in]index: 位姿的标签;
	 * 返回值：机器人的位姿；
	 */
    Zivid::HandEye::Pose enterRobotPose(size_t index)
    {
		// 输入带id的位姿(用16个空格分开的值描述4x4行-主矩阵的行)
        std::cout << "Enter pose with id (a line with 16 space separated values describing 4x4 row-major matrix) : "
                  << index << std::endl;
        std::stringstream input(getInput());
        double element{ 0 };
        std::vector<double> transformElements;
        for(size_t i = 0; i < 16 && input >> element; ++i)
        {
            transformElements.emplace_back(element);
        }

        const auto robotPose{ Zivid::Matrix4d{ transformElements.cbegin(), transformElements.cend() } };
        std::cout << "The following pose was entered: \n" << robotPose << std::endl;

        return robotPose;
    }

	/* 函数意义: 获取棋盘格点框架; 
	 * 参数意义：
	 *		- [in]camera: 棋盘image;
	 * 返回值：captureFrame；
	 */
    Zivid::Frame acquireCheckerboardFrame(Zivid::Camera &camera)
    {
        std::cout << "Capturing checkerboard image... " << std::flush;
        auto settings{ Zivid::Settings{} };
        settings.set(Zivid::Settings::Iris{ 17 });
        settings.set(Zivid::Settings::Gain{ 1.0 });
        settings.set(Zivid::Settings::Brightness{ 1.0 });
        settings.set(Zivid::Settings::ExposureTime{ std::chrono::microseconds{ 20000 } });
        settings.set(Zivid::Settings::Filters::Gaussian::Enabled::yes);
        camera.setSettings(settings);
        const auto frame = camera.capture();
        std::cout << "OK" << std::endl;

        return frame;
    }
} // namespace

int main()
{
    try
    {
        Zivid::Application zivid;

		// 连接相机
        std::cout << "Connecting to camera..." << std::endl;
        auto camera{ zivid.connectCamera() };

		// 输入机器人的位姿
        size_t currPoseId{ 0 };
        bool calibrate{ false };
        std::vector<Zivid::HandEye::CalibrationInput> input;
        do
        {
            switch(enterCommand())
            {
                case CommandType::cmdAddPose:	// 添加机器人姿态的指令
                {
                    try
                    {
                        const auto robotPose = enterRobotPose(currPoseId);

                        const auto frame = acquireCheckerboardFrame(camera);

						// 检测棋盘格的方形中心
                        std::cout << "Detecting checkerboard square centers... " << std::flush;
                        const auto result = Zivid::HandEye::detectFeaturePoints(frame.getPointCloud());
                        if(result)
                        {
                            std::cout << "OK" << std::endl;
                            input.emplace_back(Zivid::HandEye::CalibrationInput{ robotPose, result });
                            currPoseId++;
                        }
                        else
                        {
                            std::cout << "FAILED" << std::endl;
                        }
                    }
                    catch(const std::exception &e)
                    {
                        std::cout << "Error: " << Zivid::toString(e) << std::endl;
                        continue;
                    }
                    break;
                }
                case CommandType::cmdCalibrate:	// 进行手眼标定的资料
                {
                    calibrate = true;
                    break;
                }
                case CommandType::cmdUnknown:	// 未知指令
                {
                    std::cout << "Error: Unknown command" << std::endl;
                    break;
                }
            }
        } while(!calibrate);

		// 执行手眼标定
        std::cout << "Performing hand-eye calibration ... " << std::flush;
        const auto calibrationResult{ Zivid::HandEye::calibrateEyeToHand(input) };
        if(calibrationResult)
        {
            std::cout << "OK\n"
                      << "Result:\n"
                      << calibrationResult << std::endl;
        }
        else
        {
            std::cerr << "\nFAILED" << std::endl;
            return EXIT_FAILURE;
        }
    }
    catch(const std::exception &e)
    {
        std::cerr << "\nError: " << Zivid::toString(e) << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
