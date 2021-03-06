
#include <stdio.h>
#include <iostream>
#include <sstream>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/background_segm.hpp>

using namespace std;
using namespace cv;

Mat frame; //current frame
Mat fgMaskMOG; //fg mask generated by MOG method
Mat merged;
Ptr<BackgroundSubtractor> pMOG; //MOG Background subtractor
Mat channel[3];

int main(int argc, char** argv)
{

   VideoCapture inputVideo("../doubleRecord/video1.avi");              // Open input
    if (!inputVideo.isOpened())
    {
        cout  << "Could not open the input video 1" << endl;
        return -1;
    }

  //create window
  namedWindow("Frame");
  namedWindow("FG Mask MOG");
  // namedWindow("Test");
  
  //create Background Subtractor objects
  pMOG = new BackgroundSubtractorMOG2(); //MOG approach
  
  while(1)
  {
    inputVideo.read(frame);

    //update the background model
    pMOG->operator()(frame, fgMaskMOG);
    // pMOG->operator()(fgMaskMOG2, fgMaskMOG);
    fgMaskMOG.copyTo(channel[0]);
    // fgMaskMOG.copyTo(channel[1]);
    // fgMaskMOG.copyTo(channel[2]);

    channel[1] = Mat::zeros(480, 640, CV_8UC1 );
    channel[2] = Mat::zeros(480, 640, CV_8UC1 );

    merge(channel, 3, merged);

    //morphological opening (removes small objects from the foreground)
    erode(fgMaskMOG, fgMaskMOG, getStructuringElement(MORPH_ELLIPSE, Size(7, 7)) );
    dilate(fgMaskMOG, fgMaskMOG, getStructuringElement(MORPH_ELLIPSE, Size(7, 7)) );

    //morphological closing (removes small holes from the foreground)
    dilate(fgMaskMOG, fgMaskMOG, getStructuringElement(MORPH_ELLIPSE, Size(7, 7)) );
    erode(fgMaskMOG, fgMaskMOG, getStructuringElement(MORPH_ELLIPSE, Size(7, 7)) );
    
    // show the current frame and the fg masks
    imshow("Frame", frame);
    //imshow("Test", merged);
    imshow("FG Mask MOG", fgMaskMOG);
  
    if (waitKey(1) == 27) //wait for 'esc' key press for 1ms. If 'esc' key is pressed, break loop
    {
      cout << "esc key is pressed" << endl;
        break;
    }
  }
  return 0;
}
