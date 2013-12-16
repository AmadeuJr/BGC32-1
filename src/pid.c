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

uint8_t holdIntegrators = true;

///////////////////////////////////////////////////////////////////////////////

void initPID(void)
{
    uint8_t index;

    for (index = 0; index < NUMBER_OF_PIDS; index++)
    {
    	eepromConfig.PID[index].iTerm          = 0.0f;
    	eepromConfig.PID[index].lastDcalcValue = 0.0f;
    	eepromConfig.PID[index].lastDterm      = 0.0f;
    	eepromConfig.PID[index].lastLastDterm  = 0.0f;
	}
}

///////////////////////////////////////////////////////////////////////////////

float updatePID(float command, float state, float deltaT, uint8_t iHold, struct PIDdata *PIDparameters)
{
    float error;
    float dTerm;

    ///////////////////////////////////

    error = command - state;

    if (PIDparameters->type == ANGULAR)
        error = standardRadianFormat(error);

    ///////////////////////////////////

    if (iHold == false)
    {
    	PIDparameters->iTerm += error * deltaT;
    	PIDparameters->iTerm = constrain(PIDparameters->iTerm, -PIDparameters->windupGuard, PIDparameters->windupGuard);
    }

    ///////////////////////////////////

    if (PIDparameters->dErrorCalc == D_ERROR)  // Calculate D term from error
    {
		dTerm = (error - PIDparameters->lastDcalcValue) / deltaT;
        PIDparameters->lastDcalcValue = error;
	}
	else                                       // Calculate D term from state
	{
		dTerm = (PIDparameters->lastDcalcValue - state) / deltaT;

		if (PIDparameters->type == ANGULAR)
		    dTerm = standardRadianFormat(dTerm);

		PIDparameters->lastDcalcValue = state;
	}

    ///////////////////////////////////

    if (PIDparameters->type == ANGULAR)
        return(PIDparameters->P * error                +
	           PIDparameters->I * PIDparameters->iTerm +
	           PIDparameters->D * dTerm);
    else
        return(PIDparameters->P * PIDparameters->B * command +
               PIDparameters->I * PIDparameters->iTerm       +
               PIDparameters->D * dTerm                   -
               PIDparameters->P * state);

    ///////////////////////////////////
}

///////////////////////////////////////////////////////////////////////////////

void setPIDintegralError(uint8_t IDPid, float value)
{
	eepromConfig.PID[IDPid].iTerm = value;
}

///////////////////////////////////////////////////////////////////////////////

void zeroPIDintegralError(void)
{
    uint8_t index;

    for (index = 0; index < NUMBER_OF_PIDS; index++)
         setPIDintegralError(index, 0.0f);
}

///////////////////////////////////////////////////////////////////////////////

void setPIDstates(uint8_t IDPid, float value)
{
    eepromConfig.PID[IDPid].lastDcalcValue = value;
    eepromConfig.PID[IDPid].lastDterm      = value;
    eepromConfig.PID[IDPid].lastLastDterm  = value;
}

///////////////////////////////////////////////////////////////////////////////

void zeroPIDstates(void)
{
    uint8_t index;

    for (index = 0; index < NUMBER_OF_PIDS; index++)
         setPIDstates(index, 0.0f);
}

///////////////////////////////////////////////////////////////////////////////


