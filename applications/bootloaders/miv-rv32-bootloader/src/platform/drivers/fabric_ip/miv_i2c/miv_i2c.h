/*******************************************************************************
 * Copyright 2022 Microchip FPGA Embedded Systems Solutions.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * This file contains the application programming interface for the MI-V Soft IP
 * I2C module driver. This module is delivered as a part of Mi-V Extended
 * Sub-System(MIV_ESS).
 */

/*=========================================================================*//**
  @mainpage Mi-V I2C Bare Metal Driver

  ==============================================================================
  Introduction
  ==============================================================================
  The Mi-V I2C driver provides a set of functions for controlling the Mi-V I2C
  Soft-IP module. This module is delivered as a part of the Mi-V  Extended
  Sub System(MIV_ESS). The driver provides a minimal APB-driven I2C interface,
  supporting initiator read and write access to  peripheral I2C devices.

  The major features provided by the Mi-V I2C driver are:
      - Support for configuring the I2C instance.
      - I2C master operations.
      - I2C ISR.

  This driver can be used as part of a bare metal system where no operating
  system  is available. The driver can be adapted for use as part of an
  operating system, but the implementation of the adaptation layer between the
  driver and  the operating system's driver model is outside the scope of this
  driver.
  
  ==============================================================================
  Hardware Flow Dependencies
  ==============================================================================
  The application software should initialize and configure the Mi-V I2C through
  the call to the MIV_I2C_init() and MIV_I2C_config() function for Mi-V I2C
  instance in the design. The configuration parameter include base address and
  Prescaler value.

  ------------------------------
  Interrupt Control
  ------------------------------
  The Mi-V I2C driver has to enable and disable the generation of interrupts by
  Mi-V I2C at various times while operating. This enabling and disabling of the
  interrupts must be done through the Mi-V RV32 HAL provided interrupt handlers.
  For that reason, the method controlling the Mi-V I2C interrupts is system
  specific and it is necessary to customize the MIV_I2C_enable_irq() and
  MIV_I2C_disable_irq() functions as per requirement.

  The implementation of MIV_I2C_enable_irq() should permit the interrupts
  generated by the Mi-V I2C to the processor through a call to respective miv-hal
  interrupt handler. The implementation of MIV_I2C_disable_irq() should prevent
  the interrupts generated by a Mi-V I2C from interrupting the processor.
  Please refer to the miv_i2c_interrupt.c for more information about the
  implementation.

  No MIV_I2C hardware configuration parameters are used by the driver, apart
  from the MIV_I2C base address. Hence, no additional configuration files
  are required to use the driver.

  ==============================================================================
  Theory of Operation
  ==============================================================================
  The Mi-V I2C software driver is designed to allow the control of multiple
  instances of Mi-V I2C. Each instance of Mi-V I2C in the hardware design is
  associated with a single instance of the miv_i2c_instance_t structure in the
  software. User must allocate memory for one unique miv_i2c_instance_t
  structure for each instance of Mi-V I2C in the hardware.
  A pointer to the structure is passed to the subsequent driver functions in
  order to identify the MIV_I2C hardware instance and to perform requested
  operation.

  Note: Do not attempt to directly manipulate the contents of the
  miv_i2c_instance_t structure. These structures are only intended to be modified
  by the driver functions.

  The Mi-V I2C driver functions are grouped into following categories:
      - Initialization and configuration
      - I2C master operation functions to handle write, read and write_read
        operations.
      - Interrupt control

  --------------------------------
  Initialization and configuration
  --------------------------------
  The Mi-V I2C device is first initialized by the call to MIV_I2C_init(). This
  function initializes the instance of Mi-V I2C with the base address.
  MIV_I2C_init() function must be called before any other Mi-V I2C driver API.

  The configuration of the Mi-V I2C instance is done via call to the
  MIV_I2C_config() function. This function will set the prescale value which is
  used to set the frequency of the I2C clock(SCLK) generated by I2C module.

  ---------------------------------
  Transaction types
  ---------------------------------
  The driver is designed to handle three types of transactions:
   - Write transactions
   - Read transactions
   - Write-Read transaction

   ### Write Transaction
   The write transaction begins with master sending a start condition, followed
   by device address byte with the R/W bit set to logic '0', and then by the
   word address bytes. The slave acknowledges the receipt of its address with
   acknowledge bit. The master sends one byte at a time to the slave, which must
   acknowledge the receipt of each byte for the next byte to be sent. The master
   sends STOP condition to complete the transaction. The slave can abort the
   transaction by replying with negative acknowledge.

   The application programmer can choose not to send the STOP bit at the end of
   the transaction causing repetitive start conditions.

   ### Read Transaction
   The master I2C device initiates a read transaction by sending a START bit
   as soon as the bus becomes free. The start condition is followed by the
   control byte which contains 7-bit slave address followed by R/W bit set to
   logic '1'. The slave sends data one byte at a time to the master, which must
   acknowledge receipt of each byte for the next byte to be sent. The master
   sends a non-acknowledge bit following the last byte it wishes to read
   followed by a STOP bit.

   The application programmer can choose not to send a STOP bit at the end of
   the transaction causing the next transaction to begin with a repeated
   START bit.

   ### Write-Read Transaction
   The write read transaction is a combination of a write transaction
   immediately followed by a read transaction. There is no STOP condition sent
   between the write and read phase of write-read transaction. A repeated START
   condition is sent between the write and read phases.

   Whilst the write handler is being executed, the slave holds the clock line
   low to stretch the clock until the response is ready.

   The write-read transaction is typically used to send an memory/register
   address in the write transaction specifying the start address of the data to
   be transferred during the read phase.

   The application programmer can choose not to send a STOP bit at the end of
   the transaction causing the next transaction to begin with a repeated
   START bit.

   -------------------------------------
   Interrupt Control
   -------------------------------------
   The Mi-V I2C driver is interrupt driven and it uses the MIV_I2C_irq() function
   to drive the ISR state machine which is at the heart of the driver. The
   application is responsible for providing the link between the interrupt
   generating hardware and the Mi-V I2C interrupt handler and must ensure that
   the MIV_I2C_isr() function is called with the correct miv_i2c_instance_t
   structure pointer for the Mi-V I2C instance initiating the interrupt.

*//*=========================================================================*/
#ifndef MIV_I2C_H_
#define MIV_I2C_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mivi2c_regs.h"
#include <string.h>

