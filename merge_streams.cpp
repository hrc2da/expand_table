#include <opencv2/opencv.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

int main() {

    cv::VideoCapture cap1(0);
    if( !cap1.isOpened() ) return 1;

    int width=320;
    int height=240;
    int vidsendsiz = 0;

    int v4l2lo = open("/dev/video4", O_WRONLY);
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
     while (1) {
        cv::Mat frame1;
        cv::Mat frame2;
        cap1.set(CV_CAP_PROP_FRAME_WIDTH,320);
        cap1.set(CV_CAP_PROP_FRAME_HEIGHT,240);

        if (!cap1.grab())
        {
            std::cout << "ERROR READING FRAME FROM CAMERA FEED" << std::endl;
            break;
        }
        cap1>>frame1;
        frame2=frame1;

        cv::Mat combine;
        hconcat(frame1,frame2,combine);

        int siz = combine.total() * combine.elemSize();
        if (siz != vidsendsiz) {
            std::cout << "size != vidsendsiz " << siz << " / " << vidsendsiz << std::endl;
        }
        size_t written = write(v4l2lo, combine.data, siz);
        if (written < 0) {
            std::cout << "Error writing v4l2l device";
            close(v4l2lo);
            return 1;
        }

//        cv::imshow("frame",combine);
        //wait for 40 milliseconds
//        int c = cvWaitKey(40);

     }


}
