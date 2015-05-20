#include <iostream>
#include <vector>
#include <sys/time.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/tracking.hpp>

using namespace cv;
using namespace std;

//------------------------------------------------ convenience method for 
//                                                 using kalman filter with 
//                                                 Point objects
KalmanFilter KF(4, 2, 0);    
Mat_<float> measurement(2,1); 
Mat_<float> state(4, 1); // (x, y, Vx, Vy)

void initKalman(float x, float y)
{
    // Instantate Kalman Filter with
    // 4 dynamic parameters and 2 measurement parameters,
    // where my measurement is: 2D location of object,
    // and dynamic is: 2D location and 2D velocity.
    KF.init(4, 2, 0);

    measurement = Mat_<float>::zeros(2,1);
    measurement.at<float>(0, 0) = x;
    measurement.at<float>(0, 0) = y;


    KF.statePre.setTo(0);
    KF.statePre.at<float>(0, 0) = x;
    KF.statePre.at<float>(1, 0) = y;

    KF.statePost.setTo(0);
    KF.statePost.at<float>(0, 0) = x;
    KF.statePost.at<float>(1, 0) = y; 

    KF.transitionMatrix = *(Mat_<float>(4, 4) << 1,0,1,0,   0,1,0,1,  0,0,1,0,  0,0,0,1);
    setIdentity(KF.measurementMatrix);
    setIdentity(KF.processNoiseCov, Scalar::all(.005)); //adjust this for faster convergence - but higher noise
    setIdentity(KF.measurementNoiseCov, Scalar::all(1e-1));
    setIdentity(KF.errorCovPost, Scalar::all(.1));
}

Point kalmanPredict() 
{
    Mat prediction = KF.predict();
    Point predictPt(prediction.at<float>(0),prediction.at<float>(1));
    return predictPt;
}

Point kalmanCorrect(float x, float y)
{
    measurement(0) = x;
    measurement(1) = y;
    Mat estimated = KF.correct(measurement);
    Point statePt(estimated.at<float>(0),estimated.at<float>(1));
    return statePt;
}

//------------------------------------------------ main

int main (int argc, char * const argv[]) 
{
    Point s, p;

    initKalman(0, 0);

    p = kalmanPredict();
    cout << "kalman prediction: " << p.x << " " << p.y << endl;
    /* 
     * output is: kalman prediction: 0 0
     *
     * note 1:
     *         ok, the initial value, not yet new observations
     */

    s = kalmanCorrect(10, 10);
    cout << "kalman corrected state: " << s.x << " " << s.y << endl;
    /* 
     * output is: kalman corrected state: 5 5
     *
     * note 2:
     *         ok, kalman filter is smoothing the noisy observation and 
     *         slowly "following the point"
     *         .. how faster the kalman filter follow the point is 
     *            processNoiseCov parameter
     */

    p = kalmanPredict();
    cout << "kalman prediction: " << p.x << " " << p.y << endl;
    /* 
     * output is: kalman prediction: 5 5
     *
     * note 3:
     *         mhmmm, same as the last correction, probabilly there are so few data that
     *         the filter is not predicting anything..
     */

    s = kalmanCorrect(20, 20);
    cout << "kalman corrected state: " << s.x << " " << s.y << endl;
    /* 
     * output is: kalman corrected state: 10 10
     *
     * note 3:
     *         ok, same as note 2
     */

    p = kalmanPredict();
    cout << "kalman prediction: " << p.x << " " << p.y << endl;
    s = kalmanCorrect(30, 30);
    cout << "kalman corrected state: " << s.x << " " << s.y << endl;
    /* 
     * output is: kalman prediction: 10 10
     *            kalman corrected state: 16 16
     *
     * note 4:
     *         ok, same as note 2 and 3
     */    


    /*
     * now let's say I don't received observation for few frames,
     * I want anyway to update the kalman filter to predict 
     * the future states of my system
     *
     */
    for(int i=0; i<5; i++) {
        p = kalmanPredict();
        cout << "kalman prediction: " << p.x << " " << p.y << endl;
    }
    /* 
     * output is: kalman prediction: 16 16
     * kalman prediction: 16 16
     * kalman prediction: 16 16
     * kalman prediction: 16 16
     * kalman prediction: 16 16
     *
     * !!! kalman filter is still on 16, 16..
     *     no future prediction here..
     *     I'm exprecting the point to go further..
     *     why???
     *
     */    

    return 0;
}