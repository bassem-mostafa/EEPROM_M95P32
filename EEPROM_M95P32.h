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

/**
 *  @file
 *
 *  @brief Platform EEPROM M95P32 Driver
 */

// #############################################################################
// #### Control Include(s) #####################################################
// #############################################################################

// #############################################################################
// #### Control Macro(s) #######################################################
// #############################################################################

// #############################################################################
// #### File Guard #############################################################
// #############################################################################

/**
 *  @addtogroup Platform_EEPROM_Driver
 *
 *  @{
 */

/**
 *  @defgroup Platform_EEPROM_M95P32 M95P32
 *
 *  @{
 */

#ifndef EEPROM_M95P32_H_
    #define EEPROM_M95P32_H_

    #ifdef __cplusplus
extern "C"
{
    #endif /* __cplusplus */

    // #############################################################################
    // #### Include(s) #############################################################
    // #############################################################################

    // #############################################################################
    // #### Public Macro(s) ########################################################
    // #############################################################################

    // #############################################################################
    // #### Public Type(s) #########################################################
    // #############################################################################

    /**
     *  @brief EEPROM M95P32 Operation Status
     *
     *  @enum EEPROM_M95P32_Status_t
     */
    typedef enum EEPROM_M95P32_Status
    {
        EEPROM_M95P32_Status_Success = 0,     ///< Success
        EEPROM_M95P32_Status_ArgumentInvalid, ///< Invalid Argument
        EEPROM_M95P32_Status_NotSupported,    ///< Not Supported
        EEPROM_M95P32_Status_Error,           ///< Generic Error
        EEPROM_M95P32_Status_Busy,            ///< Busy
        EEPROM_M95P32_Status_Timeout,         ///< Timeout
    } EEPROM_M95P32_Status_t;

    /**
     *  @brief EEPROM M95P32
     *
     *  @note Add as many as required
     *
     *  @enum EEPROM_M95P32_t
     */
    typedef enum EEPROM_M95P32
    {
        EEPROM_M95P32_1 = 0, ///< EEPROM (M95P32) 1
        EEPROM_M95P32_Count, ///< Count
    } EEPROM_M95P32_t;

    /**
     *  @brief EEPROM M95P32 Address
     */
    typedef uint32_t EEPROM_M95P32_Address_t;

    /**
     *  @brief EEPROM M95P32 Data
     */
    typedef uint8_t EEPROM_M95P32_Data_t;

    /**
     *  @brief EEPROM M95P32 Data Length
     */
    typedef uint32_t EEPROM_M95P32_DataLength_t;

    /**
     *  @brief EEPROM M95P32 Interface
     *
     *  @struct EEPROM_M95P32_Interface_t
     */
    typedef struct EEPROM_M95P32_Interface
    {
        SPI_t SPIx;         ///<
        GPIO_t ChipSelect;  ///<
        GPIO_t PowerEnable; ///<
    } EEPROM_M95P32_Interface_t;

    // #############################################################################
    // #### Public Method(s) #######################################################
    // #############################################################################

    /**
     *  @brief Binds Instance of EEPROM M95P32 to interface
     *
     *  @param[in] M95P32x   M95P32 Instance
     *  @param[in] Interface Connection Interface
     *
     *  @return EEPROM_M95P32_Status_t
     */
    EEPROM_M95P32_Status_t EEPROM_M95P32_Bind( EEPROM_M95P32_t M95P32x, EEPROM_M95P32_Interface_t Interface );

    /**
     *  @brief Initializes Instance of EEPROM M95P32
     *
     *  @param[in] M95P32x M95P32 Instance
     *
     *  @return EEPROM_M95P32_Status_t
     */
    EEPROM_M95P32_Status_t EEPROM_M95P32_Initialize( EEPROM_M95P32_t M95P32x );

    /**
     *  @brief Cycles Instance of EEPROM M95P32
     *
     *  @param[in] M95P32x M95P32 Instance
     *
     *  @return EEPROM_M95P32_Status_t
     */
    EEPROM_M95P32_Status_t EEPROM_M95P32_Cycle( EEPROM_M95P32_t M95P32x );

    /**
     *  @brief De-initializes Instance of EEPROM M95P32
     *
     *  @param[in] M95P32x M95P32 Instance
     *
     *  @return EEPROM_M95P32_Status_t
     */
    EEPROM_M95P32_Status_t EEPROM_M95P32_DeInitialize( EEPROM_M95P32_t M95P32x );

    /**
     *  @brief Writes data starting from address
     *
     *  @param[in] M95P32x    M95P32 Instance
     *  @param[in] Address    Starting address
     *  @param[in] Data       Data pointer
     *  @param[in] DataLength Length of data
     *
     *  @return EEPROM_M95P32_Status_t
     */
    EEPROM_M95P32_Status_t EEPROM_M95P32_Write( EEPROM_M95P32_t M95P32x, EEPROM_M95P32_Address_t Address, EEPROM_M95P32_Data_t * Data, EEPROM_M95P32_DataLength_t DataLength );

    /**
     *  @brief Reads data starting from address
     *
     *  @param[in] M95P32x    M95P32 Instance
     *  @param[in] Address    Starting address
     *  @param[in] Data       Data pointer
     *  @param[in] DataLength Length of data
     *
     *  @return EEPROM_M95P32_Status_t
     */
    EEPROM_M95P32_Status_t EEPROM_M95P32_Read( EEPROM_M95P32_t M95P32x, EEPROM_M95P32_Address_t Address, EEPROM_M95P32_Data_t * Data, EEPROM_M95P32_DataLength_t DataLength );

    // #############################################################################
    // #### Public Variable(s) #####################################################
    // #############################################################################

    // #############################################################################
    // #### File Guard #############################################################
    // #############################################################################

    #ifdef __cplusplus
} /* extern "C" */
    #endif /* __cplusplus */

#endif /* EEPROM_M95P32_H_ */

/**
 *  @}
 *
 *  @}
 */

// #############################################################################
// #### END OF FILE ############################################################
// #############################################################################
