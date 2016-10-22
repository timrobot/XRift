#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include "highgui.h"
#include "ovr.h"

int main(int argc, const char** argv){
    cv::Mat img = cv::imread(argv[1], 0);

    if(img.empty()){
        return -1;
    }
    int height = img.rows;
    int width = img.cols;

    std::cout << "height is: " <<height;
    
    return 0;
}

