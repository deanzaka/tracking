#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>
#include <ctype.h>

using namespace cv;
using namespace std;

Mat image1, image2;

bool showHist = true;
bool backprojMode = false;

bool selectObject1 = false;
int trackObject1 = 0;
Point origin1;
Rect selection1;
int vmin1 = 10, vmax1 = 256, smin1 = 30;

bool selectObject2 = false;
int trackObject2 = 0;
Point origin2;
Rect selection2;
int vmin2 = 10, vmax2 = 256, smin2 = 30;

static void onMouse1( int event, int x, int y, int, void* )
{
    if( selectObject1 )
    {
        selection1.x = MIN(x, origin1.x);
        selection1.y = MIN(y, origin1.y);
        selection1.width = std::abs(x - origin1.x);
        selection1.height = std::abs(y - origin1.y);

        selection1 &= Rect(0, 0, image1.cols, image1.rows);
    }

    switch( event )
    {
    case CV_EVENT_LBUTTONDOWN:
        origin1 = Point(x,y);
        selection1 = Rect(x,y,0,0);
        selectObject1 = true;
        break;
    case CV_EVENT_LBUTTONUP:
        selectObject1 = false;
        if( selection1.width > 0 && selection1.height > 0 )
            trackObject1 = -1;
        break;
    }
}

static void onMouse2( int event, int x, int y, int, void* )
{
    if( selectObject2 )
    {
        selection2.x = MIN(x, origin2.x);
        selection2.y = MIN(y, origin2.y);
        selection2.width = std::abs(x - origin2.x);
        selection2.height = std::abs(y - origin2.y);

        selection2 &= Rect(0, 0, image2.cols, image2.rows);
    }

    switch( event )
    {
    case CV_EVENT_LBUTTONDOWN:
        origin2 = Point(x,y);
        selection2 = Rect(x,y,0,0);
        selectObject2 = true;
        break;
    case CV_EVENT_LBUTTONUP:
        selectObject2 = false;
        if( selection2.width > 0 && selection2.height > 0 )
            trackObject2 = -1;
        break;
    }
}

static void help()
{

    cout << "\n\nHot keys: \n"
            "\tESC - quit the program\n"
            "\tc - stop the tracking\n"
            "\tb - switch to/from backprojection view\n"
            "\th - show/hide object histogram\n"
            "\tp - pause video\n"
            "To initialize tracking, select the object with mouse\n";
}

