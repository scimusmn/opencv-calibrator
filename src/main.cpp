#include <iostream>
#include <string>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/calib3d.hpp>

// POSIX arguments
extern "C" {
#include <unistd.h>
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
		  int* camera,
		  int* board_width,
		  int* board_height,
		  char** file,
		  std::vector<std::string>* files)
{
    *camera = 0;
    *board_width = 5;
    *board_height = 9;
    *file = NULL;

    char* camera_id;
    opterr = 0;
    int c;

    while ((c = getopt(argc, argv, "o:c:w:h:")) != -1) {
	switch(c) {
	case 'o':
	    *file = optarg;
	    break;

	case 'c':
	    argToInt(optarg, camera);
	    break;

	case 'w':
	    argToInt(optarg, board_width);
	    break;

	case 'h':
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
    int camera, width, height;
    char* settingsfile;
    std::vector<std::string> images;
    if (!parseOptions(argc, argv,
		      &camera,
		      &width, &height,
		      &settingsfile,
		      &images)) {
	std::cerr << "FATAL: bad options!" << std::endl;
	return -1;
    }

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

    return 0;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
