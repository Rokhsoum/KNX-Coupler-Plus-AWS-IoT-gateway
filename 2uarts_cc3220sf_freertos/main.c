/**
 * main.c
 */

#include "knx_link.h"
#include "knx_app.h"

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/apps/Button.h>
#include <ti/drivers/apps/LED.h>

/* Driver configuration */
#include "ti_drivers_config.h"


int main(void) {

    Button_Params buttonParams;
    Button_Handle buttonLeft;
    Button_Handle buttonRight;
    Button_EventMask events;

    Button_Params_init(&buttonParams);

    buttonLeft = Button_open(CONFIG_BUTTON_0, &buttonParams); //(Callback)
    buttonRight = Button_open(CONFIG_BUTTON_1, &buttonParams);

    if (Button_EV_CLICKED) {
        // Received a click, handle app condition 0 etc
        ButtonLefttCallback(buttonLeft, events);
        knxAppThread();
        ledVerdeAppThread();
    }

    if (Button_EV_CLICKED) {
        // Received a click, handle app condition 0 etc
        ButtonLefttCallback(buttonRight, events);
        knxAppThread();
        ledAmarilloAppThread();
    }

	return 0;
}
