#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>
#include <ctype.h>

using namespace cv;
using namespace std;

int main( int argc, const char** argv )
{
	Mat testImage, hue, hsv, hist, histimg = Mat::zeros(200, 320, CV_8UC3), backproj;
	int hsize = 16;
	float hranges[] = {0,180};
	const float* phranges = hranges;

	namedWindow("test image", 0);
	namedWindow("histogram image", 0);
	testImage  = imread("digimon.jpg");
	imshow("test image", testImage);

	cvtColor(testImage, hsv, CV_BGR2HSV);
	int ch[]={0,0};
	hue.create(hsv.size(), hsv.depth());
	mixChannels(&hsv, 1, &hue, 1, ch, 1);								// copying only hue values
	
	calcHist(&hue, 1, 0, Mat(), hist, 1, &hsize, &phranges);				// calculate histogram
	normalize(hist, hist, 0,255, CV_MINMAX);							// normalize value to min 0 to max 255
	calcBackProject( &hue, 1, 0, hist, backproj, &phranges, 1, true );		// calculate back projection of image, by histogram

	histimg = Scalar::all(0);
      	int binW = histimg.cols / hsize;
      	Mat buf(1, hsize, CV_8UC3);
      	for( int i = 0; i < hsize; i++ )
          buf.at<Vec3b>(i) = Vec3b(saturate_cast<uchar>(i*180./hsize), 255, 255);
      	cvtColor(buf, buf, CV_HSV2BGR);

      	for( int i = 0; i < hsize; i++ )
      {
          int val = saturate_cast<int>(hist.at<float>(i)*histimg.rows/255);
          rectangle( histimg, Point(i*binW,histimg.rows),
                     Point((i+1)*binW,histimg.rows - val),
                     Scalar(buf.at<Vec3b>(i)), -1, 8 );
      }

      	if(!histimg.empty()){
       	imshow("histogram image", histimg);
    	}
      

	if (waitKey(0) == 27) //wait for 'esc' key press for 1ms. If 'esc' key is pressed, break loop
      {
	       cout << "\nESC: Closing Application\n";
	       return 0;
      }
}