//  BBBPWMDevice.h
//  BBBPWMDevice
//
//  Created by Michael Brookes on 04/10/2015.
//  Copyright © 2015 Michael Brookes. All rights reserved.
//

#ifndef BBBPWMDevice_h
#define BBBPWMDevice_h

#define SLOTS_DIR                "/sys/devices/bone_capemgr.9/slots" //!< Full system path to SLOTS, used to export device tree overlays.
#define DEVICE_DIR                "/sys/devices/ocp.3/" //!< Path to exported PWM overlay file systems
#define MODALIAS_FILE            "/sys/devices/ocp.3/48300000.epwmss/modalias" //!< This file should exist after the am33xx device overlay is exported.
#define PWM_PREP_OVERLAY_FILE    "am33xx_pwm" //!< This device tree must be exported before any specific pins
#define PWM_OVERLAY_FILE        "pwm_test_" //!< Begining of device tree overlay name
#define MAX_BUF                1024 //!< Used in setting the buffer size.
#define RETRIES                100 //!< PWM system files have an index appended to the end of the folder name. normally 1 - 99, RETRIES is used to find that index.
#define MAX_DUTY               150000
#define MIN_DUTY               700000

#include <iostream>
#include <exception>
#include <pthread.h>
#include <sstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

using namespace std;

/*!
 *  \brief     BBBPWMDevice provides low level access to the PWM files on the BeagleBone Black.
 *  \author    Michael Brookes
 *  \version   1.1
 *  \date      Oct-2015
 *  \copyright GNU Public License.
 */
class BBBPWMDevice {

public:

    pthread_t PWM_Thread;
    int PWM_TargetSpeed;
    /**
     \brief BlockNum refers to the block of exposed pins on your BBB, it will always be 8 or 9.
     */
    enum PWM_BlockNum {
        P9 = 9, //!< P9 on your BBB | USB facing you, P9 is the pin block on the left hand side.
        P8 = 8, //!< P8 on your BBB | USB facing you, P8 is the pin block on the right hand side.
    };

    /**
     \brief PinNum refers to a exposed PWM Pin on your BBB, this enum can be expanded to include more if required.
     */
    enum PWM_PinNum {
        PWM42 = 42, //!< GPIO PWM Pin Number 42
        PWM22 = 22, //!< GPIO PWM Pin Number 22
        PWM19 = 19, //!< GPIO PWM Pin Number 19
        PWM14 = 14, //!< GPIO PWM Pin Number 14
    };

    /**
     \brief PWM_RunVal - the two settings available for the Run Value of the motor.
     */
    enum PWM_RunValues {
        ON = 1,
        OFF = 0,
    };

    /**
     \brief PWM_PeriodVal - the two settings available for the Period Value of the motor.
     */
    enum PWM_PeriodValues {
        STARTUP = 1200000,
        ACTIVE = 1900000,
        INACTIVE = 0,
    };

    /**
     \fn public function int PWM_Init( )
     \brief Setup the PWM device.
     \param <void>
     \return 1 setup successful, 0 fail.
     */
    int PWM_Init( );

    /**
     \fn public function int PWM_SetRunVal( <PWM_RunValues> PWM_RunVal )
     \brief Store and write a new PWM Run Value
     \param <int> PWM_RunVal (0 or 1)
     \throws Exception on failure to write.
     \return <int> 0 Exception, > 0 success.
     */
    int PWM_SetRunVal( PWM_RunValues PWM_RunVal );

    /**
     \fn public function int PWM_SetPeriodVal( <PWM_PeriodValues> PWM_PeriodVal )
     \brief Store and write a new PWM Period Value
     \param <int> PWM_PeriodVal
     \throws Exception on failure to write.
     \return <int> 0 Exception, > 0 success.
     */
    int PWM_SetPeriodVal( PWM_PeriodValues PWM_PeriodVal );

    /**
     \fn public function void PWM_SetPinNum( PWM_BlockNum BlockNum )
     \brief Sets the Pin Number for this device. (Using the PinNum and BlockNum will tell you where this device is or should be plugged in)
     \param <void>
     \return <int> 1 success
     */
    void PWM_SetPinNum( PWM_PinNum PinNum );

    /**
     \fn public function void PWM_SetBlockNum( void )
     \brief Sets the Block Number for this PWM device. (Using the PinNum and BlockNum will tell you where this device is or should be plugged in)
     \param <void>
     \return <int> 1 success
     */
    void PWM_SetBlockNum( PWM_BlockNum BlockNum );

    /**
     \fn public function int PWM_GetPinNum( void ) const
     \brief Returns the Pin Number for this device. (Using the PinNum and BlockNum will tell you where this device is or should be plugged in)
     \param <void>
     \return <int> this->PinNum
     */
    int PWM_GetPinNum( void ) const;

    /**
     \fn public function int PWM_GetBlockNum( void ) const
     \brief Returns the Block Number for this PWM device. (Using the PinNum and BlockNum will tell you where this device is or should be plugged in)
     \param <void>
     \return <int> this->BlockNum
     */
    int PWM_GetBlockNum( void ) const;

    /**
     \fn public function int PWM_GetDutyVal( void ) const
     \brief Returns the DutyVal for this PWM Device
     \param <void>
     \return <int> this->PWM_DutyVal
     */
    int PWM_GetDutyVal( void ) const;

