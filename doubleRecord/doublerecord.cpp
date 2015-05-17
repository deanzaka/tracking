#include "opencv2/opencv.hpp"
#include <iostream>

using namespace std;
using namespace cv;

int main(){

    VideoCapture cap1(1); 
    if(!cap1.isOpened()){
        cout << "Error opening video stream 1" << endl;
        return -1;
    }

    VideoCapture cap2(2); 
    if(!cap2.isOpened()){
        cout << "Error opening video stream 2" << endl;
        return -1;
    }    

    int frame_width_1=   cap1.get(CV_CAP_PROP_FRAME_WIDTH);
    int frame_height_1=   cap1.get(CV_CAP_PROP_FRAME_HEIGHT);
    VideoWriter video1("video1.avi",CV_FOURCC('M','J','P','G'),25, Size(frame_width_1,frame_height_1),true);

    int frame_width_2=   cap2.get(CV_CAP_PROP_FRAME_WIDTH);
    int frame_height_2=   cap2.get(CV_CAP_PROP_FRAME_HEIGHT);
    VideoWriter video2("video2.avi",CV_FOURCC('M','J','P','G'),25, Size(frame_width_2,frame_height_2),true);

   for(;;){

       Mat frame1, frame2;
       cap1 >> frame1;
       cap2 >> frame2;
       video1.write(frame1);
       video2.write(frame2);
       imshow( "Frame 1", frame1 );
       imshow( "Frame 2", frame2 );
       char c = (char)waitKey(1);
       if( c == 27 ) break;
    }
  return 0;
}