int main( int argc, const char** argv )
{
    help();

    VideoCapture cap1;
    Rect trackWindow1;
    VideoCapture cap2;
    Rect trackWindow2;
    int hsize = 16;
    float hranges[] = {0,180};
    const float* phranges = hranges;

    cap1.open(1);
    cap2.open(2);


    if( !cap1.isOpened() )
    {
        cout << "***Could not initialize capturing camera 2...***\n";
        return -1;
    }

    if( !cap2.isOpened() )
    {
        cout << "***Could not initialize capturing camera 2...***\n";
        return -1;
    }

    namedWindow( "Histogram 1", 0 );
    namedWindow( "Histogram 2", 0 );

    namedWindow( "CamShift 1", 0 );
    setMouseCallback( "CamShift 1", onMouse1, 0 );
    createTrackbar( "Vmin", "CamShift 1", &vmin1, 256, 0 );
    createTrackbar( "Vmax", "CamShift 1", &vmax1, 256, 0 );
    createTrackbar( "Smin", "CamShift 1", &smin1, 256, 0 );


    namedWindow( "CamShift 2", 0 );
    setMouseCallback( "CamShift 2", onMouse2, 0 );
    createTrackbar( "Vmin", "CamShift 2", &vmin2, 256, 0 );
    createTrackbar( "Vmax", "CamShift 2", &vmax2, 256, 0 );
    createTrackbar( "Smin", "CamShift 2", &smin2, 256, 0 );

    Mat frame1, hsv1, hue1, mask1, hist1, histimg1 = Mat::zeros(200, 320, CV_8UC3), backproj1;
    Mat frame2, hsv2, hue2, mask2, hist2, histimg2 = Mat::zeros(200, 320, CV_8UC3), backproj2;
    bool paused = false;

    for(;;)
    {
        if( !paused )
        {
            cap1 >> frame1;
            if( frame1.empty() )
                break;
        }
        if( !paused )
        {
            cap2 >> frame2;
            if( frame2.empty() )
                break;
        }

        frame1.copyTo(image1);
        frame2.copyTo(image2);


        if( !paused )
        {
            cvtColor(image1, hsv1, COLOR_BGR2HSV);
            cvtColor(image2, hsv2, COLOR_BGR2HSV);
            
            if( trackObject1 )
            {
                int _vmin1 = vmin1, _vmax1 = vmax1;

                inRange(hsv1, Scalar(0, smin1, MIN(_vmin1,_vmax1)),
                        Scalar(180, 256, MAX(_vmin1, _vmax1)), mask1);
                int ch1[] = {0, 0};
                hue1.create(hsv1.size(), hsv1.depth());
                mixChannels(&hsv1, 1, &hue1, 1, ch1, 1);

                if( trackObject1 < 0 )
                {
                    Mat roi1(hue1, selection1), maskroi(mask1, selection1);
                    calcHist(&roi1, 1, 0, maskroi, hist1, 1, &hsize, &phranges);
                    normalize(hist1, hist1, 0, 255, CV_MINMAX);

                    trackWindow1 = selection1;
                    trackObject1 = 1;

                    histimg1 = Scalar::all(0);
                    int binW = histimg1.cols / hsize;
                    Mat buf1(1, hsize, CV_8UC3);
                    for( int i = 0; i < hsize; i++ )
                        buf1.at<Vec3b>(i) = Vec3b(saturate_cast<uchar>(i*180./hsize), 255, 255);
                    cvtColor(buf1, buf1, CV_HSV2BGR);

                    for( int i = 0; i < hsize; i++ )
                    {
                        int val2 = saturate_cast<int>(hist1.at<float>(i)*histimg1.rows/255);
                        rectangle( histimg1, Point(i*binW,histimg1.rows),
                                   Point((i+1)*binW,histimg1.rows - val2),
                                   Scalar(buf1.at<Vec3b>(i)), -1, 8 );
                    }
                }

                calcBackProject(&hue1, 1, 0, hist1, backproj1, &phranges);
                backproj1 &= mask1;
                RotatedRect trackBox1 = CamShift(backproj1, trackWindow1,
                                    TermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ));
                if( trackWindow1.area() <= 1 )
                {
                    int cols = backproj1.cols, rows = backproj1.rows, r = (MIN(cols, rows) + 5)/6;
                    trackWindow1 = Rect(trackWindow1.x - r, trackWindow1.y - r,
                                       trackWindow1.x + r, trackWindow1.y + r) &
                                  Rect(0, 0, cols, rows);
                }

                if( backprojMode )
                    cvtColor( backproj1, image1, COLOR_GRAY2BGR );
                ellipse( image1, trackBox1, Scalar(0,0,255), 3, CV_AA );
            }


            if( trackObject2 )
            {
                int _vmin2 = vmin2, _vmax2 = vmax2;

                inRange(hsv2, Scalar(0, smin2, MIN(_vmin2,_vmax2)),
                        Scalar(180, 256, MAX(_vmin2, _vmax2)), mask2);
                int ch2[] = {0, 0};
                hue2.create(hsv2.size(), hsv2.depth());
                mixChannels(&hsv2, 1, &hue2, 1, ch2, 1);

                if( trackObject2 < 0 )
                {
                    Mat roi2(hue2, selection2), maskroi(mask2, selection2);
                    calcHist(&roi2, 1, 0, maskroi, hist2, 1, &hsize, &phranges);
                    normalize(hist2, hist2, 0, 255, CV_MINMAX);

                    trackWindow2 = selection2;
                    trackObject2 = 1;

                    histimg2 = Scalar::all(0);
                    int binW = histimg2.cols / hsize;
                    Mat buf2(1, hsize, CV_8UC3);
                    for( int i = 0; i < hsize; i++ )
                        buf2.at<Vec3b>(i) = Vec3b(saturate_cast<uchar>(i*180./hsize), 255, 255);
                    cvtColor(buf2, buf2, CV_HSV2BGR);

                    for( int i = 0; i < hsize; i++ )
                    {
                        int val2 = saturate_cast<int>(hist2.at<float>(i)*histimg2.rows/255);
                        rectangle( histimg2, Point(i*binW,histimg2.rows),
                                   Point((i+1)*binW,histimg2.rows - val2),
                                   Scalar(buf2.at<Vec3b>(i)), -1, 8 );
                    }
                }

                calcBackProject(&hue2, 1, 0, hist2, backproj2, &phranges);
                backproj2 &= mask2;
                RotatedRect trackBox2 = CamShift(backproj2, trackWindow2,
                                    TermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ));
                if( trackWindow2.area() <= 1 )
                {
                    int cols = backproj2.cols, rows = backproj2.rows, r = (MIN(cols, rows) + 5)/6;
                    trackWindow2 = Rect(trackWindow2.x - r, trackWindow2.y - r,
                                       trackWindow2.x + r, trackWindow2.y + r) &
                                  Rect(0, 0, cols, rows);
                }

                if( backprojMode )
                    cvtColor( backproj2, image2, COLOR_GRAY2BGR );
                ellipse( image2, trackBox2, Scalar(0,0,255), 3, CV_AA );
            }


        }
        else if( trackObject1 < 0 && trackObject2 < 0 )
            paused = false;

        if( selectObject1 && selection1.width > 0 && selection1.height > 0 )
        {
            Mat roi1(image1, selection1);
            bitwise_not(roi1, roi1);
        }

        if( selectObject2 && selection2.width > 0 && selection2.height > 0 )
        {
            Mat roi2(image2, selection2);
            bitwise_not(roi2, roi2);
        }

        imshow( "CamShift 1", image1 );
        imshow( "CamShift 2", image2 );
        imshow( "Histogram 1", histimg1 );
        imshow( "Histogram 2", histimg2 );


        char c = (char)waitKey(10);
        if( c == 27 )
            break;
        switch(c)
        {
        case 'b':
            backprojMode = !backprojMode;
            break;
        case 'c':
            trackObject1 = 0;
            histimg1 = Scalar::all(0);
            break;
        case 'h':
            showHist = !showHist;
            if( !showHist )
                destroyWindow( "Histogram" );
            else
                namedWindow( "Histogram", 1 );
            break;
        case 'p':
            paused = !paused;
            break;
        default:
            ;
        }
    }

    return 0;
}