#ifndef LEGACY_DIR_STRUCTURE
#include "hal/hal.h"
#else
#include "hal.h"
#endif

/*-------------------------------------------------------------------------*//**
   The miv_i2c_status_t type is used to report the status of I2C transactions.
 */
typedef enum miv_i2c_status
{
    MIV_I2C_SUCCESS = 0u,
    MIV_I2C_IN_PROGRESS,
    MIV_I2C_FAILED,
    MIV_I2C_TIMED_OUT
}miv_i2c_status_t;

/*-------------------------------------------------------------------------*//**
  This structure is used to identify the MIV_I2C hardware instances in a system.
  Your application software should declare one instance of this structure for
  each instance of the MIV_I2C in your system. The function MIV_I2C_init()
  Initializes this structure. A pointer to an initialised instance of the structure
  should be passed as the first parameter to the MIV_I2C driver functions, to
  identify which MIV_I2C hardware instance should perform the requested operation.

  The contents of this data structure should not be modified or used outside of
  the MIV_I2C driver. Software using the MIV_I2C driver should only need to
  create one single instance of this data structure for each MIV_I2C hardware
  instance in the system, then pass a pointer to these data structures with each
  call to the MIV_I2C driver in order to identify the MIV_I2C hardware instance
  it wishes to use.
*/

typedef struct miv_i2c_instance
{
    addr_t base_addr;

    /* Transmit related info:*/
    uint_fast8_t target_addr;

    /* Current transaction type */
    uint8_t transaction;

    uint8_t bus_options;

    uint8_t ack_polling_options;

    /* Current State of the I2C master */
    uint8_t master_state;

    /* Master TX INFO: */
    const uint8_t * master_tx_buffer;
    uint_fast16_t master_tx_size;
    uint_fast16_t master_tx_idx;
    uint_fast8_t dir;

    /* Master RX INFO: */
    uint8_t * master_rx_buffer;
    uint_fast16_t master_rx_size;
    uint_fast16_t master_rx_idx;

    /* Master Status */
    volatile miv_i2c_status_t master_status;
    uint32_t master_timeout_ms;

    /* user  specific data */
    void *p_user_data ;

    /* I2C bus status */
    uint8_t bus_status;

    /* Is transaction pending flag */
    uint8_t is_transaction_pending;

    /* I2C Pending transaction */
    uint8_t pending_transaction;

}miv_i2c_instance_t;


