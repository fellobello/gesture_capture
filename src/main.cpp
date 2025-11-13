#include "gesture_capture.h"

int main() {
    GestureCapture gc("/dev/video0", false);
    if (gc.open_device() != 0) {
        // error
        std::cerr << "error: could not open webcam." << std::endl;
        return 1;
    }
    gc.run();
    gc.close_device();
    return 0;
}