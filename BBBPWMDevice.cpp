//
//  BBBPWMDevice.cpp
//  BBBPWMDevice
//
//  Created by Michael Brookes on 04/10/2015.
//  Copyright © 2015 Michael Brookes. All rights reserved.
//

#include "BBBPWMDevice.h"

/**
 \brief BBBAnalogDevice : A low level control of PWM devices on the Beaglebone Black.
 - Assigns the device block number (8 or 9)
 - Assigns the device pin number. (Please review the BeagleBone Black's data sheet to make sure your device is connected properly).
 \param <PWM_BlockNum> BlockNum
 \param <PWM_PinNum> PinNum
 */
BBBPWMDevice::BBBPWMDevice( ) {

}

/**
 \fn private function int PWM_SysCheck( void )
 \brief Checks that the system files are available for PWM operation on the BeagleBone Black.
 \param <void>
 \return <int> 0 failure load the system files, 1 success.
 */
int BBBPWMDevice::PWM_SysCheck( void ) {
    if ( stat( MODALIAS_FILE, &sb ) == 0 && S_ISREG( sb.st_mode ) )
        return 1;
    else {
        if( this->PWM_LoadOverlay( PWM_PREP_OVERLAY_FILE ) < 0 )
            return 0;
        else
            return 1;
    }

}

/**
 \fn private function int PWM_PinCheck( void )
 \brief Checks that the PWM Pin files are available for PWM operation on the BeagleBone Black.
 \param <void>
 \return <int> 99 failure load the PWM Pin files, < 99 success.
 */
int BBBPWMDevice::PWM_PinCheck( void ) {
    int i = 0;
    while( i < RETRIES ) {
        i++;
        snprintf( this->PWM_PinOverlayFolderName, sizeof( this->PWM_PinOverlayFolderName ), "%spwm_test_P%d_%d.%d", DEVICE_DIR, this->BlockNum, this->PinNum, i );
        if ( stat( this->PWM_PinOverlayFolderName, &sb ) == 0 && S_ISDIR( sb.st_mode ) )
            break;
    }
    return i;
}

/**
 \fn private function int PWM_LoadOverlay( const char* PWM_OverlayFile )
 \brief The BeagleBone Black has Overlay Files to allow operations like PWM. In this function we are attempting to export an overlay for PWM.
 \param const <char>* PWM_OverlayFile (just the file name, no path required)
 \return <int> -1 failure load the system files, 1 success.
 */
int BBBPWMDevice::PWM_LoadOverlay( const char* PWM_OverlayFile ) {
    if( this->PWM_SetFileHandle( SLOTS_DIR ) < 0 )
        return this->PWM_FileHandle;
    else {
        if( this->PWM_WriteToFile( this->PWM_Buffer, snprintf( this->PWM_Buffer, sizeof( this->PWM_Buffer ), "%s", PWM_OverlayFile ) ) > 0 ) {
            close( this->PWM_FileHandle );
            return 1;
        }
        else
            return -1;
    }
}

/**
 \fn private function int PWM_SetFileHandle( const char* PWM_FileName )
 \brief Creates a private FileHandle for use within this class.
 \param const <char>* PWM_FileName
 \throws Exception on failure to open the file.
 \return <int> -1 failure load the system files, > 0 success.
 */
int BBBPWMDevice::PWM_SetFileHandle( const char* PWM_FileName ) {
    try {
        this->PWM_FileHandle = open( PWM_FileName, O_WRONLY );
    } catch( exception& e ) {
        cerr << "Error opening file : " << PWM_FileName << " | Error = " << e.what( ) << endl;
        this->PWM_FileHandle = -1;
    }
    return this->PWM_FileHandle;
}

/**
 \fn private function int PWM_LoadPWMDefaultValues( void )
 \brief Load and store the file paths and the current values from the PWM, period, duty and run. Just so that they are set on program startup.
 \param <void>
 \return <int> -1 failure load the default values, 1 success.
 */
