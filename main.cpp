#include <opencv2/highgui.hpp>    // for imshow & waitKey
#include "Webcam.h"
#include "View.h"
#include "Image_utils.h"
#include "Params.h"
#include <vector>

int main() {
    Webcam wc("/dev/video0");
    bool running = true;

    auto frame_opt = wc.capture();
    if (!frame_opt) return -1;

    Image img_color = *frame_opt;
    Image img_skin(img_color.width, img_color.height, 1);
    Image img_gray(img_color.width, img_color.height, 1);

    View view;

    while (running) {
        frame_opt = wc.capture();
        if (!frame_opt) break;

        img_color = *frame_opt;

        blur_mask(img_color, img_color, 3);
        gray_mask(img_color, img_gray);
        skin_mask(img_color, img_skin);

        int thresh = 30;
        for (int i = 0; i < img_gray.width * img_gray.height; ++i) {
            img_gray.data[i] = (img_gray.data[i] > thresh) ? params::mask_value_on : params::mask_value_off;
        }

        auto boxes = find_bounding_boxes(img_gray);

        for (const auto& box : boxes) {
            draw_rect(img_color, box, 0, 255, 0);
        }

        int key = cv::waitKey(30);
        view.handle_key(key);

        cv::Mat mat = view.get_display_image(img_color, img_gray, img_skin);
        view.draw_overlay(mat);

        cv::imshow("webcam with boxes", mat);

        if (key == 27) running = false;
    }

    wc.close();
    return 0;

}