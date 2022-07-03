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

    knxLinkHandle_t *link = NULL;
    int frame_index = 0;

    knxLinkSetAddressReq(link, frame_index);
    knxLinkResetReq(link);
    knxLinkResetCon(link);

    sendDataReq(frame_index);
    knxLinkDataReq(link, frame_index);
    knxLinkSendDataCon(link);
    knxLinkDataInd(link);
    recvDataCon();
    recvDataInd(frame_index);


    knxAppInit();

    Button_Params buttonParams;
    Button_Handle buttonLeft;
    Button_Handle buttonRight;

    Button_Params_init(&buttonParams);

    buttonLeft = Button_open(CONFIG_BUTTON_0, &buttonParams); //mettre Callback
    buttonRight = Button_open(CONFIG_BUTTON_1, &buttonParams);

    knxAppThread();
    knxAppRecvThread();

    ledVerdeAppThread();
    ledAmarilloAppThread();

	return 0;
}