int BBBPWMDevice::PWM_LoadPWMDefaultValues( void ) {
    this->PWM_SetPWMFilePaths( );

    this->PWM_DutyVal = ( int ) this->PWM_ReadFromFile( this->duty_file_loc );
    this->PWM_SetTargetSpeed( this->PWM_DutyVal );

    if( this->PWM_GetDutyVal( ) == 0 )
        return -1;

    int CurrentPeriodVal = ( int ) this->PWM_ReadFromFile( this->period_file_loc );
    this->PWM_SetPeriodVal( ( PWM_PeriodValues ) CurrentPeriodVal );
    if( this->PWM_GetPeriodVal( ) == 0 )
        return -1;

    int CurrentRunVal = ( int ) this->PWM_ReadFromFile( this->run_file_loc );
    this->PWM_SetRunVal( ( PWM_RunValues ) CurrentRunVal );
    if( this->PWM_GetRunVal( ) < 0 )
        return -1;

    return 1;
}

/**
 \fn private function int PWM_WriteToFile( const char* PWM_Buffer, int PWM_BufferLen )
 \brief Writes a value to a file.
 \param const <char>* PWM_Buffer (holds the value to be written)
 \param <int>PWM_BufferLen (length of the buffer to be written)
 \throws Exception on failure to write.
 \return <int> -1 exception occured, 0 failed to write, 1 success.
 */
int BBBPWMDevice::PWM_WriteToFile( const char* PWM_Buffer, int PWM_BufferLen ) {
    try {
        if( write( this->PWM_FileHandle, PWM_Buffer, PWM_BufferLen ) > 0 ) {
            close( this->PWM_FileHandle );
            return 1;
        }
        else
            return 0;
    } catch( exception& e ) {
        cerr << "Error writing to file. | Error = " << e.what( ) << endl;
        return -1;
    }
}

/**
 \fn private function int PWM_ReadFromFile( string FH_Name )
 \brief Reads a value from a file.
 \param <string> FH_Name
 \return <int> 0 failed to read, > 0 success.
 */
char * BBBPWMDevice::PWM_ReadFromFile( string FH_Name ) {
    this->PWM_ReadBuffer = 0;
    this->PWM_ReadFile = fopen( FH_Name.c_str( ), "rb" );
    if ( this->PWM_ReadFile == NULL )
        cerr << "Unable to read from file : " << FH_Name << endl;
    else{
        fseek( this->PWM_ReadFile, 0, SEEK_END );
        long lSize = ftell( this->PWM_ReadFile );
        rewind( this->PWM_ReadFile );
        this->PWM_ReadBuffer = ( char* ) malloc( sizeof( char )*lSize );
        if( this->PWM_ReadBuffer == NULL )
            cerr << "Memory Error" << endl;
        else {
            fread( this->PWM_ReadBuffer, 1, lSize, this->PWM_ReadFile );
            fclose( this->PWM_ReadFile );
        }
    }
    return this->PWM_ReadBuffer;
}

/**
 \fn public function void PWM_StartThread( void )
 \brief Start athread to handle the speed settings for this PWM device
 \param <void>
 \return <void>
 */
void BBBPWMDevice::PWM_StartThread( ) {
    this->PWM_Ret = pthread_create( &this->PWM_Thread, NULL, BBBPWMDevice::PWM_SetDutyVal, this );
    if( PWM_Ret ) {
        cerr << "Error - pthread_create() returned code: " << PWM_Ret << endl;
        exit( 1 );
    }
}

/**
 \fn private function int PWM_SetPWMFilePaths( void )
 \brief Load and store the file paths for use in this class.
 \param <void>
 \return <void>
 */
void BBBPWMDevice::PWM_SetPWMFilePaths( void ) {
    this->period_file_loc = this->PWM_PinOverlayFolderName;
    this->period_file_loc.append( "/period" );
    this->duty_file_loc = this->PWM_PinOverlayFolderName;
    this->duty_file_loc.append( "/duty" );
    this->run_file_loc = this->PWM_PinOverlayFolderName;
    this->run_file_loc.append( "/run" );
}

