/*
 *  bcu.h - EIB bus coupling unit.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#define INSIDE_BCU_CPP
//#include <sblib/io_pin_names.h>
#include "bcu_base.h"
#include "user_memory.h"
//#include <sblib/internal/functions.h>
//#include <sblib/internal/variables.h>
//#include <sblib/internal/iap.h>
#include <string.h>

#ifdef DUMP_TELEGRAMS
extern Stream Serial;
#endif

extern unsigned int writeUserEepromTime;
extern volatile unsigned int systemTime;

BcuBase::BcuBase()
//:progButtonDebouncer()
{
    //progPin = PIN_PROG;
    progPinInv = true;
    enabled = false;
}

// The method begin_BCU() is renamed during compilation to indicate the BCU type.
// If you get a link error then the library's BCU_TYPE is different from your application's BCU_TYPE.
void BcuBase::begin_BCU(int manufacturer, int deviceType, int version, int busHalSettings)
{
	_begin();
#ifdef DUMP_TELEGRAMS
//    serial.begin(115200);
//    serial.println("Telegram dump enabled");
#endif

    sendTelegram[0] = 0;
    sendCtrlTelegram[0] = 0;

    connectedSeqNo = 0;
    incConnectedSeqNo = false;
    lastAckSeqNo = -1;

    connectedAddr = 0;

    userRam.status = BCU_STATUS_LL | BCU_STATUS_TL | BCU_STATUS_AL | BCU_STATUS_USR;
    userRam.deviceControl = 0;
    userRam.runState = 1;

    userEeprom.runError = 0xff;
    userEeprom.portADDR = 0;

    userEeprom.manufacturerH = manufacturer >> 8;
    userEeprom.manufacturerL = manufacturer;

    userEeprom.deviceTypeH = deviceType >> 8;
    userEeprom.deviceTypeL = deviceType;

    userEeprom.version = version;

#if BCU_TYPE != BCU1_TYPE
    unsigned int serial;
    iapReadPartID(& serial);
    memcpy (userEeprom.serial, &serial, 4);
    userEeprom.serial[4] = SBLIB_VERSION >> 8;
    userEeprom.serial[5] = SBLIB_VERSION;

    userRam.peiType = 0;     // PEI type: 0=no adapter connected to PEI.
    userEeprom.appType = 0;  // Set to BCU2 application. ETS reads this when programming.
#endif

    writeUserEepromTime = 0;
    enabled = true;
    knxBus.begin(busHalSettings);
    //progButtonDebouncer.init(1);
}

void BcuBase::_begin()
{

}

void BcuBase::end()
{
    enabled = false;

    knxBus.end();
}

void BcuBase::setOwnAddress(int addr)
{
    userEeprom.addrTab[0] = addr >> 8;
    userEeprom.addrTab[1] = addr;
#if BCU_TYPE != BCU1_TYPE
    if (userEeprom.loadState[OT_ADDR_TABLE] == LS_LOADING)
    {
        byte * addrTab =  addrTable() + 1;

        * (addrTab + 0)  = addr >> 8;
        * (addrTab + 1)  = addr;
    }
#endif
    userEeprom.modified();

    knxBus.ownAddr = addr;
}

void BcuBase::setOwnAddress(int addr1, int addr2, int addr3)
{
    setOwnAddress(addr1*256*16 + addr2*256 + addr3);    
}

void BcuBase::loop()
{
    if (!enabled)
        return;

#ifdef DUMP_TELEGRAMS
	{
   	if (knxBus.telegramLen > 0)
   	{
   		Serial.print("RCV: ");
           for (int i = 0; i < knxBus.telegramLen; ++i)
           {
               if (i) Serial.print(" ");
               Serial.print(knxBus.telegram[i], HEX);
           }
           Serial.println();
   	}
	}
#endif

    if (knxBus.telegramReceived() && !knxBus.sendingTelegram() && (userRam.status & BCU_STATUS_TL))
        processTelegram();

    if (progPin)
    {
        // Detect the falling edge of pressing the prog button
        //pinMode(progPin, INPUT|PULL_UP);
        //int oldValue = progButtonDebouncer.value();
        //if (!progButtonDebouncer.debounce(digitalRead(progPin), 50) && oldValue)
        //{
        //    userRam.status ^= 0x81;  // toggle programming mode and checksum bit
        //}
        //pinMode(progPin, OUTPUT);
        //digitalWrite(progPin, (userRam.status & BCU_STATUS_PROG) ^ progPinInv);
    }
}

void BcuBase::processTelegram()
{
}
