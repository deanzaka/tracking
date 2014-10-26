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

Point2f center(0,0);

Point2f computeIntersect(Vec4i a, Vec4i b)
{
    int x1 = a[0], y1 = a[1], x2 = a[2], y2 = a[3], x3 = b[0], y3 = b[1], x4 = b[2], y4 = b[3];
    float denom;
    if (float d = ((float)(x1 - x2) * (y3 - y4)) - ((y1 - y2) * (x3 - x4)))
    {
        Point2f pt;
        pt.x = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / d;
        pt.y = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / d;
        return pt;
    }
    else
    return Point2f(-1, -1);
}

void sortCorners(vector<Point2f>& corners, Point2f center)
{
    vector<Point2f> top, bot;
    for (int i = 0; i < corners.size(); i++)
    {
        if (corners[i].y < center.y)
            top.push_back(corners[i]);
        else
        bot.push_back(corners[i]);
    }
    Point2f tl = top[0].x > top[1].x ? top[1] : top[0];
    Point2f tr = top[0].x > top[1].x ? top[0] : top[1];
    Point2f bl = bot[0].x > bot[1].x ? bot[1] : bot[0];
    Point2f br = bot[0].x > bot[1].x ? bot[0] : bot[1];
    corners.clear();
    corners.push_back(tl);
    corners.push_back(tr);
    corners.push_back(br);
    corners.push_back(bl);
}

