#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "xtimer.h"

#include "il0373.h"
#include "il0373_pictures.h"
#include "il0373_params.h"

const uint16_t riot_32_height = sizeof(riot_logo_32)/sizeof(riot_logo_32[0]);

/* Draw the large RIOT logo with a full refresh */
void draw_riot(il0373_t *dev) {
    il0373_init_full(dev);
    il0373_fill_pixels(dev, (uint8_t*)riot_icon_200, (uint8_t*)riot_icon_200, sizeof riot_icon_200);
}

int main(void)
{
    il0373_t dev;
    int init = il0373_init(&dev, &il0373_params[0], DISPLAY_X, DISPLAY_Y);
    if (init != 0) {
        printf("IL0373 INIT FAILED: %i\n", init);
        return 1;
    }

    xtimer_ticks32_t last_wakeup = xtimer_now();
    while (1) {
        draw_riot(&dev);
        xtimer_periodic_wakeup(&last_wakeup, 3 * US_PER_SEC);
        il0373_clear(&dev);
    }

    return 0;
}
