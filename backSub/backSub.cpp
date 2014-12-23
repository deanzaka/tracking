#include <cv.h>
#include <highgui.h>

using namespace std;
using namespace cv;

Mat imgOriginal;

int main(int argc, char** argv)
{
  VideoCapture cap(2); // Capture video form camera
  
  if(!cap.isOpened()) // if not success, exit program
  {
    cout << "cannot open camera" << endl;
  }
  
  while(1)
  {
    cap.read(imgOriginal);
    imshow("Frame", imgOriginal);
  
    if (waitKey(1) == 27) //wait for 'esc' key press for 1ms. If 'esc' key is pressed, break loop
    {
      cout << "esc key is pressed" << endl;
        break;
    }
  }
  return 0;
}