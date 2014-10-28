#include "opencv2/highgui/highgui.hpp"
#include <iostream>

using namespace std;
using namespace cv;

//=============== VARIABLES ========================================//
int xVal;
int yVal;
int xHover;
int yHover;
bool lClicked = false;
bool rClicked = false;
Point2f tl;
Point2f tr;
Point2f bl;
Point2f br;
//=============== variables ========================================//

void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
    if  ( event == EVENT_LBUTTONDOWN )
    {
        //cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
        xVal = x;
        yVal = y;
    }
    else if  ( event == EVENT_RBUTTONDOWN )
    {
        //cout << "Right button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
    }
        else if  ( event == EVENT_MBUTTONDOWN )
    {
        //cout << "Middle button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
    }
    else if ( event == EVENT_MOUSEMOVE )
    {
        //cout << "Mouse move over the window - position (" << x << ", " << y << ")" << endl;
        xHover = x;
        yHover = y;
    }
}

int main(int argc, char** argv)
{
    VideoCapture cap(1); //capture the video from webcam

    if ( !cap.isOpened() )  // if not success, exit program
    {
        cout << "Cannot open the web cam" << endl;
        return -1;
    }

//============================== SETUP CORNERS ======================================================//

    waitKey();
    cout << "Setup top left...\n Press any key to start, press esc when done \n\n";
    waitKey();

    while(1)
    {
        Mat imgOriginal;
        cap.read(imgOriginal);

        //Create a window
        namedWindow("Camera", 1);

        //set the callback function for any mouse event
        setMouseCallback("Camera", CallBackFunc, NULL);

        // show clicked value
        cout << "\t" << xHover << "\t" << yHover << endl;
        tl.x = xVal;
        tl.y = yVal;

        //show the image
        imshow("Camera", imgOriginal);

        if (waitKey(1) == 27) //wait for 'esc' key press for 1ms. If 'esc' key is pressed, break loop
        {
            cout << "esc key is pressed, top left set" << endl;
            cout << "top left: " << tl << "\n\n";
                break;
        }
    }

    cout << "Setup top right...\n Press any key to start, press esc when done \n\n";
    waitKey();

    while(1)
    {
        Mat imgOriginal;
        cap.read(imgOriginal);

        //Create a window
        namedWindow("Camera", 1);

        //set the callback function for any mouse event
        setMouseCallback("Camera", CallBackFunc, NULL);

        // show clicked value
        cout << "\t" << xHover << "\t" << yHover << endl;
        tr.x = xVal;
        tr.y = yVal;

        //show the image
        imshow("Camera", imgOriginal);

        if (waitKey(1) == 27) //wait for 'esc' key press for 1ms. If 'esc' key is pressed, break loop
        {
            cout << "esc key is pressed, top right set" << endl;
            cout << "top right: " << tr << "\n\n";
                break;
        }
    }

    cout << "Setup bottom left...\n Press any key to start, press esc when done \n\n";
    waitKey();

    while(1)
    {
        Mat imgOriginal;
        cap.read(imgOriginal);

        //Create a window
        namedWindow("Camera", 1);

        //set the callback function for any mouse event
        setMouseCallback("Camera", CallBackFunc, NULL);

        // show clicked value
        cout << "\t" << xHover << "\t" << yHover << endl;
        bl.x = xVal;
        bl.y = yVal;

        //show the image
        imshow("Camera", imgOriginal);

        if (waitKey(1) == 27) //wait for 'esc' key press for 1ms. If 'esc' key is pressed, break loop
        {
            cout << "esc key is pressed, top left set" << endl;
            cout << "bottom left: " << bl << "\n\n";
                break;
        }
    }

    cout << "Setup bottom right...\n Press any key to start, press esc when done \n\n";
    waitKey();

    while(1)
    {
        Mat imgOriginal;
        cap.read(imgOriginal);

        //Create a window
        namedWindow("Camera", 1);

        //set the callback function for any mouse event
        setMouseCallback("Camera", CallBackFunc, NULL);

        // show clicked value
        cout << "\t" << xHover << "\t" << yHover << endl;
        br.x = xVal;
        br.y = yVal;

        //show the image
        imshow("Camera", imgOriginal);

        if (waitKey(1) == 27) //wait for 'esc' key press for 1ms. If 'esc' key is pressed, break loop
        {
            cout << "esc key is pressed, top right set" << endl;
            cout << "bottom right: " << br << "\n\n";
                break;
        }
    }
//============================== setup corners ======================================================//
    waitKey();
    return 0;

}
