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

    int low = 225;
    int high = 255;
    
    int noise = 0;
    int holes = 20;

    int offset = 20;

    //Create trackbars in "Object" window
    createTrackbar("Low", "Object", &low, 255);//Value (0 - 255)
    createTrackbar("High", "Object", &high, 255);

    createTrackbar("removes small noise", "Object", &noise, 20);
    createTrackbar("removes small holes", "Object", &holes, 20);

    createTrackbar("offset from max", "Object", &offset, 20);

    VideoCapture inputVideo("../../../Videos/AUAVUI2015/MyVideo.avi"); //capture the video from webcam
    
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
        
        Mat imgGRAY;
        cvtColor(imgOriginal, imgGRAY, COLOR_BGR2GRAY);

        Mat imgThresholded;
        inRange(imgGRAY, low, high, imgThresholded);

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

        int max = 0;
        for( int y = 0; y < imgThresholded.rows; y++ ) {
            for( int x = 0; x < imgThresholded.cols; x++ ) {
                if ( imgThresholded.at<uchar>(y,x) == 255 ) {
                    int gray = imgGRAY.at<uchar>(y,x);
                    if (max < gray) {
                        max = gray;
                        cout << "Max = " << max << endl;
                    }
                }
            }
        }
        
        if(max-offset > low) {
            inRange(imgGRAY, max-offset, high, imgThresholded);
        }
        
        Moments oMoments = moments(imgThresholded);

        double dM01 = oMoments.m01;
        double dM10 = oMoments.m10;
        double dArea = oMoments.m00;
        int posX, posY;

        // if the area <= 10000, I consider that the there are no object in the image
        //and it's because of the noise, the area is not zero
        if (dArea > 10000)
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