/**
 \fn public function int PWM_SetPeriodVal( int PWM_PeriodVal )
 \brief Store and write a new PWM Period Value
 \param <int> PWM_PeriodVal
 \throws Exception on failure to write.
 \return <int> 0 Exception, > 0 success.
 */
int BBBPWMDevice::PWM_SetPeriodVal( PWM_PeriodValues PWM_PeriodVal ) {
    try {
        this->PWM_PeriodVal = PWM_PeriodVal;
        this->PWM_SetFileHandle( this->period_file_loc.c_str( ) );
        return this->PWM_WriteToFile( this->PWM_Buffer, snprintf( this->PWM_Buffer, sizeof( this->PWM_Buffer ), "%d", this->PWM_PeriodVal ) );
    }
    catch ( exception& e) {
        cerr << "An exception occurred : Unable to edit PWM Period. | " << e.what( ) << endl;
        return 0;
    }
}

/**
 \fn public function int PWM_SetTargetSpeed( int TargetSpeed )
 \brief Store and write a new PWM Target Speed
 \param <int> PWM_SetTargetSpeed
 \throws Exception on failure to write.
 \return <int> 0 Exception, > 0 success.
 */
void BBBPWMDevice::PWM_SetTargetSpeed( int TargetSpeed ) {
    this->PWM_TargetSpeed = TargetSpeed;
    this->PWM_StartThread( );
}

/**
 \fn public function void PWM_SetDutyVal( void *motor_inst )
 \brief Store and write a new PWM Duty Value
 \param <PWMDevice> pwm_inst
 \throws Exception on failure to write.
 \return <void> 0.
 */
void* BBBPWMDevice::PWM_SetDutyVal( void *pwm_inst ) {
    BBBPWMDevice* PWM_Device = (BBBPWMDevice*)pwm_inst;

    while( PWM_Device->PWM_GetDutyVal( ) != PWM_Device->PWM_TargetSpeed ) {

        try {
            PWM_Device->PWM_DutyVal = ( PWM_Device->PWM_DutyVal < PWM_Device->PWM_TargetSpeed ) ? PWM_Device->PWM_DutyVal + MOTOR_STEPSMOOTH : PWM_Device->PWM_DutyVal - MOTOR_STEPSMOOTH;

            if( abs( PWM_Device->PWM_DutyVal - PWM_Device->PWM_TargetSpeed ) < MOTOR_STEPSMOOTH )
                PWM_Device->PWM_DutyVal = PWM_Device->PWM_TargetSpeed;

            PWM_Device->PWM_SetFileHandle( PWM_Device->duty_file_loc.c_str( ) );
            PWM_Device->PWM_WriteToFile( PWM_Device->PWM_Buffer, snprintf( PWM_Device->PWM_Buffer, sizeof( PWM_Device->PWM_Buffer ), "%d", PWM_Device->PWM_DutyVal ) );
        }
        catch ( exception& e ) {
            cerr << "An exception occurred : Unable to edit PWM Duty. | " << e.what( ) << endl;
        }

    }
    pthread_exit( NULL );

    return 0;
}

/**
 \fn public function int PWM_SetRunVal( int PWM_RunVal )
 \brief Store and write a new PWM Run Value
 \param <int> PWM_RunVal (0 or 1)
 \throws Exception on failure to write.
 \return <int> 0 Exception, > 0 success.
 */
int BBBPWMDevice::PWM_SetRunVal( PWM_RunValues PWM_RunVal ) {
    if(PWM_RunVal < 2 && PWM_RunVal > -1) {
        try {
            this->PWM_RunVal = PWM_RunVal;
            this->PWM_SetFileHandle( this->duty_file_loc.c_str( ) );
            return this->PWM_WriteToFile( this->PWM_Buffer, snprintf( this->PWM_Buffer, sizeof( this->PWM_Buffer ), "%d", this->PWM_DutyVal ) );
        }
        catch ( exception& e ) {
            cerr << "An exception occurred : Unable to edit PWM Run Value. | " << e.what( ) << endl;
            return 0;
        }
    }
    else
        return -1;
}

