#include <unistd.h>
#include "capture/webcam_capture.h"
#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
	// Open the default webcam (device 0)
	cv::VideoCapture cap(0);
	if (!cap.isOpened()) {
		std::cerr << "ERROR: Could not open webcam." << std::endl;
		return 1;
	}
	cv::namedWindow("Live Webcam", cv::WINDOW_AUTOSIZE);
	cv::Mat frame;
	while (true) {
		cap >> frame;
		if (frame.empty()) break;
		cv::imshow("Live Webcam", frame);
		// Exit on ESC key (ASCII 27)
		if (cv::waitKey(10) == 27) break;
	}
	cap.release();
	cv::destroyAllWindows();
	return 0;
}

