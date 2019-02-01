/**
 * Copyright (c) 2014 - 2018, Nordic Semiconductor ASA
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 * 
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 * 
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 * 
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
/** @file
 *
 * @defgroup blinky_example_main main.c
 * @{
 * @ingroup blinky_example
 * @brief Blinky Example Application main file.
 *
 * This file contains the source code for a sample application to blink LEDs.
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "boards.h"
#include "nrfx_uart.h"
#include "nrfx_timer.h"
#include "nrf_gpio.h"
#include "touch.h"
#include "SEGGER_RTT.h"
#include "identification_controller.h"
/**
 * @brief Function for application main entry.
 */

nrfx_uart_t huart1;

void uart_event_handler(nrfx_uart_event_t * p_event, void* p_context){
      switch (p_event->type)
    {
        case NRFX_UART_EVT_TX_DONE:
          SEGGER_RTT_WriteString(0, "UART TX DONE");
        default:
            break;
    }
}
void config_uart()
{
	char data[20] = "Funciona UART \n";
        char* tst = "H";
        nrfx_err_t err;
	nrfx_uart_config_t huart1_config = NRFX_UART_DEFAULT_CONFIG;
	//huart1_config.pseltxd = RX_PIN_NUMBER;
	//huart1_config.pselrxd = TX_PIN_NUMBER;
	huart1_config.pseltxd = NRF_GPIO_PIN_MAP(1, 2);
	huart1_config.pselrxd = NRF_GPIO_PIN_MAP(1, 1);
//	huart1_config.pselcts = CTS_PIN_NUMBER;
//	huart1_config.pselrts = RTS_PIN_NUMBER;
//	huart1_config.p_context = NULL;
	huart1_config.hwfc = NRF_UART_HWFC_DISABLED;
	huart1_config.parity = NRF_UART_PARITY_EXCLUDED;
	huart1_config.baudrate = NRF_UART_BAUDRATE_9600;
	huart1_config.interrupt_priority = 2;
        huart1.p_reg = NRFX_CONCAT_2(NRF_UART, 0);             
        huart1.drv_inst_idx = NRFX_CONCAT_3(NRFX_UART, 0, _INST_IDX);
	err = nrfx_uart_init(&huart1, &huart1_config, NULL);
	//err= nrfx_uart_tx(&huart1, tst, 1);
}

int main(void)
{
    /* Configure board. */
        SEGGER_RTT_WriteString(0, "HOLA");
        bsp_board_init(BSP_INIT_LEDS);
	bsp_board_leds_off();
	nrf_delay_ms(500);
	bsp_board_leds_on();
	//Uart configuration
	config_uart();
        gt511c3_initialize(); 
        gt511c3_reset_data();
        identification_enroll_user(1);
        gt511c3_deinitialize();
        gt511c3_initialize();
        identification_check(1);
        while (true)
        {
            for (int i = 0; i < LEDS_NUMBER; i++)
            {
                bsp_board_led_invert(i);
                nrf_delay_ms(500);
            }
        }
}

/**
 *@}
 **/
