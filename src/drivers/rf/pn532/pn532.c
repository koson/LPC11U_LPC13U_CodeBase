/**************************************************************************/
/*!
    @file     pn532.c
*/
/**************************************************************************/
#include "projectconfig.h"

#ifdef CFG_PN532

#include <string.h>

#include "pn532.h"
#include "pn532_bus.h"
#include "core/systick/systick.h"
#include "core/uart/uart.h"

static pn532_pcb_t _pn532_pcb;

/**************************************************************************/
/*!
    @brief  Prints a hexadecimal value in plain characters

    @param  pbtData   Pointer to the byte data
    @param  szBytes   Data length in bytes
*/
/**************************************************************************/
void pn532PrintHex(const byte_t * pbtData, const size_t szBytes)
{
  size_t szPos;
  for (szPos=0; szPos < szBytes; szPos++)
  {
    printf("%02x ", pbtData[szPos]);
  }
  printf(CFG_PRINTF_NEWLINE);
}

/**************************************************************************/
/*!
    @brief  Prints a hexadecimal value in plain characters, along with
            the char equivalents in the following format

            AA BB CC DD EE FF  ......

    @param  pbtData   Pointer to the byte data
    @param  szBytes   Data length in bytes
*/
/**************************************************************************/
void pn532PrintHexChar(const byte_t * pbtData, const size_t szBytes)
{
  size_t szPos;
  for (szPos=0; szPos < szBytes; szPos++)
  {
    printf("%02x", pbtData[szPos]);
  }
  printf("  ");
  for (szPos=0; szPos < szBytes; szPos++)
  {
    printf("%c", pbtData[szPos] <= 0x1F ? '.' : pbtData[szPos]);
  }
  printf(CFG_PRINTF_NEWLINE);
}

/**************************************************************************/
/*!
    @brief      Gets a reference to the PN532 peripheral control block,
                which can be used to determine that state of the PN532
                IC, buffers, etc.
*/
/**************************************************************************/
pn532_pcb_t * pn532GetPCB()
{
  return &_pn532_pcb;
}

/**************************************************************************/
/*!
    @brief      Initialises the appropriate serial bus (UART, etc.),and
                sets up any buffers or peripherals required by the PN532.
*/
/**************************************************************************/
error_t pn532Init(void)
{
  error_t error;

  // Clear protocol control blocks
  memset(&_pn532_pcb, 0, sizeof(pn532_pcb_t));

  // Initialise the underlying HW
  error = pn532_bus_HWInit();
  if (error)
  {
    // Something failed during init ... report the error
    return error;
  }
  else
  {
    // Set the PCB flags to an appropriate state
    _pn532_pcb.initialised = TRUE;
    // Let the caller know that initialised succeeded
    return ERROR_NONE;
  }
}

/**************************************************************************/
/*!
    @brief      Reads the response buffer from the PN532

    @param      pbtResponse
                The byte array that will hold the response data
    @param      pszLen
                Placeholder to return the actual number of bytes read
*/
/**************************************************************************/
pn532_error_t pn532Read(byte_t * pbtResponse, size_t * pszLen)
{
  if (!_pn532_pcb.initialised)
  {
    pn532Init();
  }

  // Try to wake the device up if it's in sleep mode
  if (_pn532_pcb.state == PN532_STATE_SLEEP)
  {
    pn532_error_t wakeupError = pn532_bus_Wakeup();
    if (wakeupError)
      return wakeupError;
  }

  // Read the response if the device is in an appropriate state
  if (_pn532_pcb.state == PN532_STATE_READY)
  {
    return pn532_bus_ReadResponse(pbtResponse, pszLen);
  }
  else
  {
    #ifdef PN532_DEBUGMODE
    PN532_DEBUG("Init Failed%s", CFG_PRINTF_NEWLINE);
    #endif
    return PN532_ERROR_UNABLETOINIT;
  }
}

/**************************************************************************/
/*!
    @brief      Sends a byte array of command and parameter data to the
                PN532, starting with the command byte.  The frame's
                preamble, checksums, postamble and frame identifier (0xD4)
                will all be automatically added.

    @param      abtCommand
                The byte array containg the command and any
                optional parameters
    @param      szLen
                The number of bytes in abtCommand
*/
/**************************************************************************/
pn532_error_t pn532Write(byte_t * abtCommand, size_t szLen)
{
  if (!_pn532_pcb.initialised)
  {
    pn532Init();
  }

  // Try to wake the device up if it's in sleep mode
  if (_pn532_pcb.state == PN532_STATE_SLEEP)
  {
    pn532_error_t wakeupError = pn532_bus_Wakeup();
    if (wakeupError)
      return wakeupError;
  }

  // Send the command if the device is in an appropriate state
  if (_pn532_pcb.state == PN532_STATE_READY)
  {
    return pn532_bus_SendCommand(abtCommand, szLen);
  }
  else
  {
    #ifdef PN532_DEBUGMODE
    PN532_DEBUG("Init Failed%s", CFG_PRINTF_NEWLINE);
    #endif
    return PN532_ERROR_UNABLETOINIT;
  }
}

#endif  // #ifdef CFG_PN532