/*-------------------------------------------------------------------------*//**
  MIV_I2C_RELEASE_BUS 
  =====================
  The MIV_I2C_RELEASE_BUS constant is used to specify the bus_options parameter
  for MIV_I2C_read(), MIV_I2C_write() and MIV_I2C_write_read() to indicate
  that a STOP bit must be generated at the end of the I2C transaction to release
  the bus.
 */
#define MIV_I2C_RELEASE_BUS     					0x00u


/*-------------------------------------------------------------------------*//**
  MIV_I2C_HOLD_BUS 
  =====================
  The MIV_I2C_HOLD_BUS constant is used to specify the bus_optionsparameter
  for MIV_I2C_read(), MIV_I2C_write() and MIV_I2C_write_read() to indicate
  that a STOP bit must not be generated at the end of the I2C transaction in
  order to retain the bus ownership. This causes the next transaction to
  begin with a repeated START bit and no STOP bit between the transactions.
 */
#define MIV_I2C_HOLD_BUS        					0x01u

/*-------------------------------------------------------------------------*//**
  MIV_I2C_ACK_POLLING_DISABLE 
  =====================
  The MIV_I2C_ACK_POLLING_DISABLE constant is used to specify the
  ack_polling_options parameter to functions MIV_I2C_write(),
  MIV_I2C_write_read() and MIV_I2C_read(). Acknowledgment polling is used when
  working with I2C memory devices such as EEPROM, which feature an internal
  write cycle.

  With acknowledgment polling disabled, if the target slave device responds to the
  control byte with a NACK, the MIV_I2C will abort the transfer.
 */
#define MIV_I2C_ACK_POLLING_DISABLE 				0x00u

/*-------------------------------------------------------------------------*//**
  MIV_I2C_ACK_POLLING_ENABLE 
  =====================
  The MIV_I2C_ACK_POLLING_ENABLE constant is used to specify the
  ack_polling_options parameter to functions MIV_I2C_write(),
  MIV_I2C_write_read() and MIV_I2C_read(). acknowledgment polling is used when
  working with I2C memory devices such as EEPROM, which feature an internal
  write cycle.

  With acknowledgment polling enabled, if the slave device responds to the
  control byte with a NACK, the MIV_I2C will repeatedly transmit another control
  byte until the slave device accepts the connection with an ACK, or the timeout
  specified in the MIV_I2C_wait_complete() function is reached. Acknowledgment
  polling allows for the next read/write operation to be started as soon as the
  EEPROM has completed its internal write cycle.
 */
#define MIV_I2C_ACK_POLLING_ENABLE  				0x01u

/*--------------------------------Public APIs---------------------------------*/

/*-------------------------------------------------------------------------*//**
  The MIV_I2C_init() function is used to initialize the Mi-V I2C module instance
  with the base address.

  Note: This function should be called before calling any other Mi-V I2C 
        functions.

  @param this_i2c
                   A pointer to the miv_i2c_instance_t data structure which
                   will hold all the data related to the Mi-V I2C module 
				   instance being used. A pointer to this structure is passed to 
				   rest of the Mi-V I2C driver functions for operation.
  @param base_addr
                   Base address of the Mi-V I2C module instance in the MIV_ESS 
				   soft IP.

  @return
                  This function does not return any value.

  Example:
  @code
    #define MIV_I2C_BASE_ADDR  0x7A000000u

    miv_i2c_instance_t g_miv_i2c_inst;

    void main( void )
    {
        MIV_I2C_init( &g_miv_i2c_inst, MIV_I2C_BASE_ADDR);
    }
  @endcode
 */
void
MIV_I2C_init
(
    miv_i2c_instance_t *this_i2c,
    addr_t base_addr
);

