#include <iostream>
#include <string>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/highgui.hpp>

// POSIX arguments
extern "C" {
#include <unistd.h>
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void printHelp()
{
    std::cout << "Usage undistort [OPTIONS] img1 [img2 [img3 ...]]" << std::endl
              << "  -h            Display this help message and exit" << std::endl
              << "  -f FILE       File to load the camera calibration from (default" << std::endl
              << "                'calibration.yaml')" << std::endl
              << "  -c ID         Camera to view live. Using this option ignores" << std::endl
              << "                any images provided on the command line and simply" << std::endl
              << "                shows the live feed." << std::endl;
}
    

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool argToInt(char* arg, int& integer)
{
    try { integer = std::stoi(arg); }
    catch(std::invalid_argument error) {
        std::cerr << "ERROR: '"
                  << arg
                  << "' is not a valid integer; ignoring..."
                  << std::endl;
        return false;
    }
    return true;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool parseOptions(int argc, char** argv,
                  int& camera,
                  std::string& file,
                  std::vector<std::string>& imageFiles)
{
    camera = -1;
    file = "calibration.yaml";

    opterr = 0;
    int c;

    while ((c = getopt(argc, argv, "hf:c:")) != -1) {
        switch(c) {
        case 'f':
            file = std::string(optarg);
            break;

        case 'c':
            argToInt(optarg, camera);
            break;

        case 'h':
            return false;

        case '?':
            std::cerr << "Unknown option -"
                      << optopt
                      << std::endl;
            return false;

        default:
            return false;
        }
    }

    for (int i=optind; i<argc; i++) {
        imageFiles.push_back(std::string(argv[i]));
    }

    return true;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool loadCalibration(cv::Mat& map1,
                     cv::Mat& map2,
                     std::string calibrationFile)
{
    cv::FileStorage fs(calibrationFile, cv::FileStorage::READ);
    if (!fs.isOpened()) {
        std::cerr << "ERROR: failed to open file '"
                  << calibrationFile
                  << "'!" << std::endl;
        return false;
    }

    cv::Mat cameraMatrix, distCoeffs;
    cv::Size imageSize;
    
    fs["cameraMatrix"] >> cameraMatrix;
    fs["distCoeffs"] >> distCoeffs;
    fs["imageSize"] >> imageSize;

    fs.release();

    cv::initUndistortRectifyMap
        (cameraMatrix, distCoeffs, cv::Mat(),
         cv::getOptimalNewCameraMatrix(cameraMatrix,
                                       distCoeffs,
                                       imageSize, 1,
                                       imageSize, 0),
         imageSize,
         CV_16SC2, map1, map2);

    return true;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void showCamera(int cameraId, cv::Mat map1, cv::Mat map2)
{
    cv::VideoCapture camera(cameraId);
    if (!camera.isOpened()) {
        std::cerr << "ERROR: could not open camera '"
                  << cameraId
                  << "'" << std::endl;
        return;
    }

    cv::Mat frame, undist;
    
    while(true) {
        camera >> frame;
        cv::remap(frame, undist, map1, map2, cv::INTER_LINEAR);
        imshow("Undistorted View", undist);
        cv::waitKey(10);
    }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int main(int argc, char** argv)
{
    int camera;
    std::string calibrationFile;
    std::vector<std::string> imageFiles;

    if (!parseOptions(argc, argv,
                      camera, calibrationFile, imageFiles)) {
        printHelp();
        return 1;
    }
        
    cv::Mat map1, map2;
    if (!loadCalibration(map1, map2, calibrationFile)) {
        return 1;
    }

    if (camera != -1) {
        showCamera(camera, map1, map2);
    }
    else {
        //todo
    }

    return 0;
}
