/*
 * Copyright (c) 2022 EdgeImpulse Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an "AS
 * IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language
 * governing permissions and limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "uart_ep.h"
#include "hal_data.h"
#include <stdio.h>

/*******************************************************************************************************************//**
 * @addtogroup r_SCI_B_UART_ep
 * @{
 **********************************************************************************************************************/

#define UART_RX_BUFFER_SIZE         512
#define RESET_VALUE                 (0x00)

/*
 * Private function declarations
 */
const sci_b_baud_setting_t g_uart3_baud_max_setting =
        {
        /* Baud rate calculated with 1.725% error. */.baudrate_bits_b.abcse = 0,
          .baudrate_bits_b.abcs = 0, .baudrate_bits_b.bgdm = 1, .baudrate_bits_b.cks = 0, .baudrate_bits_b.brr = 3, .baudrate_bits_b.mddr =
                  (uint8_t) 256,
          .baudrate_bits_b.brme = false };

const sci_b_baud_setting_t g_uart3_baud_default_setting =
        {
        /* Baud rate calculated with 1.725% error. */.baudrate_bits_b.abcse = 0,
          .baudrate_bits_b.abcs = 0, .baudrate_bits_b.bgdm = 1, .baudrate_bits_b.cks = 0, .baudrate_bits_b.brr = 31, .baudrate_bits_b.mddr =
                  (uint8_t) 256,
          .baudrate_bits_b.brme = false };

/*
 * Private global variables
 */
/* Temporary buffer to save data from receive buffer for further processing */
static uint8_t g_temp_buffer[UART_RX_BUFFER_SIZE] = {RESET_VALUE};

/* Flag RX completed */
static volatile uint8_t g_uart_rx_completed = false;

/* Flag TX completed */
static volatile uint8_t g_uart_tx_completed = false;
static volatile uint8_t g_uart_tx_empty = true;


/* Flag error occurred */
static volatile uint8_t g_uart_error = false;

/* Counter to update g_temp_buffer index */
static volatile uint16_t g_rx_index = RESET_VALUE;

/* Index of data sent to at hanlder */
static volatile uint16_t g_uart_rx_read_index = RESET_VALUE;

/*******************************************************************************************************************//**
 * @brief       Initialize  UART.
 * @param[in]   None
 * @retval      FSP_SUCCESS         Upon successful open and start of timer
 * @retval      Any Other Error code apart from FSP_SUCCESS  Unsuccessful open
 ***********************************************************************************************************************/
fsp_err_t uart_initialize(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Initialize UART channel with baud rate 115200 */
    err = R_SCI_B_UART_Open (&g_uart3_ctrl, &g_uart3_cfg);

    return err;
}

fsp_err_t uart_set_baud(bool is_max_baud)
{
    fsp_err_t err = FSP_SUCCESS;

    if (is_max_baud == true) {
        err = R_SCI_B_UART_BaudSet(&g_uart3_ctrl, &g_uart3_baud_max_setting);
    }
    else{
        err = R_SCI_B_UART_BaudSet(&g_uart3_ctrl, &g_uart3_baud_default_setting);
    }

    return err;
}

/*****************************************************************************************************************
 *  @brief       print user message to terminal
 *  @param[in]   p_msg
 *  @retval      FSP_SUCCESS                Upon success
 *  @retval      FSP_ERR_TRANSFER_ABORTED   Upon event failure
 *  @retval      Any Other Error code apart from FSP_SUCCESS,  Unsuccessful write operation
 ****************************************************************************************************************/
fsp_err_t uart_print_user_msg(uint8_t *p_msg, uint16_t msg_len)
{
    fsp_err_t err   = FSP_SUCCESS;

    /* Reset callback capture variable */
    g_uart_tx_completed = false;
    g_uart_error = false;

#if 0
    volatile uint8_t *pbuffer = p_msg;
    volatile uint16_t i;

    for (i = 0; i < msg_len; i++) {
        //err = R_SCI_B_UART_Write (&g_uart3_ctrl, p_msg, 1);
        //while(g_uart_tx_completed == false){};
        uart_putc(*p_msg);
        g_uart_tx_completed = false;
        p_msg++;
    }

    return FSP_SUCCESS;

#else
    //while ((g_uart_tx_empty == false)) {};
    /* Writing to terminal */
    err = R_SCI_B_UART_Write (&g_uart3_ctrl, p_msg, msg_len);
    if (FSP_SUCCESS != err) {
        return err;
    }

    /* Check for event transfer complete */
    while (g_uart_tx_completed == false) {
        /* Check if any error event occurred */
        if (g_uart_error == true) {
            return FSP_ERR_TRANSFER_ABORTED;
        }

    }

    return err;
#endif
}

