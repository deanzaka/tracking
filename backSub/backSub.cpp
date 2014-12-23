#include <cv.h>
#include <highgui.h>
#include <iostream>
#include <sstream>

using namespace std;
using namespace cv;

Mat frame; //current frame
Mat fgMaskMOG; //fg mask generated by MOG method
Mat fgMaskMOG2; //fg mask fg mask generated by MOG2 method
Ptr<BackgroundSubtractor> pMOG; //MOG Background subtractor
Ptr<BackgroundSubtractor> pMOG2; //MOG2 Background subtractor
int keyboard;

int main(int argc, char** argv)
{
  VideoCapture cap(1); // Capture video form camera
  if(!cap.isOpened()) // if not success, exit program
  {
    cout << "cannot open camera" << endl;
  }

  //create window
  namedWindow("Frame");
  namedWindow("FG Mask MOG");
  namedWindow("FG Mask MOG 2");
  
  //create Background Subtractor objects
  pMOG = createBackgroundSubtractorMOG(); //MOG approach
  pMOG2 = createBackgroundSubtractorMOG2(); //MOG2 approach
  
  while(1)
  {
    cap.read(frame);

    //update the background model
    pMOG->apply(frame, fgMaskMOG);
    pMOG2->apply(frame, fgMaskMOG2);
    
    //get the frame number and write it on the current frame
    stringstream ss;
    rectangle(frame, cv::Point(10, 2), cv::Point(100,20),
              cv::Scalar(255,255,255), -1);
    ss << cap.get(CAP_PROP_POS_FRAMES);
    string frameNumberString = ss.str();
    putText(frame, frameNumberString.c_str(), cv::Point(15, 15),
            FONT_HERSHEY_SIMPLEX, 0.5 , cv::Scalar(0,0,0));
    
    //show the current frame and the fg masks
    imshow("Frame", frame);
    imshow("FG Mask MOG", fgMaskMOG);
    imshow("FG Mask MOG 2", fgMaskMOG2);
  
    if (waitKey(1) == 27) //wait for 'esc' key press for 1ms. If 'esc' key is pressed, break loop
    {
      cout << "esc key is pressed" << endl;
        break;
    }
  }
  return 0;
}