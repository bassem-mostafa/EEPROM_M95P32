// #############################################################################
// #### Copyright ##############################################################
// #############################################################################

/*
 * Copyright 2024 BaSSeM
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

// #############################################################################
// #### Description ############################################################
// #############################################################################

// #############################################################################
// #### Control Include(s) #####################################################
// #############################################################################

#include "Platform.h"

// #############################################################################
// #### Control Macro(s) #######################################################
// #############################################################################

#ifndef DEBUG
    #define DEBUG
#endif

#ifdef DEBUG
    #undef DEBUG
#endif

// #############################################################################
// #### File Guard #############################################################
// #############################################################################

// #############################################################################
// #### Include(s) #############################################################
// #############################################################################

#include "../../EEPROM_Internal.h"
#include "EEPROM_M95P32.h"

// #############################################################################
// #### Private Macro(s) #######################################################
// #############################################################################

#define EEPROM_M95P32_BLOCK_COUNT          ( 64 )
#define EEPROM_M95P32_BLOCK_SIZE           ( 65536 )
#define EEPROM_M95P32_BLOCK_SECTOR_COUNT   ( 16 )

#define EEPROM_M95P32_SECTOR_COUNT         ( 1024 )
#define EEPROM_M95P32_SECTOR_SIZE          ( 4096 )
#define EEPROM_M95P32_SECTOR_PAGE_COUNT    ( 8 )

#define EEPROM_M95P32_PAGE_COUNT           ( 8192 )
#define EEPROM_M95P32_PAGE_SIZE            ( 512 )

#define EEPROM_M95P32_SIZE                 ( EEPROM_M95P32_PAGE_COUNT * EEPROM_M95P32_PAGE_SIZE )

#define EEPROM_M95P32_BUFFER_SIZE_TRANSMIT ( 16 )
#define EEPROM_M95P32_BUFFER_SIZE_RECEIVE  ( 16 )

// #############################################################################
// #### Private Type(s) ########################################################
// #############################################################################

typedef enum EEPROM_M95P32_Command
{
    EEPROM_M95P32_Command_WriteEnable = 0b00000110,           // 06
    EEPROM_M95P32_Command_WriteDisable = 0b00000100,          // 04
    EEPROM_M95P32_Command_StatusRead = 0b00000101,            // 05
    EEPROM_M95P32_Command_StatusWrite = 0b00000001,           // 01
    EEPROM_M95P32_Command_DataRead = 0b00000011,              // 03
    EEPROM_M95P32_Command_DataReadFast = 0b00001011,          // 0B
    EEPROM_M95P32_Command_DataReadFast_Dual = 0b00111011,     // 3B
    EEPROM_M95P32_Command_DataReadFast_Quad = 0b01101011,     // 6B
    EEPROM_M95P32_Command_PageWrite = 0b00000010,             // 02
    EEPROM_M95P32_Command_PageProgram = 0b00001010,           // 0A
    EEPROM_M95P32_Command_PageErase = 0b11011011,             // DB
    EEPROM_M95P32_Command_SectorErase = 0b00100000,           // 20
    EEPROM_M95P32_Command_BlockErase = 0b11011000,            // D8
    EEPROM_M95P32_Command_ChipErase = 0b11000111,             // C7
    EEPROM_M95P32_Command_IDRead = 0b10000011,                // 83
    EEPROM_M95P32_Command_IDReadFast = 0b10001011,            // 8B
    EEPROM_M95P32_Command_IDWrite = 0b10000010,               // 82
    EEPROM_M95P32_Command_PowerDownEnter = 0b10111001,        // B9
    EEPROM_M95P32_Command_PowerDownExit = 0b10101011,         // AB
    EEPROM_M95P32_Command_IDRead_JEDEC = 0b10011111,          // 9F
    EEPROM_M95P32_Command_CFGRead = 0b00010101,               // 15
    EEPROM_M95P32_Command_VolatileRegisterRead = 0b10000101,  // 85
    EEPROM_M95P32_Command_VolatileRegisterWrite = 0b10000001, // 81
    EEPROM_M95P32_Command_FlagsClear = 0b01010000,            // 50
    EEPROM_M95P32_Command_SFDPRead = 0b01011010,              // 5A
    EEPROM_M95P32_Command_ResetEnable = 0b01100110,           // 66
    EEPROM_M95P32_Command_ResetSoft = 0b10011001,             // 99
} EEPROM_M95P32_Command_t;

typedef union __attribute__( ( packed, aligned( 1 ) ) ) EEPROM_M95P32_StatusRegister
{
    uint8_t Value[ 1 ];

    struct
    {
        uint8_t WIP  : 1; ///< Write in progress (WIP) bit
        uint8_t WEL  : 1; ///< Write enable latch (WEL) bit
        uint8_t BP   : 3; ///< Block protection bits
        uint8_t      : 1; ///< Reserved
        uint8_t TB   : 1; ///< Top / bottom protection (TB) bit
        uint8_t SRWD : 1; ///< Status register write protect (SRWD) bit
    };
} EEPROM_M95P32_StatusRegister_t;

typedef union __attribute__( ( packed, aligned( 1 ) ) ) EEPROM_M95P32_ConfigurationRegister
{
    uint8_t Value[ 1 ];

    struct
    {
        uint8_t LID : 1; ///< lock ID bit (LID)
        uint8_t     : 4; ///< Reserved
        uint8_t DRV : 2; ///< Output driver strength
        uint8_t     : 1; ///< Reserved
    };
} EEPROM_M95P32_ConfigurationRegister_t;

typedef union __attribute__( ( packed, aligned( 1 ) ) ) EEPROM_M95P32_SafetyRegister
{
    uint8_t Value[ 1 ];

    // FIXME Check necessity ?
    struct
    {
        uint8_t ECC3DS : 1; ///< ECC 3 detection flag (ECC3DS) bit
        uint8_t ECC3D  : 1; ///< ECC 3 detection flag (ECC3D) bit
        uint8_t ECC2C  : 1; ///< ECC 2 correction flag (ECC2C) bit
        uint8_t ECC1C  : 1; ///< ECC 1 correction flag (ECC1C) bit
        uint8_t        : 4; ///< Reserved
    };

    struct
    {
        uint8_t ECC   : 4; ///< ECC (Error-Correcting Code) flag bits
        uint8_t PRF   : 1; ///< Program flag (PRF) bit
        uint8_t ERF   : 1; ///< Erase flag (ERF) bit
        uint8_t PUF   : 1; ///< Power-up flag (PUF) bit
        uint8_t PAMAF : 1; ///< Protected array modify attempt flag (PAMAF) bit
    };
} EEPROM_M95P32_SafetyRegister_t;

typedef union __attribute__( ( packed, aligned( 1 ) ) ) EEPROM_M95P32_VolatileRegister
{
    uint8_t Value[ 1 ];

    struct
    {
        uint8_t BUFLD : 1; ///< Buffer loading status (BUFLD) bit
        uint8_t BUFEN : 1; ///< Buffer loading activation (BUFEN) bit
        uint8_t       : 6; ///< Reserved
    };
} EEPROM_M95P32_VolatileRegister_t;

typedef struct __attribute__( ( packed, aligned( 1 ) ) ) EEPROM_M95P32_SerialFlashDiscoverableParameter
{
    uint8_t ID; ///< Parameter ID

    struct
    {
        uint8_t Minor; ///< Parameter Minor Revision
        uint8_t Major; ///< Parameter Major Revision
    } Version;         ///< Parameter Revision

    uint8_t length; //< Parameter Length in DWORDs (32-bit)

    union
    {
        uint32_t Pointer : 24; ///< Parameter Table Pointer (PTP)
        uint32_t         : 8;  ///< Reserved
    };
} EEPROM_M95P32_SerialFlashDiscoverableParameter_t;

typedef union __attribute__( ( packed, aligned( 1 ) ) ) EEPROM_M95P32_SerialFlashDiscoverableParameterRegister
{
    uint8_t Value[ 512 ];

    struct
    {
        uint8_t Signature[ 4 ]; ///< SFDP signature = 0x50444653 (ASCII "SFDP")

        struct
        {
            uint8_t Minor; ///< SFDP Minor Revision
            uint8_t Major; ///< SFDP Major Revision
        } Version;         ///< SFDP Revision

        uint8_t NumberOfParameters; ///< Number of Parameter Headers (NPH)
                                    ///< Specifies the number of parameter headers in the SFDP data structure.
                                    ///< This number is 0-based. Therefore, 0 indicates 1 parameter header.
        uint8_t AccessControl;      ///< ????
        EEPROM_M95P32_SerialFlashDiscoverableParameter_t Parameter[];
    };
} EEPROM_M95P32_SerialFlashDiscoverableParameterRegister_t;

// 6.4 JEDEC Flash Parameter Tables
//
// Parameter tables contain coded information describing the features and capabilities of the serial
// flash. The first parameter table as defined by JEDEC is mandatory and its starting address is
// specified by the PTP field of the 1st Parameter Header. The length of this table is nine
// DWORDs. This table identifies some of the basic features of flash memory devices.

// TODO Add structures of mandatory and other parameters

typedef EEPROM_M95P32_Status_t ( *EEPROM_M95P32_OperationHandler_t )( EEPROM_M95P32_Instance_t * Instance );

typedef enum EEPROM_M95P32_OperationType
{
    EEPROM_M95P32_OperationType_None = 0,

    // Preliminary Process Operation
    EEPROM_M95P32_OperationType_Pending,

    // Basic Operation(s)
    EEPROM_M95P32_OperationType_PowerOn,
    EEPROM_M95P32_OperationType_PowerOff,

    // General Operation(s)
    EEPROM_M95P32_OperationType_WriteEnable,
    EEPROM_M95P32_OperationType_WriteDisable,
    EEPROM_M95P32_OperationType_StatusRead,
    EEPROM_M95P32_OperationType_StatusWrite,
    EEPROM_M95P32_OperationType_DataRead,
    EEPROM_M95P32_OperationType_DataReadFast,
    EEPROM_M95P32_OperationType_DataReadFast_Dual,
    EEPROM_M95P32_OperationType_DataReadFast_Quad,
    EEPROM_M95P32_OperationType_PageWrite,
    EEPROM_M95P32_OperationType_PageProgram,
    EEPROM_M95P32_OperationType_PageErase,
    EEPROM_M95P32_OperationType_SectorErase,
    EEPROM_M95P32_OperationType_BlockErase,
    EEPROM_M95P32_OperationType_ChipErase,
    EEPROM_M95P32_OperationType_IDRead,
    EEPROM_M95P32_OperationType_IDReadFast,
    EEPROM_M95P32_OperationType_IDWrite,
    EEPROM_M95P32_OperationType_PowerDownEnter,
    EEPROM_M95P32_OperationType_PowerDownExit,
    EEPROM_M95P32_OperationType_IDRead_JEDEC,
    EEPROM_M95P32_OperationType_CFGRead,
    EEPROM_M95P32_OperationType_VolatileRegisterRead,
    EEPROM_M95P32_OperationType_VolatileRegisterWrite,
    EEPROM_M95P32_OperationType_FlagsClear,
    EEPROM_M95P32_OperationType_SFDPRead,
    EEPROM_M95P32_OperationType_ResetEnable,
    EEPROM_M95P32_OperationType_ResetSoft,
} EEPROM_M95P32_OperationType_t;

typedef struct EEPROM_M95P32_OperationContext
{
} EEPROM_M95P32_OperationContext_t;

typedef struct EEPROM_M95P32_Operation
{
    EEPROM_M95P32_OperationType_t Type;       ///< Type
    EEPROM_M95P32_OperationHandler_t Handler; ///< Handler
    EEPROM_M95P32_Status_t Status;            ///< Status
    TIM_Timestamp_t Timeout;                  ///< Timeout
    EEPROM_M95P32_OperationContext_t Context; ///< Context
} EEPROM_M95P32_Operation_t;

typedef EEPROM_M95P32_Status_t ( *EEPROM_M95P32_ProcessHandler_t )( EEPROM_M95P32_Instance_t * Instance );

typedef enum EEPROM_M95P32_ProcessType
{
    EEPROM_M95P32_ProcessType_None = 0, ///< None
    EEPROM_M95P32_ProcessType_Atomic,   ///< Atomic (Single Operation)

    EEPROM_M95P32_ProcessType_Initialize, ///< Initialize
} EEPROM_M95P32_ProcessType_t;

typedef struct EEPROM_M95P32_ProcessContext
{
    EEPROM_M95P32_Operation_t Operation; ///< Operation
} EEPROM_M95P32_ProcessContext_t;

typedef struct EEPROM_M95P32_Process
{
    EEPROM_M95P32_ProcessType_t Type;       ///< Type
    EEPROM_M95P32_ProcessHandler_t Handler; ///< Handler
    EEPROM_M95P32_ProcessContext_t Context; ///< Context
} EEPROM_M95P32_Process_t;

typedef enum EEPROM_M95P32_Event
{
    EEPROM_M95P32_Event_None = 0,
} EEPROM_M95P32_Event_t;

typedef struct EEPROM_M95P32_InstanceContext
{
    EEPROM_M95P32_Event_t Event;

    EEPROM_M95P32_Process_t Process;

    BUFFER_t Transmit;
    uint8_t TransmitContent[ EEPROM_M95P32_BUFFER_SIZE_TRANSMIT ];

    BUFFER_t Receive;
    uint8_t ReceiveContent[ EEPROM_M95P32_BUFFER_SIZE_RECEIVE ];
} EEPROM_M95P32_InstanceContext_t;

typedef struct EEPROM_M95P32_Context
{
    TIM_Timestamp_t Timestamp;
    EEPROM_M95P32_InstanceContext_t Context[ EEPROM_M95P32_Count ];
} EEPROM_M95P32_Context_t;

// #############################################################################
// #### Private Method(s) Prototype ############################################
// #############################################################################
//
static EEPROM_M95P32_Status_t EEPROM_M95P32_Context_Initialize( void );
static EEPROM_M95P32_Status_t EEPROM_M95P32_Context_Cycle( void );
static EEPROM_M95P32_Status_t EEPROM_M95P32_Context_DeInitialize( void );

static EEPROM_M95P32_Status_t EEPROM_M95P32_Instance_Initialize( EEPROM_M95P32_Instance_t * Instance );
static EEPROM_M95P32_Status_t EEPROM_M95P32_Instance_Cycle( EEPROM_M95P32_Instance_t * Instance );
static EEPROM_M95P32_Status_t EEPROM_M95P32_Instance_DeInitialize( EEPROM_M95P32_Instance_t * Instance );

static EEPROM_M95P32_Status_t EEPROM_M95P32_SetProcess( EEPROM_M95P32_Instance_t * Instance, EEPROM_M95P32_ProcessType_t ProcessType );

static EEPROM_M95P32_Status_t EEPROM_M95P32_Transfer( EEPROM_M95P32_Instance_t * Instance );

static EEPROM_M95P32_Status_t EEPROM_M95P32_ProcessInitialize( EEPROM_M95P32_Instance_t * Instance );

static EEPROM_M95P32_Status_t EEPROM_M95P32_OperationPowerOffExecute( EEPROM_M95P32_Instance_t * Instance );
static EEPROM_M95P32_Status_t EEPROM_M95P32_OperationPowerOffResolve( EEPROM_M95P32_Instance_t * Instance );

static EEPROM_M95P32_Status_t EEPROM_M95P32_OperationPowerOnExecute( EEPROM_M95P32_Instance_t * Instance );
static EEPROM_M95P32_Status_t EEPROM_M95P32_OperationPowerOnResolve( EEPROM_M95P32_Instance_t * Instance );

static EEPROM_M95P32_Status_t EEPROM_M95P32_OperationWriteEnableExecute( EEPROM_M95P32_Instance_t * Instance );
static EEPROM_M95P32_Status_t EEPROM_M95P32_OperationWriteEnableResolve( EEPROM_M95P32_Instance_t * Instance );

// #############################################################################
// #### Private Variable(s) ####################################################
// #############################################################################

static EEPROM_M95P32_Context_t EEPROM_M95P32_Context;

// #############################################################################
// #### Private Method(s) ######################################################
// #############################################################################

static EEPROM_M95P32_Status_t EEPROM_M95P32_Context_Initialize( void )
{
    EEPROM_M95P32_Status_t Status = EEPROM_M95P32_Status_Success;

    do
    {
        EEPROM_Trace( "%s( void )", __FUNCTION__ );
    }
    while ( 0 );

    return Status;
}

static EEPROM_M95P32_Status_t EEPROM_M95P32_Context_Cycle( void )
{
    EEPROM_M95P32_Status_t Status = EEPROM_M95P32_Status_Success;

    do
    {
        EEPROM_Trace( "%s( void )", __FUNCTION__ );

        TIM_Status_t TIM_Status = TIM_Status_Error;
        if ( ( TIM_Status = TIM_GetTimestamp( EEPROM_TIM, &EEPROM_M95P32_Context.Timestamp ) ) != TIM_Status_Success )
        {
            Status = EEPROM_M95P32_Status_Error;
            break;
        }
    }
    while ( 0 );

    return Status;
}

static EEPROM_M95P32_Status_t EEPROM_M95P32_Context_DeInitialize( void )
{
    EEPROM_M95P32_Status_t Status = EEPROM_M95P32_Status_Success;

    do
    {
        EEPROM_Trace( "%s( void )", __FUNCTION__ );
    }
    while ( 0 );

    return Status;
}

static EEPROM_M95P32_Status_t EEPROM_M95P32_Instance_Initialize( EEPROM_M95P32_Instance_t * Instance )
{
    EEPROM_M95P32_Status_t Status = EEPROM_M95P32_Status_Success;

    do
    {
        EEPROM_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );

        // TODO GPIOs Configuration

        EEPROM_M95P32_InstanceContext_t * Context = &EEPROM_M95P32_Context.Context[ Instance->M95P32 ];

        Context->Event = EEPROM_M95P32_Event_None;

        BUFFER_Status_t BUFFER_Status = BUFFER_Status_Error;
        if ( ( BUFFER_Status = BUFFER_Initialize( &Context->Transmit, Context->TransmitContent, UTIL_SizeOf( Context->TransmitContent ) ) ) != BUFFER_Status_Success )
        {
            Status = EEPROM_M95P32_Status_Error;
            break;
        }

        if ( ( BUFFER_Status = BUFFER_Initialize( &Context->Receive, Context->ReceiveContent, UTIL_SizeOf( Context->ReceiveContent ) ) ) != BUFFER_Status_Success )
        {
            Status = EEPROM_M95P32_Status_Error;
            break;
        }

        Instance->Context = Context;

        Status = EEPROM_M95P32_SetProcess( Instance, EEPROM_M95P32_ProcessType_Initialize );
    }
    while ( 0 );

    return Status;
}

static EEPROM_M95P32_Status_t EEPROM_M95P32_Instance_Cycle( EEPROM_M95P32_Instance_t * Instance )
{
    EEPROM_M95P32_Status_t Status = EEPROM_M95P32_Status_Success;

    do
    {
        EEPROM_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );

        EEPROM_M95P32_InstanceContext_t * Context = &EEPROM_M95P32_Context.Context[ Instance->M95P32 ];
        EEPROM_M95P32_Process_t * Process = &Context->Process;
        EEPROM_M95P32_Operation_t * Operation = &Process->Context.Operation;
        EEPROM_M95P32_Event_t Event = Context->Event; // CAUTION: Has to copy events occurred at the early start of the cycle, so as to be cleared at the end of the cycle,
                                                      //          which let events occurs after that for the next cycle call

        if ( Operation->Handler != NULL )
        {
            EEPROM_M95P32_Status_t EEPROM_M95P32_Status = EEPROM_M95P32_Status_Error;
            if ( ( EEPROM_M95P32_Status = Operation->Handler( Instance ) ) != EEPROM_M95P32_Status_Success )
            {
                Status = EEPROM_M95P32_Status;
                // FIXME Operation reported non success status, is there any action ?
            }
        }

        if ( Process->Handler != NULL )
        {
            EEPROM_M95P32_Status_t EEPROM_M95P32_Status = EEPROM_M95P32_Status_Error;
            if ( ( EEPROM_M95P32_Status = Process->Handler( Instance ) ) != EEPROM_M95P32_Status_Success )
            {
                Status = EEPROM_M95P32_Status;
                // FIXME Process reported non success status, is there any action ?
            }
        }
    }
    while ( 0 );

    return Status;
}

static EEPROM_M95P32_Status_t EEPROM_M95P32_Instance_DeInitialize( EEPROM_M95P32_Instance_t * Instance )
{
    EEPROM_M95P32_Status_t Status = EEPROM_M95P32_Status_Success;

    do
    {
        EEPROM_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );
    }
    while ( 0 );

    return Status;
}

static EEPROM_M95P32_Status_t EEPROM_M95P32_SetProcess( EEPROM_M95P32_Instance_t * Instance, EEPROM_M95P32_ProcessType_t ProcessType )
{
    EEPROM_M95P32_Status_t Status = EEPROM_M95P32_Status_Success;
    do
    {
        EEPROM_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );

        EEPROM_M95P32_InstanceContext_t * Context = &EEPROM_M95P32_Context.Context[ Instance->M95P32 ];
        EEPROM_M95P32_Process_t * Process = &Context->Process;
        EEPROM_M95P32_Operation_t * Operation = &Process->Context.Operation;

        Process->Type = ProcessType;

        switch ( ProcessType )
        {
            case EEPROM_M95P32_ProcessType_None:
                Process->Handler = NULL;
                break;

            case EEPROM_M95P32_ProcessType_Initialize:
                Process->Handler = EEPROM_M95P32_ProcessInitialize;
                break;

            default:
                EEPROM_Warning( "%s Not Handled Type %d", __FUNCTION__, ProcessType );
                Status = EEPROM_M95P32_Status_NotSupported;
                break;
        }
        if ( Status != EEPROM_M95P32_Status_Success )
        {
            break;
        }

        Process->Type = ProcessType;

        Operation->Handler = NULL;
        Operation->Status = EEPROM_M95P32_Status_Success;
        Operation->Timeout = EEPROM_M95P32_Context.Timestamp;

        switch ( ProcessType )
        {
            case EEPROM_M95P32_ProcessType_None:
                Operation->Type = EEPROM_M95P32_OperationType_None;
                break;

            default:
                Operation->Type = EEPROM_M95P32_OperationType_Pending;
                break;
        }
    }
    while ( 0 );
    return Status;
}

static EEPROM_M95P32_Status_t EEPROM_M95P32_Transfer( EEPROM_M95P32_Instance_t * Instance )
{
    EEPROM_M95P32_Status_t Status = EEPROM_M95P32_Status_Error;
    do
    {
        EEPROM_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );

        GPIO_Status_t GPIO_Status = GPIO_Status_Error;
        if ( ( GPIO_Status = GPIO_Write( Instance->ChipSelect, GPIO_Value_Low ) ) != GPIO_Status_Success )
        {
            Status = EEPROM_M95P32_Status_Error;
            break;
        }

        SPI_Status_t SPI_Status = SPI_Status_Error;
        if ( ( SPI_Status = SPI_Transaction( Instance->SPIx, Instance->Context->Transmit.Content, Instance->Context->Transmit.Length, Instance->Context->Receive.Content, Instance->Context->Receive.Length ) ) != SPI_Status_Success )
        {
            Status = EEPROM_M95P32_Status_Error;
            break;
        }

        if ( ( GPIO_Status = GPIO_Write( Instance->ChipSelect, GPIO_Value_High ) ) != GPIO_Status_Success )
        {
            Status = EEPROM_M95P32_Status_Error;
            break;
        }

        Status = EEPROM_M95P32_Status_Success;
    }
    while ( 0 );
    return Status;
}

static EEPROM_M95P32_Status_t EEPROM_M95P32_ProcessInitialize( EEPROM_M95P32_Instance_t * Instance )
{
    EEPROM_M95P32_Status_t Status = EEPROM_M95P32_Status_Success;

    do
    {
        EEPROM_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );

        EEPROM_M95P32_InstanceContext_t * Context = &EEPROM_M95P32_Context.Context[ Instance->M95P32 ];
        EEPROM_M95P32_Process_t * Process = &Context->Process;
        EEPROM_M95P32_Operation_t * Operation = &Process->Context.Operation;

        if ( Process->Type != EEPROM_M95P32_ProcessType_Initialize )
        {
            EEPROM_Error( "%s Got %d Expected %d", __FUNCTION__, Process->Type, EEPROM_M95P32_ProcessType_Initialize );
            Status = EEPROM_M95P32_Status_Error;
            break;
        }

        if ( Operation->Handler != NULL )
        {
            // Operation In-progress
            break;
        }

        if ( Operation->Status != EEPROM_M95P32_Status_Success )
        {
            Operation->Type = EEPROM_M95P32_OperationType_None;
        }

        switch ( Operation->Type )
        {
            case EEPROM_M95P32_OperationType_Pending:
                Operation->Status = EEPROM_M95P32_OperationPowerOffExecute( Instance );
                break;

            case EEPROM_M95P32_OperationType_PowerOff:
                Operation->Status = EEPROM_M95P32_OperationPowerOnExecute( Instance );
                break;

            case EEPROM_M95P32_OperationType_PowerOn:
            default:
                // TODO OnComplete Callback
                // if ( Instance->OnComplete != NULL )
                // {
                //     Instance->OnComplete( Instance, Operation->Status );
                // }

                Status = EEPROM_M95P32_SetProcess( Instance, EEPROM_M95P32_ProcessType_None );
                break;
        }
    }
    while ( 0 );

    return Status;
}

static EEPROM_M95P32_Status_t EEPROM_M95P32_OperationPowerOffExecute( EEPROM_M95P32_Instance_t * Instance )
{
}

static EEPROM_M95P32_Status_t EEPROM_M95P32_OperationPowerOffResolve( EEPROM_M95P32_Instance_t * Instance )
{
}

static EEPROM_M95P32_Status_t EEPROM_M95P32_OperationPowerOnExecute( EEPROM_M95P32_Instance_t * Instance )
{
}

static EEPROM_M95P32_Status_t EEPROM_M95P32_OperationPowerOnResolve( EEPROM_M95P32_Instance_t * Instance )
{
}

static EEPROM_M95P32_Status_t EEPROM_M95P32_OperationWriteEnableExecute( EEPROM_M95P32_Instance_t * Instance )
{
}

static EEPROM_M95P32_Status_t EEPROM_M95P32_OperationWriteEnableResolve( EEPROM_M95P32_Instance_t * Instance )
{
}

// #############################################################################
// #### Public Method(s) #######################################################
// #############################################################################

EEPROM_M95P32_Status_t EEPROM_M95P32_Initialize( EEPROM_M95P32_Instance_t * Instance )
{
    EEPROM_M95P32_Status_t Status = EEPROM_M95P32_Status_Success;

    do
    {
        EEPROM_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );

        if ( ( Status = EEPROM_M95P32_Context_Initialize( ) ) != EEPROM_M95P32_Status_Success )
        {
            break;
        }

        if ( ( Status = EEPROM_M95P32_Instance_Initialize( Instance ) ) != EEPROM_M95P32_Status_Success )
        {
            break;
        }
    }
    while ( 0 );

    return Status;
}

EEPROM_M95P32_Status_t EEPROM_M95P32_Cycle( EEPROM_M95P32_Instance_t * Instance )
{
    EEPROM_M95P32_Status_t Status = EEPROM_M95P32_Status_Success;

    do
    {
        EEPROM_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );

        if ( ( Status = EEPROM_M95P32_Context_Cycle( ) ) != EEPROM_M95P32_Status_Success )
        {
            break;
        }

        if ( ( Status = EEPROM_M95P32_Instance_Cycle( Instance ) ) != EEPROM_M95P32_Status_Success )
        {
            break;
        }
    }
    while ( 0 );

    return Status;
}

EEPROM_M95P32_Status_t EEPROM_M95P32_DeInitialize( EEPROM_M95P32_Instance_t * Instance )
{
    EEPROM_M95P32_Status_t Status = EEPROM_M95P32_Status_Success;

    do
    {
        EEPROM_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );

        if ( ( Status = EEPROM_M95P32_Instance_DeInitialize( Instance ) ) != EEPROM_M95P32_Status_Success )
        {
            break;
        }

        if ( ( Status = EEPROM_M95P32_Context_DeInitialize( ) ) != EEPROM_M95P32_Status_Success )
        {
            break;
        }
    }
    while ( 0 );

    return Status;
}

EEPROM_M95P32_Status_t EEPROM_M95P32_Write( EEPROM_M95P32_Instance_t * Instance, EEPROM_M95P32_Address_t Address, EEPROM_M95P32_Data_t * Data, EEPROM_M95P32_DataLength_t DataLength )
{
    EEPROM_M95P32_Status_t Status = EEPROM_M95P32_Status_NotSupported;

    do
    {
        EEPROM_Trace( "%s( Instance=%p, Address=%06X, Data=%p, DataLength=%d)", __FUNCTION__, Instance, Address, Data, DataLength );
    }
    while ( 0 );

    return Status;
}

EEPROM_M95P32_Status_t EEPROM_M95P32_Read( EEPROM_M95P32_Instance_t * Instance, EEPROM_M95P32_Address_t Address, EEPROM_M95P32_Data_t * Data, EEPROM_M95P32_DataLength_t DataLength )
{
    EEPROM_M95P32_Status_t Status = EEPROM_M95P32_Status_NotSupported;

    do
    {
        EEPROM_Trace( "%s( Instance=%p, Address=%06X, Data=%p, DataLength=%d)", __FUNCTION__, Instance, Address, Data, DataLength );
    }
    while ( 0 );

    return Status;
}

// #############################################################################
// #### Public Variable(s) #####################################################
// #############################################################################

// #############################################################################
// #### File Guard #############################################################
// #############################################################################

// #############################################################################
// #### END OF FILE ############################################################
// #############################################################################
