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
    std::cout << "Usage:"
              << std::endl
              << "  -h            Show this help message" << std::endl
              << "  -s            Show images while processing" << std::endl
              << "  -c ID         Capture images from camera ID rather than command line" << std::endl
              << "  -n NUM        Capture NUM images before running" << std::endl
              << "  -x WIDTH      Calibration chessboard width" << std::endl
              << "  -y HEIGHT     Calibration chessboard height" <<std::endl
              << "  -e NUM        Calibration chessboard square size (arb. units)" << std::endl;
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

bool parseOptions(int argc, char** argv,
                  int& camera, int& n_images,
                  bool& showImages, bool& showHelp,
                  int& board_width,
                  int& board_height,
                  float& squareSize,
                  std::string& file,
                  std::vector<std::string>& files)
{
    camera = -1;
    n_images = 10;

    showImages = false;
    showHelp = false;
    
    board_width = 6;
    board_height = 9;
    squareSize = 1.0f;
    file = "";

    opterr = 0;
    int c;

    while ((c = getopt(argc, argv, "sho:c:x:y:e:")) != -1) {
        switch(c) {
        case 's':
            showImages = true;
            break;

        case 'h':
            printHelp();
            showHelp = true;
            break;
	    
        case 'o':
            file = std::string(optarg);
            break;

        case 'c':
            argToInt(optarg, camera);
            break;

        case 'n':
            argToInt(optarg, n_images);
            break;

        case 'x':
            argToInt(optarg, board_width);
            break;

        case 'y':
            argToInt(optarg, board_height);
            break;

        case 'e':
            try { squareSize = std::stof(optarg); }
            catch(std::invalid_argument error) {
                std::cerr << "ERROR: '"
                          << optarg
                          << "' is not a valid number; ignoring..."
                          << std::endl;
            }
            break;
                
        case '?':
            std::cerr << "Unknown option -"
                      << optopt
                      << "; ignoring..."
                      << std::endl;
            break;
	    
        default:
            return false;
        }
    }

    for (int i=optind; i<argc; i++) {
        files.push_back(std::string(argv[i]));
    }

    return true;    
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool getCameraImages(std::vector<cv::Mat>& images,
                     int cameraId,
                     int nImages)
{
    cv::VideoCapture camera(cameraId);
    if (!camera.isOpened()) {
        std::cerr << "ERROR: failed to open camera " << cameraId << "!" << std::endl;
        return false;
    }

    cv::Mat frame, display;
    std::string text = "0/";
    text += std::to_string(nImages);
    
    while (images.size() < nImages) {
        camera >> frame;
        frame.copyTo(display);
        cv::putText(display, text, cv::Point(0,20), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255,255,255), 2);
        cv::imshow("Frame", display);
        if (cv::waitKey(10) != -1) {
            cv::cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
            images.push_back(frame);

            text = std::to_string(images.size());
            text += "/";
            text += std::to_string(nImages);
        }
    }

    return true;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

std::vector<cv::Mat> getFileImages(std::vector<std::string> imageFiles)
{
    std::vector<cv::Mat> images;
    
    for (int i=0; i<imageFiles.size(); i++) {
        cv::Mat img = cv::imread(imageFiles[i], cv::IMREAD_GRAYSCALE);
        if (img.empty()) {
            std::cerr << "WARNING: failed to read image '"
                      << imageFiles[i]
                      << "'; continuing..."
                      << std::endl;
            continue;
        }
        images.push_back(img);
    }

    return images;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

std::vector<std::vector<cv::Point2f>> getImagePoints(std::vector<cv::Mat> images,
                                                     cv::Size boardSize,
                                                     bool showImages)
{
    std::vector<std::vector<cv::Point2f>> imagePoints;
    
    for (int i=0; i<images.size(); i++) {
        cv::Mat img = images[i];
        std::vector<cv::Point2f> points;
        
        bool patternFound = cv::findChessboardCorners(img,
                                                      boardSize,
                                                      points);
        if (patternFound) {
            cv::cornerSubPix(img, points,
                             cv::Size(11, 11),
                             cv::Size(-1, -1),
                             cv::TermCriteria(cv::TermCriteria::Type::EPS +
                                              cv::TermCriteria::MAX_ITER,
                                              30, 0.1));
            imagePoints.push_back(points);
        }
        
        if (showImages) {
            cv::drawChessboardCorners(img, boardSize, cv::Mat(points), patternFound);
				  
            cv::imshow("Frame", img);
            cv::waitKey();
        }
    }

    return imagePoints;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

std::vector<std::vector<cv::Point3f>> getObjectPoints(int nObjects,
                                                      cv::Size boardSize,
                                                      float squareSize)
{
    std::vector<cv::Point3f> corners;
    for (int i=0; i<boardSize.height; i++) {
        for (int j=0; j<boardSize.width; j++) {
            corners.push_back(cv::Point3f(j*squareSize, i*squareSize, 0));
        }
    }

    std::vector<std::vector<cv::Point3f>> objectPoints(nObjects, corners);
    return objectPoints;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int main(int argc, char** argv)
{
    int camera, n_images;
    bool showImages, showHelp;
    int width, height;
    float squareSize;
    std::string settingsfile;
    std::vector<std::string> imageFiles;
    if (!parseOptions(argc, argv,
                      camera, n_images,
                      showImages, showHelp,
                      width, height, squareSize,
                      settingsfile,
                      imageFiles)) {
        std::cerr << "FATAL: bad options!" << std::endl;
        printHelp();
        return -1;
    }

    // quit without running if showing help
    if (showHelp) { return 0; }

    if (settingsfile == "") {
        settingsfile = "calibration.yaml";
    }

    std::cout << "Camera: " << camera << std::endl;
    std::cout << "Settings: " << settingsfile << std::endl;
    std::cout << "Board Size: " << width << "x" << height << std::endl;
    std::cout << "Images: [ ";
    for (int i=0; i<imageFiles.size(); i++) {
        std::cout << imageFiles[i] << ", ";
    }
    std::cout << "]" << std::endl;

    // get calibration images
    std::vector<cv::Mat> images;
    
    if (camera >= 0) {
        getCameraImages(images, camera, n_images);
    }
    else {
        images = getFileImages(imageFiles);
    }

    std::vector<std::vector<cv::Point2f>>
        imagePoints = getImagePoints(images, cv::Size(width, height), showImages);

    if (imagePoints.size() < 1) {
        std::cerr << "ERROR: failed to find at least one"
                  << " chessboard image; aborting calibration!"
                  << std::endl;
        return 2;
    }

    std::vector<std::vector<cv::Point3f>>
        objectPoints = getObjectPoints(imagePoints.size(), cv::Size(width, height), squareSize);
    
    return 0;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
