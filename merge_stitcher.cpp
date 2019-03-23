#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/stitching.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>

#include <iostream>
#include <sstream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>


int main()
{
    cv::VideoCapture cap1(0);
    if( !cap1.isOpened() ) return 1;
    cv::VideoCapture cap2(1);
    if( !cap2.isOpened() ) return 1;

    int width=640;
    int height=480;
    int vidsendsiz = 0;

    cv::Mat frame1;
    cv::Mat frame2;
    cv::Mat combine;
    int siz;
    size_t written;

    cap1.set(CV_CAP_PROP_FRAME_WIDTH,width);
    cap1.set(CV_CAP_PROP_FRAME_HEIGHT,height);

    cap2.set(CV_CAP_PROP_FRAME_WIDTH,width);
    cap2.set(CV_CAP_PROP_FRAME_HEIGHT,height);

    int v4l2lo = open("/dev/video2", O_WRONLY);
    if(v4l2lo < 0) {
        std::cout << "Error opening v4l2l device: " << strerror(errno);
        exit(-2);
    }
    struct v4l2_format v;
    int t;
    v.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    t = ioctl(v4l2lo, VIDIOC_G_FMT, &v);
     if( t < 0 ) {
            exit(t);
        }
     v.fmt.pix.width = width * 2;
     v.fmt.pix.height = height; //Does NOT change when merging
     v.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
     vidsendsiz = width * height * 3 * 2;
     v.fmt.pix.sizeimage = vidsendsiz;
     t = ioctl(v4l2lo, VIDIOC_S_FMT, &v);
     if( t < 0 ) {
        exit(t);
     }

    bool try_use_gpu = true;
    cv::Stitcher::Mode mode = cv::Stitcher::PANORAMA; //Stitcher::SCANS;
    std::vector<cv::Mat> imgs;
    cv::Stitcher stitcher = cv::Stitcher::createDefault(true);

    while (true) {
        cap1>>frame1;
        cap2>>frame2;

        imgs.push_back(frame1);
        imgs.push_back(frame2);

        cv::Stitcher::Status status = stitcher.stitch(imgs, combine);

        if (status != cv::Stitcher::OK) {
            std::cout << "Can't stitch images, error code = " << int(status) << std::endl;
            return -1;
        }
        siz = combine.total() * combine.elemSize();
 //       if (siz != vidsendsiz) {
 //           std::cout << "size != vidsendsiz " << siz << " / " << vidsendsiz << std::endl;
 //       }
        written = write(v4l2lo, combine.data, siz);
        if (written < 0) {
            std::cout << "Error writing v4l2l device";
            close(v4l2lo);
            return 1;
        }

        switch (cv::waitKey(10)) {
            case 40:
                return 0;
        }

        imgs.erase(imgs.begin(),imgs.end());

    }

    return 0;
}
