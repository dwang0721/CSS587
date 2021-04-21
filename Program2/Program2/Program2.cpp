/**
    CSS587 Computer Vision
    Program2.cpp
    @author Di Wang
    @version 1.1 4/16/21	
*/

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace cv;
using namespace std;

/* Overlay parameters */
struct OverlayParams {
    Mat foreground;
    Mat background;
	Mat outImage;
    int cRed = 0;
    int cGreen = 0;
	int cBlue = 0;
	int distance = 0;
};

/*function declarations*/
void customMaxIdx(Mat hist, int histSize, int* maxIdx);
void overlayImage(Mat foreground, Mat background, Mat output, int cRed, int cGreen, int cBlue, int distance);
void voteForegroundPixel(Mat foreground, Mat hist, int bucketSize);
void onColorDistanceChange(int pos, void* data);

/**
* Find the most common color (Color Key) in the foreground image, 
* replace the pixels that are close the the Color Key with the background image pixels
*/
int main(int argc, char* argv[])
{	
	// input image and assertion
	Mat fImage = imread("foreground.jpg");
	Mat bImage = imread("background2.jpg");
	Mat outImage = fImage.clone();
	assert(fImage.channels()==3);
	assert(bImage.channels()==3);

	// constants and variables
	const int buckets = 16;
	const int bucketSize = 256/buckets;
	double maxVal = 0;
	int maxIdx[3] = {0,0,0};
	int dims[] = {buckets,buckets,buckets};
	Mat hist(3, dims, CV_32S, Scalar::all(0));

    // show Input Images
    imshow("Foreground Image", fImage);
    imshow("Background Image", bImage);
    waitKey(0);

	// vote pixel
	voteForegroundPixel(fImage, hist, bucketSize);

	// find color key
	customMaxIdx(hist, bucketSize, maxIdx);

    // calculate the average color key
    int cRed = maxIdx[0] * bucketSize + bucketSize / 2;
    int cGreen = maxIdx[1] * bucketSize + bucketSize / 2;
    int cBlue = maxIdx[2] * bucketSize + bucketSize / 2;

	// display color key
	Mat colorKey = Mat(300, 300, CV_8UC3, Scalar(cBlue, cGreen, cRed));
	String displayText = format( "Color Key: (%d, %d, %d)",  cRed, cGreen, cBlue );
	putText(colorKey, displayText, Point(50, 150), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255.0, 255.0, 255.0), 1);
    imshow("Color Key", colorKey);
    waitKey(0);

	// replace pixel colors
	namedWindow("overlay");
	OverlayParams overlayParam = {fImage, bImage, outImage, cRed, cGreen, cBlue, 30}; 
	createTrackbar("Distance: ", "overlay", &overlayParam.distance, 255, onColorDistanceChange, &overlayParam);
	onColorDistanceChange(overlayParam.distance, &overlayParam);
    waitKey(0);

    // write the image to disk
    imwrite("overlay.jpg", overlayParam.outImage);
    return 0;
}

/**
*	Implement a function to find the location of the max count.
*   similar to the opencv function: minMaxIdx(hist, 0, &maxVal, 0, maxIdx);
*	@param hist, histogram.
*	@param histSize, histogram size.
*   @param maxIdx, returns the location of the max count.
*/
void customMaxIdx(Mat hist, int histSize, int* maxIdx){
	int maxVal = 0;
    for (int i = 0; i < histSize; i++) {
        for (int j = 0; j < histSize; j++) {
            for (int k = 0; k < histSize; k++) {
                int count = hist.at<int>(i, j, k);
                if (count > maxVal) {
                    maxVal = count;
                    maxIdx[0] = i;
                    maxIdx[1] = j;
                    maxIdx[2] = k;
                }
            }
        }
    }
}

/**
*	Count the image and store the counts in the histogram.
*   @param foreground, foreground image.
*	@param hist, histogram.
*	@param bucketSize, bucket size.
*/
void voteForegroundPixel(Mat foreground, Mat hist, int bucketSize){
    for (int r = 0; r < foreground.rows; r++) {
        for (int c = 0; c < foreground.cols; c++) {
            uchar blue = saturate_cast<uchar>(foreground.at<Vec3b>(r, c)[0]);
            uchar green = saturate_cast<uchar>(foreground.at<Vec3b>(r, c)[1]);
            uchar red = saturate_cast<uchar>(foreground.at<Vec3b>(r, c)[2]);
            int x = red / bucketSize;
            int y = green / bucketSize;
            int z = blue / bucketSize;
            hist.at<int>(x, y, z) += 1;
        }
    }
}

/**
*	Overlay and Merge foreground and background images using the color key.
*   @param foreground, foreground image.
*	@param background, background image.
*	@param output, the output of merging foreground and background image.
*	@param cRed, red channel of the color key.
*	@param cGreen, blue channel of the color key.
*	@param distance, how close to the color key.
*/	
void overlayImage(Mat foreground, Mat background, Mat output, int cRed, int cGreen, int cBlue, int distance){
    for (int r = 0; r < foreground.rows; r++) {
        for (int c = 0; c < foreground.cols; c++) {
            uchar blue = saturate_cast<uchar>(foreground.at<Vec3b>(r, c)[0]);
            uchar green = saturate_cast<uchar>(foreground.at<Vec3b>(r, c)[1]);
            uchar red = saturate_cast<uchar>(foreground.at<Vec3b>(r, c)[2]);

            int diff = abs(cRed - red) + abs(cGreen - green) + abs(cBlue - blue);
            if (diff < distance) {
                Vec3b backgrounColor = background.at<Vec3b>(r%background.rows, c%background.cols);
                output.at<Vec3b>(r, c) = backgrounColor;
			}else {
				Vec3b foregrounColor = foreground.at<Vec3b>(r, c);
				output.at<Vec3b>(r, c) = foregrounColor;
			}
        }
    }	
}

/**
	Callback function that change the overlay result.
    @param pos, the current position of the slider.
	@param data, the user data for the callback.
*/	
void onColorDistanceChange(int pos, void* data){
	OverlayParams* p = (OverlayParams*) data;
    overlayImage(p->foreground, p->background, p->outImage, p->cRed, p->cGreen, p->cBlue, p->distance);
	imshow("overlay", p->outImage);
}