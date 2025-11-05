#include <iostream>
#include <unistd.h>
#include "capture/webcam_capture.h"

int main() {
	char cwd[1024];
	if (getcwd(cwd, sizeof(cwd)) != NULL) {
		std::cout << "current working dir: " << cwd << "\n";
	} else {
		perror("getcwd error");
	}

	capture::WebcamCapture cam("/dev/video0");

	if(!cam.open_device()) {
		std::cerr << "failed to open webcam buddy...";
		return 1;
	}

	auto frame = cam.capture_frame();

	if (frame) {
		frame->save_to_ppm("frame_output.ppm");
		std::cout << "captured and saved frame :)";
	} else {
		std::cerr << "failed to capture bruv :(";
	}

	cam.close_device();
	return 0;
}