/**
 *
 * @param c
 */
void uart_putc(uint8_t c)
{
    R_SCI_B_UART_Write(&g_uart3_ctrl, &c, 1);
    while(g_uart_tx_empty == false){};

    g_uart_tx_empty = false;
}

void DumpChar(char data)
{
    //while (g_uart3_ctrl.p_reg->SSR_SMCI_b.TDRE == 0);
    //g_uart3_ctrl.p_reg->TDR = data;
    //g_uart3_ctrl.p_reg->SSR_SMCI_b.TDRE = 0;

    /* Transmit interrupts must be disabled to start with. */
    g_uart3_ctrl.p_reg->CCR0 &= (uint32_t) ~(R_SCI_B0_CCR0_TIE_Msk | R_SCI_B0_CCR0_TEIE_Msk);
    while (g_uart3_ctrl.p_reg->CSR_b.TEND == 0);

    g_uart3_ctrl.p_reg->CCR0 &= (uint32_t) ~(R_SCI_B0_CCR0_TE_Msk);
    while (g_uart3_ctrl.p_reg->CESR_b.TIST == 1);

    g_uart3_ctrl.p_reg->TDR = (uint32_t)data;
    // CCR1.CTSE 

    //g_uart3_ctrl.p_reg->CCR0 |= (uint32_t) (R_SCI_B0_CCR0_TE_Msk | R_SCI_B0_CCR0_TIE_Msk);  // fire interrupt
}

/**
 * @brief Returns the local rx uart buffer only if reception is complete (ie received a '\r\n') or if inference is running (just care of 'b')
 *
 * @param is_inference_running
 * @return
 */
char uart_get_rx_data(uint8_t is_inference_running)
{
    char ret_val = -1;

    if (g_uart_rx_completed == true)
    {
        if (g_uart_rx_read_index < g_rx_index)
        {
            ret_val = (char)g_temp_buffer[g_uart_rx_read_index++];

            if (g_uart_rx_read_index == g_rx_index)
            {
                g_rx_index = 0;
                g_uart_rx_read_index = 0;
                g_uart_rx_completed = false;
            }
        }
        else
        {
            g_rx_index = 0;
            g_uart_rx_read_index = 0;
        }
    }
    else if (is_inference_running)
    {
        /* need to check if any 'b' */
        uint16_t i;

        for (i = g_uart_rx_read_index; i< g_rx_index; i ++)
        {
            if (g_temp_buffer[i] == 'b')
            {
                ret_val = 'b';
                break;
            }
        }
    }

    return ret_val;
}

/*******************************************************************************************************************//**
 *  @brief       Deinitialize SCI UART module
 *  @param[in]   None
 *  @retval      None
 **********************************************************************************************************************/
void deinit_uart(void)
{
    //fsp_err_t err = FSP_SUCCESS;

    /* Close module */
    R_SCI_B_UART_Close (&g_uart3_ctrl);
}

/*****************************************************************************************************************
 *  @brief      UART user callback
 *  @param[in]  p_args
 *  @retval     None
 ****************************************************************************************************************/
void user_uart_callback(uart_callback_args_t *p_args)
{
    /* Logged the event in global variable */
    //g_uart_event = (uint8_t)p_args->event;

    switch(p_args->event)
    {
        case UART_EVENT_RX_CHAR:
        {
            g_temp_buffer[g_rx_index++] = (uint8_t ) p_args->data;
            if (p_args->data == CARRIAGE_ASCII) {
                g_uart_rx_completed = true;
            }
        }
        break;
        case UART_EVENT_RX_COMPLETE:
        {
            g_uart_rx_completed = true;
        }
        break;
        case UART_EVENT_TX_COMPLETE:
        {
            g_uart_tx_completed = true;
        }
        break;
        case UART_EVENT_ERR_PARITY:
        case UART_EVENT_ERR_FRAMING:
        case UART_EVENT_ERR_OVERFLOW:
        case UART_EVENT_BREAK_DETECT:
        {
            g_uart_error = true;
        }
        break;
        case UART_EVENT_TX_DATA_EMPTY:
        default:
        {
            g_uart_tx_empty = true;
        }
        break;
    }
}

/*******************************************************************************************************************//**
 * @} (end addtogroup r_SCI_B_UART_ep)
 **********************************************************************************************************************/
