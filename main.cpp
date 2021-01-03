#include "camera.h"
#include "vec.h"

#include <cstdlib>
#include <cstdio>

int main(void) {

    const int image_width = 256;
    const int image_height = 256;

    printf("P3\n%d %d\n255", image_width, image_height);

    for (int j = image_height - 1; j >= 0; --j) {
        for (int i = 0; i < image_width; ++i) {
            auto r = double(i) / (image_width - 1);
            auto g = double(j) / (image_height - 1);
            auto b = 0.25;

            int ir = static_cast<int>(255.999 * r);
            int ig = static_cast<int>(255.999 * g);
            int ib = static_cast<int>(255.999 * b);

            printf("%d %d %d\n", ir, ig, ib);
        }
    }

    return 0;
}
