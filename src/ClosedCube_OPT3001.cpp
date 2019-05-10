/*

Arduino library for Texas Instruments OPT3001 Digital Ambient Light Sensor
Written by AA for ClosedCube
---

The MIT License (MIT)

Copyright (c) 2015 ClosedCube Limited

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/
#include "ClosedCube_OPT3001.h"

/*!
 * @brief  OPT3001 constructor using i2c
 * @param  *theWire
 *         optional wire
 */
ClosedCube_OPT3001::ClosedCube_OPT3001(TwoWire *theWire)
{
	_wire = theWire;
}

OPT3001_ErrorCode ClosedCube_OPT3001::begin(uint8_t address) {
	OPT3001_ErrorCode error = NO_ERROR;
	_address = address;
	_wire->begin();

	return NO_ERROR;
}

uint16_t ClosedCube_OPT3001::readManufacturerID() {
	uint16_t result = 0;
	OPT3001_ErrorCode error = writeData(MANUFACTURER_ID);
	if (error == NO_ERROR)
		error = readData(&result);
	return result;
}

uint16_t ClosedCube_OPT3001::readDeviceID() {
	uint16_t result = 0;
	OPT3001_ErrorCode error = writeData(DEVICE_ID);
	if (error == NO_ERROR)
		error = readData(&result);
	return result;
}

OPT3001_Config ClosedCube_OPT3001::readConfig() {
	OPT3001_Config config;
	OPT3001_ErrorCode error = writeData(CONFIG);
	if (error == NO_ERROR)
		error = readData(&config.rawData);
	return config;
}

OPT3001_ErrorCode ClosedCube_OPT3001::writeConfig(OPT3001_Config config) {
	_wire->beginTransmission(_address);
	_wire->write(CONFIG);
	_wire->write(config.rawData >> 8);
	_wire->write(config.rawData & 0x00FF);
	return (OPT3001_ErrorCode)(-10 * _wire->endTransmission());
}

OPT3001 ClosedCube_OPT3001::readResult() {
	return readRegister(RESULT);
}

OPT3001 ClosedCube_OPT3001::readHighLimit() {
	return readRegister(HIGH_LIMIT);
}

OPT3001 ClosedCube_OPT3001::readLowLimit() {
	return readRegister(LOW_LIMIT);
}

OPT3001 ClosedCube_OPT3001::readRegister(OPT3001_Commands command) {
	OPT3001_ErrorCode error = writeData(command);
	if (error == NO_ERROR) {
		OPT3001 result;
		result.lux = 0;
		result.error = NO_ERROR;

		OPT3001_ER er;
		error = readData(&er.rawData);
		if (error == NO_ERROR) {
			result.raw = er;
			result.lux = 0.01*pow(2, er.Exponent)*er.Result;
		}
		else {
			result.error = error;
		}

		return result;
	}
	else {
		return returnError(error);
	}
}

OPT3001_ErrorCode ClosedCube_OPT3001::writeData(OPT3001_Commands command)
{
	_wire->beginTransmission(_address);
	_wire->write(command);
	return (OPT3001_ErrorCode)(-10 * _wire->endTransmission(true));
}

OPT3001_ErrorCode ClosedCube_OPT3001::readData(uint16_t* data)
{
	uint8_t	buf[2];

	_wire->requestFrom(_address, (uint8_t)2);

	int counter = 0;
	while (_wire->available() < 2)
	{
		counter++;
		delay(10);
		if (counter > 250)
			return TIMEOUT_ERROR;
	}

	_wire->readBytes(buf, 2);
	*data = (buf[0] << 8) | buf[1];

	return NO_ERROR;
}


OPT3001 ClosedCube_OPT3001::returnError(OPT3001_ErrorCode error) {
	OPT3001 result;
	result.lux = 0;
	result.error = error;
	return result;
}
