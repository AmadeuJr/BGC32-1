/*

BGC32 from FocusFlight, a new alternative firmware
for the EvvGC controller

Original work Copyright (c) 2013 John Ihlein
                                 Alan K. Adamson

This file is part of BGC32.

Includes code and/or ideas from:

  1)BaseFlight
  2)EvvGC
  2)S.O.H. Madgwick

BGC32 is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

BGC32 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with EvvGC. If not, see <http://www.gnu.org/licenses/>.

*/

///////////////////////////////////////////////////////////////////////////////

#include "board.h"

///////////////////////////////////////////////////////////////////////////////

eepromConfig_t eepromConfig;

sensors_t      sensors;

float          testPhase      = -1.0f * D2R;
float          testPhaseDelta = 10.0f * D2R;

uint16_t       timerValue;

///////////////////////////////////////////////////////////////////////////////

int main(void)
{
    uint32_t currentTime;

    systemInit();

    systemReady = true;

    while (1)
    {
        ///////////////////////////////

        if (frame_50Hz)
        {
            frame_50Hz = false;

            currentTime      = micros();
            deltaTime50Hz    = currentTime - previous50HzTime;
            previous50HzTime = currentTime;

            processPointingCommands();

            executionTime50Hz = micros() - currentTime;
        }

        ///////////////////////////////

        if (frame_10Hz)
        {
            frame_10Hz = false;

            currentTime      = micros();
            deltaTime10Hz    = currentTime - previous10HzTime;
            previous10HzTime = currentTime;

            // HJI if (newMagData == true)
            // HJI {
            // HJI     sensors.mag10Hz[XAXIS] =   (float)rawMag[XAXIS].value * magScaleFactor[XAXIS] - eepromConfig.magBias[XAXIS];
            // HJI     sensors.mag10Hz[YAXIS] =   (float)rawMag[YAXIS].value * magScaleFactor[YAXIS] - eepromConfig.magBias[YAXIS];
            // HJI     sensors.mag10Hz[ZAXIS] = -((float)rawMag[ZAXIS].value * magScaleFactor[ZAXIS] - eepromConfig.magBias[ZAXIS]);

            // HJI     newMagData = false;
            // HJI     magDataUpdate = true;
            // HJI }

            cliCom();

            if (!gimbalStateEnabled)
                LED1_TOGGLE;

            executionTime10Hz = micros() - currentTime;
        }

        ///////////////////////////////

        if (frame_500Hz)
        {
#ifdef _DTIMING
            LA2_ENABLE;
#endif
            frame_500Hz = false;

            currentTime       = micros();
            deltaTime500Hz    = currentTime - previous500HzTime;
            previous500HzTime = currentTime;

            TIM_Cmd(TIM6, DISABLE);
            timerValue = TIM_GetCounter(TIM6);
            TIM_SetCounter(TIM6, 0);
            TIM_Cmd(TIM6, ENABLE);

            dt500Hz = (float)timerValue * 0.0000005f;  // For integrations in 500 Hz loop

            sensors.accel500Hz[XAXIS] =  ((float)accelData500Hz[XAXIS] - accelTCBias[XAXIS]) * ACCEL_SCALE_FACTOR;
            sensors.accel500Hz[YAXIS] =  ((float)accelData500Hz[YAXIS] - accelTCBias[YAXIS]) * ACCEL_SCALE_FACTOR;
            sensors.accel500Hz[ZAXIS] = -((float)accelData500Hz[ZAXIS] - accelTCBias[ZAXIS]) * ACCEL_SCALE_FACTOR;

            sensors.gyro500Hz[ROLL ] =  ((float)gyroData500Hz[ROLL ] - gyroRTBias[ROLL ] - gyroTCBias[ROLL ]) * GYRO_SCALE_FACTOR;
            sensors.gyro500Hz[PITCH] =  ((float)gyroData500Hz[PITCH] - gyroRTBias[PITCH] - gyroTCBias[PITCH]) * GYRO_SCALE_FACTOR;
            sensors.gyro500Hz[YAW  ] = -((float)gyroData500Hz[YAW  ] - gyroRTBias[YAW  ] - gyroTCBias[YAW  ]) * GYRO_SCALE_FACTOR;

            sensors.accel500Hz[ROLL ] = firstOrderFilter(sensors.accel500Hz[ROLL ], &firstOrderFilters[ACCEL_X_500HZ_LOWPASS ]);
            sensors.accel500Hz[PITCH] = firstOrderFilter(sensors.accel500Hz[PITCH], &firstOrderFilters[ACCEL_Y_500HZ_LOWPASS]);
            sensors.accel500Hz[YAW  ] = firstOrderFilter(sensors.accel500Hz[YAW  ], &firstOrderFilters[ACCEL_Z_500HZ_LOWPASS  ]);

            MargAHRSupdate(sensors.gyro500Hz[ROLL],   sensors.gyro500Hz[PITCH],  sensors.gyro500Hz[YAW],
                           sensors.accel500Hz[XAXIS], sensors.accel500Hz[YAXIS], sensors.accel500Hz[ZAXIS],
                           sensors.mag10Hz[XAXIS],    sensors.mag10Hz[YAXIS],    sensors.mag10Hz[ZAXIS],
                           false,  //magDataUpdate,
                           dt500Hz);

            //magDataUpdate = false;

            computeMotorCommands(dt500Hz);

            executionTime500Hz = micros() - currentTime;

#ifdef _DTIMING
            LA2_DISABLE;
#endif
        }

        ///////////////////////////////

        if (frame_100Hz)
        {
            frame_100Hz = false;

            currentTime       = micros();
            deltaTime100Hz    = currentTime - previous100HzTime;
            previous100HzTime = currentTime;

            executionTime100Hz = micros() - currentTime;
        }

        ///////////////////////////////

        if (frame_5Hz)
        {
            frame_5Hz = false;

            currentTime     = micros();
            deltaTime5Hz    = currentTime - previous5HzTime;
            previous5HzTime = currentTime;

            LED2_TOGGLE;

            executionTime5Hz = micros() - currentTime;
        }

        ///////////////////////////////

        if (frame_1Hz)
        {
            frame_1Hz = false;

            currentTime     = micros();
            deltaTime1Hz    = currentTime - previous1HzTime;
            previous1HzTime = currentTime;

            if (gimbalStateEnabled)
                LED1_TOGGLE;

            executionTime1Hz = micros() - currentTime;
        }

        ////////////////////////////////
    }
}

///////////////////////////////////////////////////////////////////////////////