/*-------------------------------------------------------------------------*//**
  The MIV_I2C_config() function is used to configure the Mi-V I2C module. This
  function will set the prescale value which is used to set the frequency of
  the I2C clock(SCLK) generated by I2C module and also enables the I2C core and
  interrupts.

  @param this_i2c
                   A pointer to the miv_i2c_instance_t data structure which
                   will hold all the data related to the Mi-V I2C module 
				   instance being used. A pointer to this structure is passed to 
				   rest of the Mi-V I2C driver functions for operation.

  @param clk_prescale
                  The value used to set the frequency of Mi-V I2C serial clock
                  (SCLK) generated by the Mi-V I2C module instance. The 
				  prescaler value required to set particular frequency of 
				  Mi-V I2C can be calculated using following formula:

  prescaler = (System Clock Frequency) / (5 * (Desired I2C Clock Frequency)) - 1

  @return
                  This function does not return any value.


  Example:
  @code
    #define MIV_I2C_BASE_ADDR  0x7A000000u

    miv_i2c_instance_t g_miv_i2c_inst;

    void main( void )
    {
        MIV_I2C_init( &g_miv_i2c_inst, MIV_I2C_BASE_ADDR);

        Configuring Mi-V I2C core at Normal Speed (100MHz) for 50MHz Sys clock.
        MIV_I2C_config(&g_miv_i2c_inst, 0x63);
    }
  @endcode
 */
void
MIV_I2C_config
(
    miv_i2c_instance_t *this_i2c,
    uint16_t clk_prescale
);


uint8_t
MIV_I2C_start
(
    miv_i2c_instance_t *this_i2c
);


uint8_t
MIV_I2C_stop
(
    miv_i2c_instance_t *this_i2c
);

/*-------------------------------------------------------------------------*//**
  The MIV_I2C_write() is used to set up and start the Mi-V I2C master write
  transaction. This function is used for all Mi-V master write operation.

  For more information about the operation, please refer to the 'theory of
  operations' section at the start of this document.

  This function returns immediately after initiating the transaction. The content
  of the write buffer passed as parameter should not be modified until the write
  transaction completes. It also means that the memory allocated for the write
  buffer should not be freed or should not go out of scope before the write
  completes.
  You can check for the write transaction completion by polling the master_status
  from miv_i2c_instance_t structure as shown in the sample code.

  @param this_i2c
                   A pointer to the miv_i2c_instance_t data structure which
                   will hold all the data related to the Mi-V I2C module 
				   instance being used. A pointer to this structure is passed to 
				   rest of the Mi-V I2C driver functions for operation.

  @param i2c_target_addr
                  This parameter specifies the serial address for the slave
                  device.

  @param write_buffer
                  This parameter is a pointer to the buffer holding data to be
                  written to target I2C device.
                  Care must be taken not to release the memory used by this
                  buffer before the write transaction completes.

  @param write_size
                  Number of bytes held in the write_buffer to be written to the
                  I2C device.
  @param bus_options:
                  The bus_options parameter is used to indicate if the I2C bus
                  should be released on completion of the write transaction.
                  Using the MIV_I2C_RELEASE_BUS constant for the bus_options
                  parameter causes a STOP bit to be generated at the end of the
                  write transaction causing the bus to be released for other I2C
                  devices to use. Using the MIV_I2C_HOLD_BUS constant as
                  bus_options parameter prevents a STOP bit from being generated
                  at the end of the write transaction, preventing other I2C
                  devices from initiating a bus transaction.

  @param ack_polling_options:
                 The ack_polling_options parameter is used to indicate how the
                 MIV_I2C will respond if the slave device transmits a NACK to
                 the I2C control byte. Using the MIV_I2C_ACK_POLLING_DISABLE
                 constant for the ack_polling_options parameter causes the
                 MIV_I2C to abort the transfer if the slave device responds to
                 the I2C control byte with a NACK. Using the
                 MIV_I2C_ACK_POLLING_ENABLE constant for the ack_polling_options
                 parameter causes the MIV_I2C to repeatedly transmit a control
                 byte to the slave device until the slave device responds with
                 an ACK.
  @return
                 This function does not return any value.


  Example:
  @code
    #define MIV_I2C_BASE_ADDR  0x7A000000u

    miv_i2c_instance_t g_miv_i2c_inst;

    void main( void )
    {
        MIV_I2C_init( &g_miv_i2c_inst, MIV_I2C_BASE_ADDR);

        Configuring Mi-V I2C core at Normal Speed (100MHz) for 50MHz Sys clock.
        MIV_I2C_config(&g_miv_i2c_inst, 0x63);

        MIV_I2C_write (&g_miv_i2c_inst,
                       DUALEE_SLAVEADDRESS_1,
                       i2c_tx_buffer,
                       transfer_size,
                       MIV_I2C_RELEASE_BUS,
                       MIV_I2C_ACK_POLLING_ENABLE
                      );

        // Wait till the miv i2c status changes
        do {
            miv_i2c_status = miv_i2c.master_status;
        }while (MIV_I2C_IN_PROGRESS == miv_i2c_status);
    }
  @endcode
 */
