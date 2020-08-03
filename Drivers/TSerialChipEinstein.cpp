// ==============================
// File:			TSerialChipEinstein.cpp
// Project:			Einstein
//
// Copyright 2020 by Eckhart Koeppen (eck@40hz.org)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// ==============================
// $Id$
// ==============================

#include "TSerialChipEinstein.impl.h"
#include <OptionArray.h>
#include <HALOptions.h>
#include <CommErrors.h>
#include "K/Misc/RelocHack.h"

PROTOCOL_IMPL_SOURCE_MACRO(TSerialChipEinstein)	// Magic stuff, do not touch

typedef long (*InterruptHandlerProcPtr)(void*);

InterruptObject* RegisterInterrupt(
	ULong inInterruptMask,
	void* inCookie,
	InterruptHandlerProcPtr inHandler,
	ULong inFlags);
void DeregisterInterrupt(InterruptObject*);

extern "C" long EnableInterrupt(InterruptObject*, ULong);
extern "C" long DisableInterrupt(InterruptObject*);
extern "C" long ClearInterrupt(InterruptObject*);

#define kCMOSerialEinsteinLoc 'eloc'

__attribute__((naked)) void nwt_log(const char *str)
{
    asm("stmdb   sp!, {r1, lr}\n"
        "ldr     lr, id0\n"
        "mov     r1, r0\n"
        "mcr     p10, 0, lr, c0, c0\n"
        "ldmia   sp!, {r1, pc}\n"
        "id0:\n"
        ".word      0x11a\n");
}

void LH(const char *func, int line, int v)
{
    char buffer[80];
    sprintf(buffer, "%s %d %d", func, line, v);
    nwt_log(buffer);
}

THMOSerialEinsteinHardware::THMOSerialEinsteinHardware()
{
	SetAsOption(kCMOSerialEinsteinLoc);
	SetLength(sizeof(THMOSerialEinsteinHardware));
}

NewtonErr
TSerialChipEinstein::HandleInterrupt()
{
	SerialStatus intStatus = GetSerialStatus();
	if (intStatus & kSerialTxBufferEmpty) {
		(fIntHandlers.TxBEmptyIntHandler)(fSerialTool);
	}
	if (intStatus & kSerialRxCharAvailable) {
		(fIntHandlers.RxCAvailIntHandler)(fSerialTool);
	}
	return noErr;
}

TSerialChip*
TSerialChipEinstein::New(void)
{
	fInterruptObject = RegisterInterrupt(0x00100000,
			this,
			(InterruptHandlerProcPtr) &TSerialChipEinstein::HandleInterrupt,
			0);
	EnableInterrupt(fInterruptObject, 0);
	fLocationID = 0;
	fInterruptObject = NULL;
	RemoveChipHandler(NULL);
	return this;
}

void
TSerialChipEinstein::Delete( void )
{
	DeregisterInterrupt(fInterruptObject);
}

NewtonErr
TSerialChipEinstein::InstallChipHandler(void* serialTool, SCCChannelInts* intHandlers)
{
	fSerialTool = serialTool;
	memcpy(&fIntHandlers, intHandlers, sizeof(fIntHandlers));
	return noErr;
}

NewtonErr
TSerialChipEinstein::RemoveChipHandler(void* serialTool)
{
	fSerialTool = NULL;
	memset(&fIntHandlers, 0, sizeof(fIntHandlers));
	return noErr;
}

// ======================================================================= //
// Every program has at least one bug and can be shortened by at least one //
// instruction -- from which, by induction, one can deduce that every      //
// program can be reduced to one instruction which doesn't work.           //
// ======================================================================= //