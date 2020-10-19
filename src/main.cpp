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
	      << "  -y HEIGHT     Calibration chessboard height" <<std::endl;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool argToInt(char* arg, int* integer)
{
    try { *integer = std::stoi(arg); }
    catch(std::invalid_argument error) {
	std::cerr << "WARNING: '"
			  << arg
			  << "' is not a valid numeric identifier; ignoring..."
			  << std::endl;
	return false;
    }
    return true;
}

bool parseOptions(int argc, char** argv,
		  int* camera, int* n_images,
		  bool* showImages, bool* showHelp,
		  int* board_width,
		  int* board_height,
		  char** file,
		  std::vector<std::string>* files)
{
    *camera = 0;
    *n_images = 10;

    *showImages = false;
    *showHelp = false;
    
    *board_width = 5;
    *board_height = 9;
    *file = NULL;

    char* camera_id;
    opterr = 0;
    int c;

    while ((c = getopt(argc, argv, "sho:c:x:y:")) != -1) {
	switch(c) {
	case 's':
	    *showImages = true;
	    break;

	case 'h':
	    printHelp();
	    *showHelp = true;
	    break;
	    
	case 'o':
	    *file = optarg;
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
	files->push_back(std::string(argv[i]));
    }

    return true;    
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int main(int argc, char** argv)
{
    int camera, n_images;
    bool showImages, showHelp;
    int width, height;
    char* settingsfile;
    std::vector<std::string> images;
    if (!parseOptions(argc, argv,
		      &camera, &n_images,
		      &showImages, &showHelp,
		      &width, &height,
		      &settingsfile,
		      &images)) {
	std::cerr << "FATAL: bad options!" << std::endl;
	return -1;
    }

    // quit without running if showing help
    if (showHelp) { return 0; }

    if (settingsfile == NULL) {
	settingsfile = "calibration.yaml";
    }

    std::cout << "Camera: " << camera << std::endl;
    std::cout << "Settings: " << settingsfile << std::endl;
    std::cout << "Board Size: " << width << "x" << height << std::endl;
    std::cout << "Images: [ ";
    for (int i=0; i<images.size(); i++) {
	std::cout << images[i] << ", ";
    }
    std::cout << "]" << std::endl;

    // process each image
    for (int i=0; i<images.size(); i++) {
	cv::Mat img = cv::imread(images[i], cv::IMREAD_GRAYSCALE);
	if (img.empty()) {
	    std::cerr << "WARNING: failed to read image '"
		      << images[i]
		      << "'; continuing..."
		      << std::endl;
	    continue;
	}
	
	std::vector<cv::Point2f> points;
	bool patternFound = cv::findChessboardCorners(img,
						      cv::Size(width, height),
						      points);
	/*if (patternFound) {
	    cv::cornerSubPix(img, points,
			     cv::Size(11, 11),
			     cv::Size(-1, -1),
			     cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER,
					      30, 0.1));
					      }*/

	if (showImages) {
	    cv::drawChessboardCorners(img, cv::Size(width, height), cv::Mat(points), patternFound);
				  
	    cv::imshow("Frame", img);
	    cv::waitKey();
	}
    }

    return 0;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
