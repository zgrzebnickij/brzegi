#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

#define PI 3.1415

int main() {

	VideoCapture cap(0); //capture the video from web cam

	if (!cap.isOpened())  // if not success, exit program
	{
		cout << "Cannot open the web cam" << endl;
		return -1;
	}

	/*Mat image;
	Mat imgCanny;
	image = imread("grzadki1.jpg");
	if (!image.data)                              // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		system("Pause");
		return -1;
	}*/
	namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"
	namedWindow("Control1", CV_WINDOW_AUTOSIZE);

	int iLowH = 39;
	int iHighH = 69;

	int iLowS = 68;
	int iHighS = 255;

	int iLowV = 40;
	int iHighV = 255;

	int max_angle = 180;
	int min_angle = 0;


	cvCreateTrackbar("max", "Control1", &max_angle, 180); //Hue (0 - 179)
	cvCreateTrackbar("min", "Control1", &min_angle, 90);
	//Create trackbars in "Control" window
	cvCreateTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
	cvCreateTrackbar("HighH", "Control", &iHighH, 179);

	cvCreateTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
	cvCreateTrackbar("HighS", "Control", &iHighS, 255);

	cvCreateTrackbar("LowV", "Control", &iLowV, 255); //Value (0 - 255)
	cvCreateTrackbar("HighV", "Control", &iHighV, 255);

	Mat imgOriginal;

	int houghVote = 300;
	int size_destruct = 5;
	int size_fill = 5;

	cvCreateTrackbar("houghVote", "Control1", &houghVote, 400);
	cvCreateTrackbar("fill", "Control1", &size_fill, 10);
	cvCreateTrackbar("destr", "Control1", &size_destruct, 10);
	while (true) {

		Mat imgOriginal;

		bool bSuccess = cap.read(imgOriginal); // read a new frame from video

		if (!bSuccess) //if not success, break loop
		{
			cout << "Cannot read a frame from video stream" << endl;
			break;
		}

		//image.copyTo(imgOriginal);

		Mat imgHSV;

		cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

		Mat imgThresholded;

		inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image

			 //morphological opening (remove small objects from the foreground)
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(size_destruct, size_destruct)));
		dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(size_destruct, size_destruct)));

		//morphological closing (fill small holes in the foreground)
		
		dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(size_fill, size_fill)));
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(size_fill, size_fill)));

		cvCreateTrackbar("fill", "Control", &size_fill, 10);
		cvCreateTrackbar("destr", "Control", &size_destruct, 10);

		//--- Tworzenie konturów
		std::vector<std::vector<cv::Point> > contours;
		std::vector<cv::Point> contours_poly;
		cv::Rect boundRect;
		cv::Mat cont;
		imgThresholded.copyTo(cont);

		/*
		findContours(cont, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

		int max = 100, i_cont = 0;
		cv::Mat drawing = cv::Mat::zeros(cont.size(), CV_8UC3);

		std::vector<int> indexes;

		for (int i = 0; i < contours.size(); i++)
		{
			if (abs(cv::contourArea(cv::Mat(contours[i]))) > max)
			{
				indexes.push_back(i);
			}
		}
		std::cout << "Ilosc: " << indexes.size() << std::endl;
		if (indexes.size() >= 0)
		{
			for (auto index : indexes) {
				approxPolyDP(cv::Mat(contours[index]), contours_poly, 3, true);
				boundRect = cv::boundingRect(cv::Mat(contours_poly));
				fillConvexPoly(imgOriginal, contours_poly, contours_poly.size());
				rectangle(imgOriginal, boundRect.tl(), boundRect.br(), cv::Scalar(125, 250, 125), 2, 8, 0);
				line(imgOriginal, boundRect.tl(), boundRect.br(), cv::Scalar(250, 125, 125), 2, 8, 0);
				line(imgOriginal, cv::Point(boundRect.x + boundRect.width, boundRect.y), cv::Point(boundRect.x, boundRect.y + boundRect.height), cv::Scalar(250, 125, 125), 2, 8, 0);
				drawContours(drawing, contours, index, cv::Scalar(125, 125, 250), 2);
			}
		}*/

		Mat contours1 = imgThresholded;
		//Canny(imgThresholded, contours1, 50, 350);
		//Mat contoursInv;
		//threshold(drawing, contours1, 128, 255, THRESH_BINARY_INV);

		std::vector<Vec2f> lines;
		if (houghVote < 100 or lines.size() > 2) { // we lost all lines. reset
			houghVote = 200;
		}
		else { houghVote += 25; }
		while (lines.size() < 5 && houghVote > 0) {
			HoughLines(contours1, lines, 1, 3.1415 / 180, houghVote,0.0,0.0,min_angle*PI/180,max_angle*PI/180);
			houghVote -= 3;
		}
		std::cout<< "houghVote" << houghVote << "\n";
		std::cout << "min " << min_angle * PI / 180 << ", max" << max_angle * PI / 180 <<"\n";
		Mat result(contours1.rows, contours1.cols, CV_8U, Scalar(255));
		imgOriginal.copyTo(result);

		std::vector<Vec2f>::const_iterator it = lines.begin();
		Mat hough(imgOriginal.size(), CV_8U, Scalar(0));
		while (it != lines.end()) {

			float rho = (*it)[0];   // first element is distance rho
			float theta = (*it)[1]; // second element is angle theta

			//if (theta < 30.*PI / 180. || theta > 150.*PI / 180.) { //     filter theta angle to find lines with theta between 30 and 150 degrees (mostly vertical)

									// point of intersection of the line with first row
				Point pt1(rho / cos(theta), 0);
				// point of intersection of the line with last row
				Point pt2((rho - result.rows*sin(theta)) / cos(theta), result.rows);
				// draw a white line
				line(result, pt1, pt2, Scalar(255), 8);
				line(hough, pt1, pt2, Scalar(255), 8);
			//}
			Point x1(result.cols/2,0);
			Point x2(result.cols / 2, result.rows);

			line(result, x1, x2, Scalar(255), 8);
			std::cout << "line: (" << rho << "," << theta << ")\n";
			++it;
		}

		cv::imshow("result", result);
		cv::imshow("hough", hough);
		cv::imshow("image", imgOriginal);
		//cv::imshow("Contours", drawing);
		cv::imshow("Thresholded Image", imgThresholded); //show the thresholded image
														 //cv::imshow("Original", imgOriginal); //show the original image
		//waitKey(0);
		if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
		{
			cout << "esc key is pressed by user" << endl;
			break;
		}
	}
}
/*
#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
	VideoCapture cap(0); //capture the video from web cam

	if (!cap.isOpened())  // if not success, exit program
	{
		cout << "Cannot open the web cam" << endl;
		return -1;
	}

	namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"

	int iLowH = 34;
	int iHighH = 63;

	int iLowS = 68;
	int iHighS = 154;

	int iLowV = 24;
	int iHighV = 180;

	//Create trackbars in "Control" window
	cvCreateTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
	cvCreateTrackbar("HighH", "Control", &iHighH, 179);

	cvCreateTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
	cvCreateTrackbar("HighS", "Control", &iHighS, 255);

	cvCreateTrackbar("LowV", "Control", &iLowV, 255); //Value (0 - 255)
	cvCreateTrackbar("HighV", "Control", &iHighV, 255);

	int houghVote = 0;

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

		Mat imgThresholded;

		inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image

																									  //morphological opening (remove small objects from the foreground)
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
		dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

		//morphological closing (fill small holes in the foreground)
		dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));



		//--- Tworzenie konturów
		std::vector<std::vector<cv::Point> > contours;
		std::vector<cv::Point> contours_poly;
		cv::Rect boundRect;
		cv::Mat cont;
		imgThresholded.copyTo(cont);


		findContours(cont, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

		int max = 100, i_cont = 0;
		cv::Mat drawing = cv::Mat::zeros(cont.size(), CV_8UC3);

		std::vector<int> indexes;

		for (int i = 0; i< contours.size(); i++)
		{
			if (abs(cv::contourArea(cv::Mat(contours[i]))) > max)
			{
				indexes.push_back(i);
			}
		}
		std::cout << "Ilosc: " << indexes.size() << std::endl;
		if (indexes.size() >= 0)
		{
			for (auto index : indexes) {
				approxPolyDP(cv::Mat(contours[index]), contours_poly, 3, true);
				boundRect = cv::boundingRect(cv::Mat(contours_poly));
				fillConvexPoly(imgOriginal, contours_poly, contours_poly.size());
				rectangle(imgOriginal, boundRect.tl(), boundRect.br(), cv::Scalar(125, 250, 125), 2, 8, 0);
				line(imgOriginal, boundRect.tl(), boundRect.br(), cv::Scalar(250, 125, 125), 2, 8, 0);
				line(imgOriginal, cv::Point(boundRect.x + boundRect.width, boundRect.y), cv::Point(boundRect.x, boundRect.y + boundRect.height), cv::Scalar(250, 125, 125), 2, 8, 0);
				drawContours(drawing, contours, index, cv::Scalar(125, 125, 250), 2);
			}
		}

		Mat contours1 = imgThresholded;
		//Canny(imgOriginal, contours1, 50, 350);
		//Mat contoursInv;
		//threshold(contours1, contoursInv, 128, 255, THRESH_BINARY_INV);

		std::vector<Vec2f> lines;
		if (houghVote < 1 or lines.size() > 2) { // we lost all lines. reset 
			houghVote = 200;
		}
		else { houghVote += 25; }
		while (lines.size() < 5 && houghVote > 0) {
			HoughLines(contours1, lines, 1, 3.1415 / 180, houghVote);
			houghVote -= 5;
		}
		std::cout << houghVote << "\n";
		Mat result(contours1.rows, contours1.cols, CV_8U, Scalar(255));
		imgOriginal.copyTo(result);

		std::vector<Vec2f>::const_iterator it = lines.begin();
		Mat hough(imgOriginal.size(), CV_8U, Scalar(0));
		while (it != lines.end()) {

			float rho = (*it)[0];   // first element is distance rho
			float theta = (*it)[1]; // second element is angle theta

									//if (theta < PI/20. || theta > 19.*PI/20.) { // filter theta angle to find lines with theta between 30 and 150 degrees (mostly vertical)

									// point of intersection of the line with first row
			Point pt1(rho / cos(theta), 0);
			// point of intersection of the line with last row
			Point pt2((rho - result.rows*sin(theta)) / cos(theta), result.rows);
			// draw a white line
			line(result, pt1, pt2, Scalar(255), 8);
			line(hough, pt1, pt2, Scalar(255), 8);
			//}

			std::cout << "line: (" << rho << "," << theta << ")\n"; 
			++it;
		}

		cv::imshow("result", result);
		cv::imshow("hough", hough);
		//cv::imshow("Contours", drawing);
		cv::imshow("Thresholded Image", imgThresholded); //show the thresholded image
		//cv::imshow("Original", imgOriginal); //show the original image



		if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
		{
			cout << "esc key is pressed by user" << endl;
			break;
		}
	}

	return 0;

}*/
/*#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

int main() {
	Mat image;
	Mat imgCanny;
	image = imread("grzdki5.jpg");
	if (!image.data)                              // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		system("Pause");
		return -1;
	}
	string name = "Display window";
	Canny(image, imgCanny, 50, 350);					// input image, output image, low threshold, high threshold
	namedWindow(name, WINDOW_AUTOSIZE);// Create a window for display.
	imshow(name, imgCanny);
	cv::waitKey(0);
	return 0;
}
*/