/**
 \fn public function void PWM_SetPinNum( PWM_PinNum PinNum )
 \brief Sets the Pin Number for this device. (Using the PinNum and BlockNum will tell you where this device is or should be plugged in)
 \param <PWM_PinNum> PinNum
 */
void BBBPWMDevice::PWM_SetPinNum( PWM_PinNum PinNum ) {
    this->PinNum = PinNum;
}

/**
 \fn public function int PWM_SetBlockNum( PWM_BlockNum BlockNum )
 \brief Sets the Block Number for this PWM device. (Using the PinNum and BlockNum will tell you where this device is or should be plugged in)
 \param <PWM_BlockNum> BlockNum
 */
void BBBPWMDevice::PWM_SetBlockNum( PWM_BlockNum BlockNum ) {
    this->BlockNum = BlockNum;
}

/**
 \fn public function int PWM_Init( )
 \brief Setup the PWM device.
 \param <void>
 \return 1 setup successful, 0 fail.
 */
int BBBPWMDevice::PWM_Init( ) {

    if( this->PWM_PinCheck( ) >= RETRIES ) {
        snprintf( this->PWM_PinOverlay, sizeof( this->PWM_PinOverlay ), "bone_pwm_P%d_%d", this->BlockNum, this->PinNum );
        if( this->PWM_LoadOverlay( this->PWM_PinOverlay ) < 0 ) {
            cerr << "Critical Error 2 : Unable to setup PWM on your BeagleBone Black, sys error - unable to export :" << this->PWM_PinOverlay << endl;
            exit( 1 );
        }
        sleep(1);
    }

    if( this->PWM_PinCheck( ) >= RETRIES ) {
        cerr << "Critical Error 3 : Unable to setup PWM on your BeagleBone Black, sys error - unable to export :" << this->PWM_PinOverlayFileName << endl;
        exit( 1 );
    }

    if( this->PWM_SysCheck( ) == -1 ){
        cerr << "Critical Error 1 : Unable to setup PWM on your BeagleBone Black, sys error - unable to export am3xx_pwm" << endl;
        exit( 1 );
    }

    if( this->PWM_LoadPWMDefaultValues( ) == -1 ) {
        cerr << "Critical Error 4 : Unable to use PWM on your BeagleBone Black, sys error - unable to load PWM values on initialisation." << endl;
        exit( 1 );
    }

    return 1;
}


/**
 \fn public function int PWM_GetPinNum( void ) const
 \brief Returns the Pin Number for this device. (Using the PinNum and BlockNum will tell you where this device is or should be plugged in)
 \param <void>
 \return <int> this->PinNum
 */
int BBBPWMDevice::PWM_GetPinNum( void ) const {
    return this->PinNum;
}

/**
 \fn public function int PWM_GetBlockNum( void ) const
 \brief Returns the Block Number for this PWM device. (Using the PinNum and BlockNum will tell you where this device is or should be plugged in)
 \param <void>
 \return <int> this->BlockNum
 */
int BBBPWMDevice::PWM_GetBlockNum( void ) const {
    return this->BlockNum;
}

/**
 \fn public function int PWM_GetDutyVal( void ) const
 \brief Returns the DutyVal for this PWM Device
 \param <void>
 \return <int> this->PWM_DutyVal
 */
int BBBPWMDevice::PWM_GetDutyVal( void ) const {
    return this->PWM_DutyVal;
}

/**
 \fn public function int PWM_GetPeriodVal( void ) const
 \brief Returns the PeriodVal for this PWM Device
 \param <void>
 \return <int> this->PWM_PeriodVal
 */
int BBBPWMDevice::PWM_GetPeriodVal( void ) const {
    return this->PWM_PeriodVal;
}

/**
 \fn public function int PWM_GetRunVal( void ) const
 \brief Returns the RunVal for this PWM Device
 \param <void>
 \return <int> this->PWM_RunVal
 */
int BBBPWMDevice::PWM_GetRunVal( void ) const {
    return this->PWM_RunVal;
}