void
MIV_I2C_write
(
    miv_i2c_instance_t *this_i2c,
    uint8_t i2c_target_addr,
    const uint8_t *write_buffer,
    uint16_t write_size,
    uint8_t bus_options,
    uint8_t ack_polling_options
);

/*-------------------------------------------------------------------------*//**
  The MIV_I2C_read() is used to set up and start the Mi-V I2C master read
  transaction. This function is used for all MIV_I2C master read operation.

  For more information about the operation, please refer to the 'theory of
  operations' section at the start of this document.

  This function returns immediately after initiating the transaction. The content
  of the read buffer passed as parameter should not be modified until the write
  transaction completes. It also means that the memory allocated for the read
  buffer should not be freed or should not go out of scope before the read
  completes.
  You can check for the write transaction completion by polling the master_status
  from miv_i2c_instance_t structure as shown in the sample code.

  @param this_i2c
                   A pointer to the miv_i2c_instance_t data structure which
                   will hold all the data related to the Mi-V I2C module 
				   instance being used. A pointer to this structure is passed to 
				   rest of the Mi-V I2C driver functions for operation.

  @param i2c_target_addr
                   This parameter specifies the serial address for the slave
                   device.

  @param read_buffer
                   This parameter is a pointer to the buffer where the data
                   received from the I2C slave device is stored.
                   Care must be taken not to release the memory used by this
                   buffer before the write transaction completes.

  @param read_size
                   Number of bytes held in the write_buffer to be read from the
                   I2C device.

  @param bus_options:
                   The bus_options parameter is used to indicate if the I2C bus
                   should be released on completion of the write transaction.
                   Using the MIV_I2C_RELEASE_BUS constant for the bus_options
                   parameter causes a STOP bit to be generated at the end of the
                   write transaction causing the bus to be released for other I2C
                   devices to use. Using the MIV_I2C_HOLD_BUS constant as
                   bus_options parameter prevents a STOP bit from being generated
                   at the end of the write transaction, preventing other I2C
                   devices from initiating a bus transaction.

  @param ack_polling_options:
                   The ack_polling_options parameter is used to indicate how the
                   MIV_I2C will respond if the slave device transmits a NACK to
                   the I2C control byte. Using the MIV_I2C_ACK_POLLING_DISABLE
                   constant for the ack_polling_options parameter causes the
                   MIV_I2C to abort the transfer if the slave device responds to
                   the I2C control byte with a NACK. Using the
                   MIV_I2C_ACK_POLLING_ENABLE constant for the ack_polling_options
                   parameter causes the MIV_I2C to repeatedly transmit a control
                   byte to the slave device until the slave device responds with
                   an ACK.
  @return
                   This function does not return any value.


  Example:
  @code
    #define MIV_I2C_BASE_ADDR  0x7A000000u

    miv_i2c_instance_t g_miv_i2c_inst;

    void main( void )
    {
        MIV_I2C_init( &g_miv_i2c_inst, MIV_I2C_BASE_ADDR);

        Configuring Mi-V I2C core at Normal Speed (100MHz) for 50MHz Sys clock.
        MIV_I2C_config(&g_miv_i2c_inst, 0x63);

        MIV_I2C_write (&g_miv_i2c_inst,
                       DUALEE_SLAVEADDRESS_1,
                       i2c_tx_buffer,
                       transfer_size,
                       MIV_I2C_RELEASE_BUS,
                       MIV_I2C_ACK_POLLING_ENABLE
                      );

        // Wait till the miv i2c status changes
        do {
            miv_i2c_status = miv_i2c.master_status;
        }while (MIV_I2C_IN_PROGRESS == miv_i2c_status);

        // reset miv_i2c_status variable
        miv_i2c_status = 0u;

        MIV_I2C_read (&g_miv_i2c_inst,
                       DUALEE_SLAVEADDRESS_1,
                       i2c_rx_buffer,
                       transfer_size,
                       MIV_I2C_RELEASE_BUS,
                       MIV_I2C_ACK_POLLING_ENABLE
                      );

        // Wait till the miv i2c status changes
        do {
            miv_i2c_status = miv_i2c.master_status;
        }while (MIV_I2C_IN_PROGRESS == miv_i2c_status);
    }
  @endcode
 */
