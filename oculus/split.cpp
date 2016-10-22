#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include "highgui.h"
#include "ovr.h"

int main(int argc, const char** argv){
    cv::Mat img = cv::imread(argv[1]);

    if(img.empty() || !img.data){
        printf("no image\n");
        return -1;
    }
    int height = img.rows;
    int width = img.cols;
    int crop = width/20;
    
    printf("Image dimensions: height=%d width=%d crop=%d\n", height, width, crop);

    cv::Mat subImage1(img, cv::Range::all(), cv::Range(0, width-crop)); // Mat dst(Mat src, yrange, xrange)
    cv::Mat subImage2(img, cv::Range::all(), cv::Range(crop, width));

    cv::namedWindow("Left");
    cv::imshow("Left", subImage1);

    cv::namedWindow("Right");
    cv::imshow("Right", subImage2);

    cv::waitKey(0);

    return 0;
}

