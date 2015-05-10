#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>
#include <ctype.h>

using namespace cv;
using namespace std;

Mat testImage;
bool selectObject = false;
int trackObject = 0;
Point origin;
Rect selection;
int vmin = 10, vmax = 256, smin = 30;

static void onMouse( int event, int x, int y, int, void* )
{
    	if( selectObject )
    	{
        	selection.x = MIN(x, origin.x);
        	selection.y = MIN(y, origin.y);
        	selection.width = std::abs(x - origin.x);
        	selection.height = std::abs(y - origin.y);

        	selection &= Rect(0, 0, testImage.cols, testImage.rows);
    	}

    	switch( event )
    	{
    		case CV_EVENT_LBUTTONDOWN:
        		origin = Point(x,y);
        		selection = Rect(x,y,0,0);
        		selectObject = true;
        		break;
    		case CV_EVENT_LBUTTONUP:
        		selectObject = false;
        		if( selection.width > 0 && selection.height > 0 )
            			trackObject = -1;
        		break;
    	}
}

int main( int argc, const char** argv )
{
	Mat hue, hsv, hist, histimg, mask = Mat::zeros(200, 320, CV_8UC3), backproj;
	Rect trackWindow;
	int hsize = 16;
	float hranges[] = {0,180};
	const float* phranges = hranges;

	namedWindow("test image", 0);
	namedWindow("histogram image", 0);
	setMouseCallback( "test image", onMouse, 0 );

	while(1) {
		testImage  = imread("digimon.jpg");
		cvtColor(testImage, hsv, CV_BGR2HSV);

      		if (trackObject) {
      			int _vmin = vmin, _vmax = vmax;
                	inRange(hsv, Scalar(0, smin, MIN(_vmin,_vmax)),
                        		Scalar(180, 256, MAX(_vmin, _vmax)), mask);
			int ch[]={0,0};
			hue.create(hsv.size(), hsv.depth());
			mixChannels(&hsv, 1, &hue, 1, ch, 1);									// copying only hue values

			if(trackObject < 0) {

				Mat roi(hue, selection), maskroi(mask, selection);					// select region of interest
		      		calcHist(&roi, 1, 0, maskroi, hist, 1, &hsize, &phranges);			// calculate histogram
				normalize(hist, hist, 0,255, CV_MINMAX);							// normalize value to min 0 to max 255
				
				trackWindow = selection;
	      			trackObject = 1;

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
			}
		    	
		  	if( selectObject && selection.width > 0 && selection.height > 0 )
		       {
		            	Mat roi(testImage, selection);
		            	bitwise_not(roi, roi);
		       }

		       cout << "\n selection width: " << selection.width << "\t";    
		       cout << "selection height: " << selection.height << "\t";
		       cout << "select object: " << selectObject << "\t";
		}


		cout << "track object: " << trackObject << "\n";

		if(!histimg.empty()) {
	      		imshow("histogram image", histimg);
	      	}
		imshow("test image", testImage);

		if (waitKey(1) == 27) //wait for 'esc' key press for 1ms. If 'esc' key is pressed, break loop
	      {
		       cout << "\nESC: Closing Application\n";
		       return 0;
	      }
	}
}