void
MIV_I2C_read
(
    miv_i2c_instance_t *this_i2c,
    uint8_t i2c_target_addr,
    uint8_t *read_buffer,
    uint16_t read_size,
    uint8_t bus_options,
    uint8_t ack_polling_options
);

/*-------------------------------------------------------------------------*//**
  The MIV_I2C_write_read() is used to set up and start the Mi-V I2C master
  write_read transaction. This function is used for all MIV_I2C master write_read
  operation.

  This function is used in cases where data is being requested from a specific
  address offset inside the target I2C slave device.
  In this type of I2C operation, the I2C master starts by initiating a write
  operation. During this write operation, the specific address offset is written
  to the I2C slave. Once the address offset has been written to the I2C slave,
  the I2C master transmits a repeated start, and initiates a read operation to
  read data from the set address.

  For more information about the operation, please refer to the 'theory of
  operations' section at the start of this document.

  This function returns immediately after initiating the transaction. The content
  of the write and read buffer passed as parameter should not be modified until
  the write transaction completes. It also means that the memory allocated for
  the write and read buffer should not be freed or should not go out of scope
  before the operation completes.
  You can check for the write_read transaction completion by polling the
  master_status from miv_i2c_instance_t structure.

  @param this_i2c
                   A pointer to the miv_i2c_instance_t data structure which
                   will hold all the data related to the Mi-V I2C module 
				   instance being used. A pointer to this structure is passed to 
				   rest of the Mi-V I2C driver functions for operation.

  @param i2c_target_addr
                  This parameter specifies the serial address for the slave
                  device.

  @param write_buffer
                  This parameter is a pointer to the buffer holding data to be
                  written to target I2C device.
                  Care must be taken not to release the memory used by this
                  buffer before the write transaction completes.

  @param write_size
                  Number of bytes held in the write_buffer to be written to the
                  I2C device.

  @param read_buffer
                 This parameter is a pointer to the buffer where the data
                 received from the I2C slave device is stored.
                 Care must be taken not to release the memory used by this
                 buffer before the write transaction completes.

  @param read_size
                 Number of bytes held in the write_buffer to be read from the
                 I2C device.

  @param bus_options:
                 The bus_options parameter is used to indicate if the I2C bus
                 should be released on completion of the write transaction.
                 Using the MIV_I2C_RELEASE_BUS constant for the bus_options
                 parameter causes a STOP bit to be generated at the end of the
                 write transaction causing the bus to be released for other I2C
                 devices to use. Using the MIV_I2C_HOLD_BUS constant as
                 bus_options parameter prevents a STOP bit from being generated
                 at the end of the write transaction, preventing other I2C
                 devices from initiating a bus transaction.

  @param ack_polling_options:
                 The ack_polling_options parameter is used to indicate how the
                 MIV_I2C will respond if the slave device transmits a NACK to
                 the I2C control byte. Using the MIV_I2C_ACK_POLLING_DISABLE
                 constant for the ack_polling_options parameter causes the
                 MIV_I2C to abort the transfer if the slave device responds to
                 the I2C control byte with a NACK. Using the
                 MIV_I2C_ACK_POLLING_ENABLE constant for the ack_polling_options
                 parameter causes the MIV_I2C to repeatedly transmit a control
                 byte to the slave device until the slave device responds with
                 an ACK or the timeout specified in the MIV_I2C_wait_complete()
                 function is reached.
  @return
                 This function does not return any value.


  Example:
  @code
    #define MIV_I2C_BASE_ADDR  0x7A000000u

    miv_i2c_instance_t g_miv_i2c_inst;

    void main( void )
    {
        MIV_I2C_init( &g_miv_i2c_inst, MIV_I2C_BASE_ADDR);

        Configuring Mi-V I2C core at Normal Speed (100MHz) for 50MHz Sys clock.
        MIV_I2C_config(&g_miv_i2c_inst, 0x63);

        MIV_I2C_write (&g_miv_i2c_inst,
                       DUALEE_SLAVEADDRESS_1,
                       i2c_tx_buffer,
                       transfer_size,
                       MIV_I2C_RELEASE_BUS,
                       MIV_I2C_ACK_POLLING_ENABLE
                      );

        // Wait till the miv i2c status changes
        do {
            miv_i2c_status = miv_i2c.master_status;
        }while (MIV_I2C_IN_PROGRESS == miv_i2c_status);

        // reset miv_i2c_status variable
        miv_i2c_status = 0u;

    uint8_t addr_offset[2] = {0x00, 0x00};
    MIV_I2C_write_read(&miv_i2c,
                        DUALEE_SLAVEADDRESS_1,
                        addr_offset,
                        sizeof(addr_offset),
                        i2c_rx_buffer,
                        transfer_size,
                        MIV_I2C_RELEASE_BUS,
                        MIV_I2C_ACK_POLLING_ENABLE
                       );

        // Wait till the miv i2c status changes
        do {
            miv_i2c_status = miv_i2c.master_status;
        }while (MIV_I2C_IN_PROGRESS == miv_i2c_status);
    }
  @endcode
 */