    /**
     \fn public function int PWM_GetPeriodVal( void ) const
     \brief Returns the PeriodVal for this PWM Device
     \param <void>
     \return <int> this->PWM_PeriodVal
     */
    int PWM_GetPeriodVal( void ) const;

    /**
     \fn public function int PWM_GetRunVal( void ) const
     \brief Returns the RunVal for this PWM Device
     \param <void>
     \return <int> this->PWM_RunVal
     */
    int PWM_GetRunVal( void ) const;

    /**
     \fn public function void PWM_SetTargetSpeed( int TargetSpeed )
     \brief Returns the RunVal for this PWM Device
     \param <int> TargetSpeed
     \return <int> this->PWM_RunVal
     */
    void PWM_SetTargetSpeed( int TargetSpeed );

    /**
     \brief BBBAnalogDevice : A low level control of PWM devices on the Beaglebone Black.
     \param <void>
     */
    BBBPWMDevice( );

protected:

    int PWM_RunVal; //!< Stores the PWM Devices Run Value
    int PWM_DutyVal; //!< Stores the PWM Devices Duty Value
    int PWM_PeriodVal; //!< Stores the PWM Devices Period Value
    int PWM_FileHandle; //!< Stores the PWM Devices File Handle
    int PWM_Ret; //!< Stores the thread created value.

    PWM_PinNum PinNum; //!< <PWM_PinNum> enum for Pin Number
    PWM_BlockNum BlockNum; //!< <PWM_BlockNum> enum for Block Number

    FILE *PWM_ReadFile; //!< File object used for reading values

    struct stat sb; //!< Used to discover if a folder or file exists already.

    string period_file_loc; //!< Stores the PWM Devices Period File Location
    string duty_file_loc; //!< Stores the PWM Devices Duty File Location
    string run_file_loc; //!< Stores the PWM Devices Run File Location

    char *PWM_ReadBuffer; //!< Used to read to files.
    char PWM_Buffer[MAX_BUF]; //!< Used to write to files.
    char PWM_PinOverlayFileName[MAX_BUF]; //!< Stores the PWM File name
    char PWM_PinOverlayFolderName[MAX_BUF]; //!< Stores the PWM folder name
    char PWM_PinOverlay[MAX_BUF]; //!< Stores the PWM device tree overlay name.

    /**
     \fn private function void* PWM_SetDutyVal( void *pwm_inst )
     \brief Store and write a new PWM Duty Value
     \param <void>
     \throws Exception on failure to write.
     \return <int> 0 Exception, > 0 success.
     */
    static void *PWM_SetDutyVal( void *pwm_inst );

    /**
     \fn private function int PWM_SysCheck( void )
     \brief Checks that the system files are available for PWM operation on the BeagleBone Black.
     \param <void>
     \return <int> 0 failure load the system files, 1 success.
     */
    int PWM_SysCheck( void );

    /**
     \fn private function int PWM_StartThread( void )
     \brief Starts a thread to modify the speed of the PWM Device
     \param <void>
     \return <void>
     */
    void PWM_StartThread( void );

    /**
     \fn private function int PWM_PinCheck( void )
     \brief Checks that the PWM Pin files are available for PWM operation on the BeagleBone Black.
     \param <void>
     \return <int> 99 failure load the PWM Pin files, < 99 success.
     */
    int PWM_PinCheck( void );

    /**
     \fn private function int PWM_LoadOverlay( const char* PWM_OverlayFile )
     \brief The BeagleBone Black has Overlay Files to allow operations like PWM. In this function we are attempting to export an overlay for PWM.
     \param const <char>* PWM_OverlayFile (just the file name, no path required)
     \return <int> -1 failure load the system files, 1 success.
     */
    int PWM_LoadOverlay( const char *PWM_OverlayFile );

    /**
     \fn private function int PWM_SetFileHandle( const char* PWM_FileName )
     \brief Creates a private FileHandle for use within this class.
     \param const <char>* PWM_FileName
     \throws Exception on failure to open the file.
     \return <int> -1 failure load the system files, > 0 success.
     */
    int PWM_SetFileHandle( const char *PWM_FileName );

    /**
     \fn private function int PWM_LoadPWMDefaultValues( void )
     \brief Load and store the file paths and the current values from the PWM, period, duty and run. Just so that they are set on program startup.
     \param <void>
     \return <int> -1 failure load the default values, 1 success.
     */
    int PWM_LoadPWMDefaultValues( void );

    /**
     \fn private function int PWM_WriteToFile( const char* PWM_Buffer, int PWM_BufferLen )
     \brief Writes a value to a file.
     \param const <char>* PWM_Buffer (holds the value to be written)
     \param <int>PWM_BufferLen (length of the buffer to be written)
     \throws Exception on failure to write.
     \return <int> -1 exception occured, 0 failed to write, 1 success.
     */
    int PWM_WriteToFile( const char *PWM_Buffer, int PWM_BufferLen );

    /**
     \fn private function int PWM_ReadFromFile( string FH_Name )
     \brief Reads a value from a file.
     \param <string> FH_Name
     \return <int> 0 failed to read, > 0 success.
     */
    char *PWM_ReadFromFile( string FH_Name );

    /**
     \fn private function int PWM_SetPWMFilePaths( void )
     \brief Load and store the file paths for use in this class.
     \param <void>
     \return <void>
     */
    void PWM_SetPWMFilePaths( void );
};

#endif /* BBBAnalogDevice_h */
