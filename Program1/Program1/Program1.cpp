/**
    CSS587 Computer Vision
    Program1.cpp
    @author Di Wang
    @version 1.1 4/6/21

	OpenCV Investigation:
	1. Erode()
	2. SFIT

	Edge detection findings:
	These settings gives best result from eye balling.
		Max Threshold: 75
		Min Threshold: 15
		Gaussian Sigma: 1.5
	Gaussian Sigma 1.5 gives an good filtering result on image noises, but not getting rid of important edge details. 
	With Max Threshold 75 and Min Threshold 15, I can see the contours of the cat, the phone, and the content layout in the background monitor. 
	Decreasing Sigma introduces many wrong edges, increasing the size the cat can hardly be identified from the edge detection output. 
*/

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/features2d.hpp>

using namespace cv;
using namespace std;

/* Canny parameters */
struct CannyParams {    
    Mat inputImage;
	Mat outImage;
    int threshold1 = 0;
    int threshold2 = 0;
};

/* Gaussian Parameters*/
struct GaussianParams {
    Mat inputImage;
    Mat outImage;
    int sigma = 0;
};

/*function declarations*/
void rotate180(Mat inImage);
void onSigmaChange(int pos, void* data);
void onThresholdChange(int pos, void* data);

/**
* [Investigation] Erode and SFIT.
* [Implementation] flip the image both vertically and horizontally (180 degree rotation), 
*                  convert it to gray scale, blur it, and detect edges.* 
*/					
int main(int argc, char* argv[])
{	
		Mat image = imread("pippy.jpg");
		Mat outImage;
		
		// reading the image
		namedWindow("Original Image");
		imshow("Original Image", image);
		waitKey(0);

		// erode
		Mat erodeOutput;
        Mat kernel = getStructuringElement(MORPH_RECT, Size(5, 5));
		erode(image, erodeOutput, kernel);
        imshow("erode", erodeOutput);
        imwrite("erode_output.jpg", erodeOutput);
		waitKey(0);

		// SIFT 
		Mat siftOutput;
		vector<KeyPoint> keypoints;
		Ptr<SIFT> sift = SIFT::create();		
		sift->detect(image, keypoints);
		drawKeypoints(image, keypoints, siftOutput);
		imshow("SIFT", siftOutput);
		imwrite("SIFT_output.jpg",siftOutput);
		waitKey(0);

		// flip the image
        rotate180(image);
        namedWindow("roate180");
        imshow("roate180", image);
        waitKey(0);

		// convert to gray scale
		cvtColor(image, image, COLOR_BGR2GRAY);
        namedWindow("grayScale");
        imshow("grayScale", image);
        waitKey(0);
		
		// blur
		namedWindow("blur");
		GaussianParams gaussParam = {image, outImage, 2}; 
        createTrackbar("Sigma: ", "blur", &gaussParam.sigma, 10, onSigmaChange, &gaussParam);
        onSigmaChange(gaussParam.sigma, &gaussParam);
        waitKey(0);

		// edge detection		
        namedWindow("edge detection");
		CannyParams cannyParam = {gaussParam.outImage, outImage, 20, 60}; 
		createTrackbar("Max Threshold: ", "edge detection", &cannyParam.threshold2, 80, onThresholdChange, &cannyParam);
		createTrackbar("Min Threshold: ", "edge detection", &cannyParam.threshold1, 80, onThresholdChange, &cannyParam);
		onThresholdChange( cannyParam.threshold2, &cannyParam );       
        waitKey(0);

		// write the image to disk
		imwrite("output.jpg", cannyParam.outImage);
		return 0;
}

/**
	rotate an image by 180 degree.
    @param image, the input and output image.
*/	
void rotate180(Mat inImage){
    flip(inImage, inImage, 0);
    flip(inImage, inImage, 1);
}

/**
	Callback function that shows an edges of an image.
    @param pos, the current position of the slider.
	@param data, the user data for the callback.
*/	
void onSigmaChange(int pos, void* data)
{
	GaussianParams* p = (GaussianParams*)data;
	double sigmaX = p->sigma;
	double sigmaY = p->sigma;
	if (p->sigma <= 0){ // avoid invalid input
        sigmaX = 0.01;
        sigmaY = 0.01;
	}
    GaussianBlur(p->inputImage, p->outImage, Size(0, 0), sigmaX, sigmaY, 4);
    imshow("blur", p->outImage);
}

/**
	Callback function that shows an edges of an image.
    @param pos, the current position of the slider.
	@param data, the user data for the callback.
*/	
void onThresholdChange(int pos, void* data)
{
	CannyParams* p = (CannyParams*)data;
    Canny(p->inputImage, p->outImage, p->threshold1, p->threshold2, 3, true);
    imshow("edge detection", p->outImage);
}