void
MIV_I2C_write_read
(
    miv_i2c_instance_t *this_i2c,
    uint8_t target_addr,
    const uint8_t *write_buffer,
    uint16_t write_size,
    uint8_t *read_buffer,
    uint16_t read_size,
    uint8_t bus_options,
    uint8_t ack_polling_options
);

/*-------------------------------------------------------------------------*//**
  The MIV_I2C_isr() function contains the MIV_I2C's interrupt service routine.
  This ISR is at the heart of the MIV_I2C driver, and is used to control the
  interrupt-driven, byte-by-byte I2C read and write operations.

  The ISR operates as a Finite State Machine (FSM), which uses the previously
  completed I2C operation and its result to determine which I2C operation will
  be performed next.

  The ISR operation is divided into following categories:
      - MIV_I2C_IDLE
      - MIV_I2C_TX_STA_CB
      - MIV_I2C_TX_DATA
      - MIV_I2C_RX_DATA

  ##### MIV_I2C_IDLE
  The MIV_I2C_IDLE is entered on reset, or when an I2C master operation has been
  completed or aborted.
  Upon entering, the FSM will remain in this state until a write, read, or
  write-read operation is requested

  ##### MIV_I2C_STA_CB
  The MIV_I2C_TX_STA_CB operation is performed when the start condition and
  control byte(i2c target address(7-bit) and direction of transaction(1-bit)) is
  transmitted by the Mi-V I2C master device to the slave.
  If the target I2C slave device responded to the previous START Condition +
  Control Byte with an ACK, the MIV_I2C will start the requested I2C
  read/write operation.
  If the target slave I2C slave device responds with NACK, the MIV_I2C will
  remain in this state or return to the idle state based on ack_polling
  configuration.

  ##### MIV_I2C_TX_DATA
  The MIV_I2C_TX_DATA state is entered after the target slave device accepts a 
  write request with an ACK.
  This state is used to handle the byte-by-byte MIV_I2C write operations.
  The FSM will remain in this state until either all data bytes have been
  written to the target slave device, or an error occurs during the write
  operation.

  ##### MIV_I2C_RX_DATA
  The MIV_I2C_RX_DATA state is entered after the target slave device accepts a 
  read request with an ACK.
  This state is used to handle the byte-by-byte MIV_I2C read operations.
  The FSM will remain in this state until either all data bytes have been
  received from the target slave device, or an error occurs.

  @param this_i2c
                   A pointer to the miv_i2c_instance_t data structure which
                   will hold all the data related to the Mi-V I2C module 
				   instance being used. A pointer to this structure is passed to 
				   rest of the Mi-V I2C driver functions for operation.
 */
void
MIV_I2C_isr
(
    miv_i2c_instance_t *this_i2c
);

/*-------------------------------------------------------------------------*//**
  The MIV_I2C_get_status() returns the 8-bit Mi-V I2C status register value.

  @param this_i2c
                   A pointer to the miv_i2c_instance_t data structure which
                   will hold all the data related to the Mi-V I2C module 
				   instance being used. A pointer to this structure is passed to 
				   rest of the Mi-V I2C driver functions for operation.
  @return
                   This function returns 8-bit Mi-V I2C status register value.
 */
uint8_t
MIV_I2C_get_status
(
    miv_i2c_instance_t *this_i2c
);

#endif  /* MIV_I2C_H_ */
