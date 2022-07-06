/**
 * main.c
 */

#include <knx_link_conf.h>
#include "knx_link.h"
#include "knx_app.h"
#include "knx_link_internal.h"

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/apps/Button.h>
#include <ti/drivers/apps/LED.h>

/* Driver configuration */
#include "ti_drivers_config.h"


int main(void) {

    knxLinkInit(0, 0, 0);

    knxAppInit();

    Button_Params buttonParams;
    Button_Handle buttonLeft;
    Button_Handle buttonRight;

    Button_Params_init(&buttonParams);

    buttonLeft = Button_open(CONFIG_BUTTON_0, &buttonParams); //mettre Callback
    buttonRight = Button_open(CONFIG_BUTTON_1, &buttonParams);



	return 0;
}