int main( int argc, char** argv )
{
    VideoCapture cap(1); //capture the video from webcam

    if ( !cap.isOpened() )  // if not success, exit program
    {
        cout << "Cannot open the web cam" << endl;
        return -1;
    }

    //=============== OBJECT CONTROL ==============================================//
    namedWindow("Object", CV_WINDOW_AUTOSIZE); //create a window called "Object"

    int iLowH = 0;
    int iHighH = 10;

    int iLowS = 135;
    int iHighS = 255;

    int iLowV = 50;
    int iHighV = 255;

    //Create trackbars in "Object" window
    createTrackbar("LowH", "Object", &iLowH, 179); //Hue (0 - 179)
    createTrackbar("HighH", "Object", &iHighH, 179);

    createTrackbar("LowS", "Object", &iLowS, 255); //Saturation (0 - 255)
    createTrackbar("HighS", "Object", &iHighS, 255);

    createTrackbar("LowV", "Object", &iLowV, 255);//Value (0 - 255)
    createTrackbar("HighV", "Object", &iHighV, 255);
    //=============== object control ==============================================//


    //=============== FIELD CONTROL ==============================================//
    namedWindow("Field", CV_WINDOW_AUTOSIZE); //create a window called "Field"

    int fLowH = 0;
    int fHighH = 100;

    int fLowS = 180;
    int fHighS = 255;

    int fLowV = 120;
    int fHighV = 255;

    //Create trackbars in "Field" window
    createTrackbar("LowH", "Field", &fLowH, 179); //Hue (0 - 179)
    createTrackbar("HighH", "Field", &fHighH, 179);

    createTrackbar("LowS", "Field", &fLowS, 255); //Saturation (0 - 255)
    createTrackbar("HighS", "Field", &fHighS, 255);

    createTrackbar("LowV", "Field", &fLowV, 255);//Value (0 - 255)
    createTrackbar("HighV", "Field", &fHighV, 255);
    //=============== field control ==============================================//

    //Capture a temporary image from the camera
    Mat imgTmp;
    cap.read(imgTmp);

    //Create a black image with the size as the camera output
    Mat imgLines = Mat::zeros( imgTmp.size(), CV_8UC3 );;

    int c = 0;
    while (true)
    {
        Mat imgOriginal;
        bool bSuccess = cap.read(imgOriginal); // read a new frame from video

        if (!bSuccess) //if not success, break loop
        {
            cout << "Cannot read a frame from video stream" << endl;
            break;
        }

        Mat imgHSV;
        cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

        //==================== OBJECT DETECTION ===========================================================================//
        Mat imgThresholded;
        inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image

        //morphological opening (removes small objects from the foreground)
        erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
        dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

        //morphological closing (removes small holes from the foreground)
        dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
        erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

        //Calculate the moments of the thresholded image
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
            circle( imgOriginal, Point(posX,posY), 16.0, Scalar( 0, 0, 255), 3, 8 );

            cout << posX << "\t";
            cout << posY << "\n\n";
        }
        imshow("Thresholded Image", imgThresholded); //show the thresholded image
        //==================== object detection ===========================================================================//

        //==================== FIELD DETECTION ===========================================================================//
        Mat imgField;
        inRange(imgHSV, Scalar(fLowH, fLowS, fLowV), Scalar(fHighH, fHighS, fHighV), imgField); //Threshold the image

        //morphological opening (removes small objects from the foreground)
        erode(imgField, imgField, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
        dilate( imgField, imgField, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

        //morphological closing (removes small holes from the foreground)
        dilate( imgField, imgField, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
        erode(imgField, imgField, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

        imshow("Field Image", imgField); //show the thresholded image
        //==================== field detection ===========================================================================//


        //==================== GENERATE LINES ============================================================================//
        Mat imgGray;
        blur(imgField,imgGray,Size(3, 3));
        Canny(imgGray,imgGray,100,100,3); //Get edge map
        vector<Vec4i> lines;
        HoughLinesP(imgGray, lines, 1, CV_PI/180, 30, 30, 10);
        c = lines.size();
        cout << "Lines: " << c << "\n";

        if(c >= 4)
        {
            // Expand the lines
            for (int i = 0; i < lines.size(); i++)
            {
                Vec4i v = lines[i];
                lines[i][0] = 0;
                lines[i][1] = ((float)v[1] - v[3]) / (v[0] - v[2]) * -v[0] + v[1];
                lines[i][2] = imgOriginal.cols;
                lines[i][3] = ((float)v[1] - v[3]) / (v[0] - v[2]) * (imgOriginal.cols - v[2]) + v[3];
            }

            // Draw lines
            for (int i = 0; i < lines.size(); i++)
            {
                Vec4i v = lines[i];
                line(imgOriginal, Point(v[0], v[1]), Point(v[2], v[3]), CV_RGB(0,255,0));
            }

            vector<Point2f> corners;
            for (int i = 0; i < lines.size(); i++)
            {
                for (int j = i+1; j < lines.size(); j++)
                {
                    Point2f pt = computeIntersect(lines[i], lines[j]);
                    if (pt.x >= 0 && pt.y >= 0) corners.push_back(pt);
                }
            }

            vector<Point2f> approx;
            approxPolyDP(Mat(corners), approx, arcLength(Mat(corners), true) * 0.02, true);
            if (approx.size() == 4)
            {
                cout << "\nLines: " << c << "\n";
                cout << "Corners: " << corners.size() << "\n";
                cout << "The object is quadrilateral! \n\n";

                // Get mass center
                for (int i = 0; i < corners.size(); i++)
                center += corners[i];
                center *= (1. / corners.size());
                //sortCorners(corners, center);

                // Draw corner points
                for(int i = 0; i < corners.size(); i++)
                {
                    string out;          // string which will contain the result
                    ostringstream convert;   // stream used for the conversion
                    convert << i;      // insert the textual representation of 'Number' in the characters in the stream
                    out = convert.str(); // set 'out' to the contents of the stream
                    circle(imgOriginal, corners[i], 3, Scalar(255,0,0), 3, 8);
                    putText(imgOriginal,out,corners[i],FONT_HERSHEY_SIMPLEX,1,Scalar(255,0,0));
                }

                // Draw mass center
                //circle(imgOriginal, center, 3, CV_RGB(255,255,0), 2);

                imshow("Original", imgOriginal); //show the original image;
                usleep(3000000);
            }
        }
        //==================== generate lines ============================================================================//

        //imshow("Edge Map", imgGray); //show the edge map
        //imgOriginal = imgOriginal + imgLines;
        //imshow("Original", imgOriginal); //show the original image


            if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
            {
            cout << "esc key is pressed by user" << endl;
                    break;
            }
    }
    return 0;
}
