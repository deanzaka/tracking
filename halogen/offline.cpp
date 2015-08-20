#include <vector>
#include <cv.h>
#include <cvaux.h>
#include <highgui.h>
#include <iostream>
#include <list>

#include <opencv2/video/background_segm.hpp>
#include <opencv2/legacy/blobtrack.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc_c.h>

#include <opencv2/core/core.hpp>
#include <opencv2/video/tracking.hpp>

#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>

using namespace cv;
using namespace std;


int main( int argc, char** argv )
{
    bool paused = false;
    Mat frame, imgOriginal, imgHSV;

    namedWindow("Object", CV_WINDOW_AUTOSIZE);

    int iLowH = 125;
    int iHighH = 170;

    int iLowS = 2;
    int iHighS = 20;

    int iLowV = 230;
    int iHighV = 255;
    
    int noise = 0;
    int holes = 10;

    //Create trackbars in "Object" window
    createTrackbar("LowH", "Object", &iLowH, 179); //Hue (0 - 179)
    createTrackbar("HighH", "Object", &iHighH, 179);

    createTrackbar("LowS", "Object", &iLowS, 255); //Saturation (0 - 255)
    createTrackbar("HighS", "Object", &iHighS, 255);

    createTrackbar("LowV", "Object", &iLowV, 255);//Value (0 - 255)
    createTrackbar("HighV", "Object", &iHighV, 255);

    createTrackbar("removes small noise", "Object", &noise, 10);
    createTrackbar("removes small holes", "Object", &holes, 10);

    VideoCapture inputVideo("../../../Videos/AUAVUI2015/cut/GRD-030815-0825-CUT.avi"); //capture the video from webcam
    
    if ( !inputVideo.isOpened() )  // if not success, exit program
    {
        cout << "Cannot open file" << endl;
        return -1;
    }
    
    while (true)
    {
        if (!paused) {
            // bSuccess = inputVideo.read(imgOriginal); // read a new frame from video
            inputVideo >> frame;
            if( frame.empty() ) {
                cout << "No frame" << endl;   
                break;
            }
        }
        
        frame.copyTo(imgOriginal);
        
        // Mat imgHSV;
        // cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

        // Mat imgThresholded;
        // inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image
        
        Mat imgGRAY;
        cvtColor(imgOriginal, imgGRAY, COLOR_BGR2GRAY);

        Mat imgThresholded;
        inRange(imgGRAY, iLowV, iHighV, imgThresholded);

        if(noise > 0) {
            //morphological opening (removes small objects from the foreground)
            erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(noise, noise)) );
            dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(noise, noise)) );
        }

        if(holes > 0) {
            //morphological closing (removes small holes from the foreground)
            dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(holes, holes)) );
            erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(holes, holes)) );
        }

        Moments oMoments = moments(imgThresholded);

        double dM01 = oMoments.m01;
        double dM10 = oMoments.m10;
        double dArea = oMoments.m00;
        int posX, posY;

        cout << "Area: " << dArea << endl;

        // if the area <= 10000, I consider that the there are no object in the image
        //and it's because of the noise, the area is not zero
        if (dArea > 10000 && dArea < 500000)
        {
            //calculate the position of the ball
            posX = dM10 / dArea;
            posY = dM01 / dArea;

            // Draw a circle
            circle( imgGRAY, Point(posX,posY), 16.0, Scalar( 0, 0, 255), 3, 8 );

            cout << "Object position: \t";
            cout << posX << "\t";
            cout << posY << "\n";
        }


        imshow("Gray", imgGRAY); //show GRAY image
        imshow("Original", imgOriginal); //show the original image
        imshow("Thresholded", imgThresholded); //show Thresholded image
        
        char c = (char)waitKey(30); //wait for key press for 30ms

        if (c == 27) // ascii of esc
        {
            cout << "esc key is pressed by user" << endl;
            break;
        }
        else if (c == 'p') paused = !paused;
    }
    return 0;
}
