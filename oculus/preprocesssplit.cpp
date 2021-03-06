#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
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

    // find the new dimensions to match up to the oculus rift ratio
    int new_width = 576; // 9 / 10 of the original width
    int new_height = new_width * subImage1.rows / subImage1.cols;

    // resize both images
    cv::Mat resizedLeft, resizedRight;
    cv::resize(subImage1, resizedLeft, cv::Size(new_width, new_height));
    cv::resize(subImage2, resizedRight, cv::Size(new_width, new_height));

    // create the left image
    cv::Mat leftImage(800, 640, CV_8UC3);
    int xoffset = 640 - new_width;          // width = 1280
    int yoffset = 400 - (new_height / 2);   // height = 800


    std::cout << "offsets: " << xoffset << "," << yoffset << std::endl;
    std::cout << "width: " << new_width << "," << new_height << std::endl;


    resizedLeft.copyTo(leftImage(cv::Rect(xoffset, yoffset, new_width, new_height)));

    // create the right image
    cv::Mat rightImage(800, 640, CV_8UC3);
    resizedRight.copyTo(rightImage(cv::Rect(0, yoffset, new_width, new_height)));

    cv::namedWindow("Left");
    cv::imshow("Left", leftImage);

    cv::namedWindow("Right");
    cv::imshow("Right", rightImage);

    cv::waitKey(0);

    return 0;
}

