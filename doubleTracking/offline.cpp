#include <vector>
#include <cv.h>
#include <cvaux.h>
#include <highgui.h>
#include <iostream>
#include <fstream>
#include <list>

#include <opencv2/video/background_segm.hpp>
#include <opencv2/legacy/blobtrack.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc_c.h>

#include <opencv2/core/core.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2/video/background_segm.hpp>

#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <fstream>

#define PI 3.14159265

using namespace cv;
using namespace std;

int main( int argc, char** argv )
{
     VideoCapture inputVideo1("../doubleRecord/video1.avi");              // Open input
    if (!inputVideo1.isOpened())
    {
        cout  << "Could not open the input video 1" << endl;
        return -1;
    }

    VideoCapture inputVideo2("../doubleRecord/video2.avi");              // Open input
    if (!inputVideo2.isOpened())
    {
        cout  << "Could not open the input video 1" << endl;
        return -1;
    }

    int ex1 = static_cast<int>(inputVideo1.get(CV_CAP_PROP_FOURCC));     // Get Codec Type- Int form
    int ex2 = static_cast<int>(inputVideo2.get(CV_CAP_PROP_FOURCC));     // Get Codec Type- Int form

    // Transform from int to char via Bitwise operators
    char EXT1[] = {(char)(ex1 & 0XFF) , (char)((ex1 & 0XFF00) >> 8),(char)((ex1 & 0XFF0000) >> 16),(char)((ex1 & 0XFF000000) >> 24), 0};
    char EXT2[] = {(char)(ex2 & 0XFF) , (char)((ex2 & 0XFF00) >> 8),(char)((ex2 & 0XFF0000) >> 16),(char)((ex2 & 0XFF000000) >> 24), 0};

    Size S1 = Size((int) inputVideo1.get(CV_CAP_PROP_FRAME_WIDTH),    // Acquire input size
                  (int) inputVideo1.get(CV_CAP_PROP_FRAME_HEIGHT));
    Size S2 = Size((int) inputVideo2.get(CV_CAP_PROP_FRAME_WIDTH),    // Acquire input size
                  (int) inputVideo2.get(CV_CAP_PROP_FRAME_HEIGHT));

    //=============== OBJECT CONTROL ==============================================//
    namedWindow("Object", CV_WINDOW_AUTOSIZE); //create a window called "Object"

    /*
    // Camera 1
    int iLowH = 0;
    int iHighH = 50;

    int iLowS = 90;
    int iHighS = 255;

    int iLowV = 130;
    int iHighV = 255;
	*/

    //Camera 2
    int iLowH = 0;
    int iHighH = 10;

    int iLowS = 150;
    int iHighS = 255;

    int iLowV = 110;
    int iHighV = 255;
	

    //Create trackbars in "Object" window
    createTrackbar("LowH", "Object", &iLowH, 179); //Hue (0 - 179)
    createTrackbar("HighH", "Object", &iHighH, 179);

    createTrackbar("LowS", "Object", &iLowS, 255); //Saturation (0 - 255)
    createTrackbar("HighS", "Object", &iHighS, 255);

    createTrackbar("LowV", "Object", &iLowV, 255);//Value (0 - 255)
    createTrackbar("HighV", "Object", &iHighV, 255);
    //=============== object control ==============================================//

    //Capture a temporary image from the camera
    Mat imgTmp1;
    Mat imgTmp2;
    inputVideo1.read(imgTmp1);
    inputVideo2.read(imgTmp2);

    Mat backSub1;
    Mat backSub2;
    Mat fgMaskMOG1;
    Mat fgMaskMOG2;
    Mat channel1[3];
    Mat channel2[3];


    Ptr<BackgroundSubtractor> pMOG1; //MOG Background subtractor
    Ptr<BackgroundSubtractor> pMOG2; //MOG Background subtractor
    pMOG1 = new BackgroundSubtractorMOG(); //MOG approach
    pMOG2 = new BackgroundSubtractorMOG(); //MOG approach
  
    
    // make text file to save data
    ofstream myfile ("data.txt");

    while (true)
    {
        Mat imgOriginal1;
        bool bSuccess1 = inputVideo1.read(imgOriginal1); // read a new frame from video
        Mat imgOriginalCopy1; // make copy of imgOriginal
        inputVideo1.read(imgOriginalCopy1);
        

        Mat imgOriginal2;
        bool bSuccess2 = inputVideo2.read(imgOriginal2); // read a new frame from video
        Mat imgOriginalCopy2; // make copy of imgOriginal
        inputVideo2.read(imgOriginalCopy2);

        if (!bSuccess1) //if not success, break loop
        {
            cout << "Cannot read a file from video 1" << endl;
            break;
        }

        if (!bSuccess2) //if not success, break loop
        {
            cout << "Cannot read a file from video 2" << endl;
            break;
        }

        //==================== BACKGROUND SUBSTRACTION =========================================================================//

        split(imgOriginal1, channel1); //split to RGB channel
        pMOG1->operator()(imgOriginal1, fgMaskMOG1); //update the background model

        //masking each channel with MOG
        bitwise_and(channel1[0], fgMaskMOG1, channel1[0]);
        bitwise_and(channel1[1], fgMaskMOG1, channel1[1]);
        bitwise_and(channel1[2], fgMaskMOG1, channel1[2]);

        merge(channel1, 3, backSub1); //merging masked channel

        split(imgOriginal2, channel2); //split to RGB channel
        pMOG2->operator()(imgOriginal2, fgMaskMOG2); //update the background model

        //masking each channel with MOG
        bitwise_and(channel2[0], fgMaskMOG2, channel2[0]);
        bitwise_and(channel2[1], fgMaskMOG2, channel2[1]);
        bitwise_and(channel2[2], fgMaskMOG2, channel2[2]);

        merge(channel2, 3, backSub2); //merging masked channel        

        imshow("Background Substraction 1", backSub1);
        imshow("Background Substraction 2", backSub2);
        //==================== BACKGROUND SUBSTRACTION =========================================================================//

        //==================== OBJECT DETECTION CAM1 ===========================================================================//
        Mat imgHSV1;
        cvtColor(imgOriginal1, imgHSV1, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

        Mat imgThresholded1;
        inRange(imgHSV1, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded1); //Threshold the image

        //morphological opening (removes small objects from the foreground)
        erode(imgThresholded1, imgThresholded1, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
        dilate(imgThresholded1, imgThresholded1, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

        //morphological closing (removes small holes from the foreground)
        dilate(imgThresholded1, imgThresholded1, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
        erode(imgThresholded1, imgThresholded1, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

        //Calculate the moments of the thresholded image
        Moments oMoments1 = moments(imgThresholded1);

        double dM011 = oMoments1.m01;
        double dM101 = oMoments1.m10;
        double dArea1 = oMoments1.m00;
        int posX1, posY1;

        // if the area <= 10000, I consider that the there are no object in the image
        //and it's because of the noise, the area is not zero
        if (dArea1 > 10000)
        {
            //calculate the position of the ball
            posX1 = dM101 / dArea1;
            posY1 = dM011 / dArea1;

            // Draw a circle
            circle( imgOriginalCopy1, Point(posX1,posY1), 16.0, Scalar( 0, 0, 255), 3, 8 );

            cout << "Cam 1 position: \t";
            cout << posX1 << "\t";
            cout << posY1 << "\n";
        }
        imshow("Thresholded Image 1", imgThresholded1); //show the thresholded image
        imshow("Original 1", imgOriginal1); //show the original image
        //==================== object detection ===========================================================================//

        //==================== OBJECT DETECTION CAM2 ===========================================================================//
        Mat imgHSV2;
        cvtColor(imgOriginal2, imgHSV2, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

        Mat imgThresholded2;
        inRange(imgHSV2, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded2); //Threshold the image

        //morphological opening (removes small objects from the foreground)
        erode(imgThresholded2, imgThresholded2, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
        dilate(imgThresholded2, imgThresholded2, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

        //morphological closing (removes small holes from the foreground)
        dilate(imgThresholded2, imgThresholded2, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
        erode(imgThresholded2, imgThresholded2, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

        //Calculate the moments of the thresholded image
        Moments oMoments2 = moments(imgThresholded2);

        double dM012 = oMoments2.m01;
        double dM102 = oMoments2.m10;
        double dArea2 = oMoments2.m00;
        int posX2, posY2;

        // if the area <= 10000, I consider that the there are no object in the image
        //and it's because of the noise, the area is not zero
        if (dArea2 > 10000)
        {
            //calculate the position of the ball
            posX2 = dM102 / dArea2;
            posY2 = dM012 / dArea2;

            // Draw a circle
            circle( imgOriginalCopy2, Point(posX2,posY2), 16.0, Scalar( 0, 0, 255), 3, 8 );
    
            cout << "Cam 2 position: \t";
            cout << posX2 << "\t";
            cout << posY2 << "\n";
        }
        imshow("Thresholded Image 2", imgThresholded2); //show the thresholded image
        imshow("Original 2", imgOriginal2); //show the original image
        //==================== object detection ===========================================================================//

        //==================== DISTANCE ESTIMATION ========================================================================//

        double dist = 200;

        // convert pixel position to angle
        double angleX1 = ((posX1*64) / 640) + 13;
        double angleX2 = 90 - (((posX2*64) / 640) + 13);

        // calculate tangensial value for angles
        double tan1 = tan( angleX1 * PI / 180.0 );
        double tan2 = tan( angleX2 * PI / 180.0 );

        // calculate object position
        int posX, posY;
        posX = (tan1 * dist) / (tan1 + tan2);
        posY = (tan2 * posX);

        cout << "Object position: \t";
        cout << posX << "\t";
        cout << posY << "\t";

        //==================== distance estimation ========================================================================//

        //==================== HEIGHT ESTIMATION ========================================================================//
        
        double stand = 103.0;
        double posR, angleZ, tanZ;
        int posZ;

        posR = sqrt(posX*posX + posY*posY);

        if(posY2 > 240) {
        	angleZ = ((posY2*48) / 480) - 24;
        	tanZ = tan(angleZ * PI / 180.0);

        	posZ = posR * tanZ;
        	posZ = stand - posZ;
        }
        else if (posY2 < 240){
        	angleZ = 24 - ((posY2*48)/480);
        	tanZ = tan(angleZ * PI / 180.0);

        	posZ = posR * tanZ;
        	posZ = stand + posZ;
        }
        else posZ = stand;
        cout << posZ << "\n\n";
        //==================== height estimation ========================================================================//

        // write to file
  		if (myfile.is_open())
  		{
    		myfile << "Object position: \t";
	        myfile << posX << "\t";
    	    myfile << posY << "\t";
    	    myfile << posZ << "\n";
	        
  		}
  		else cout << "Unable to open file";

        if (waitKey(20) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
        {
        	myfile.close();
        	cout << "esc key is pressed by user" << endl;
                break;
        }
    }
    return 0;
}
