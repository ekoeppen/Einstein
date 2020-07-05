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
Long DeregisterInterrupt(InterruptObject*);

extern "C" long EnableInterrupt(InterruptObject*, ULong);
extern "C" long DisableInterrupt(InterruptObject*);
extern "C" long ClearInterrupt(InterruptObject*);

#define kCMOSerialEinsteinLoc 'eloc'

THMOSerialEinsteinHardware::THMOSerialEinsteinHardware()
{
	SetAsOption(kCMOSerialEinsteinLoc);
	SetLength(sizeof(THMOSerialEinsteinHardware));
}

NewtonErr
TSerialChipEinstein::HandleInterrupt()
{
	ULong intStatus = GetSerialStatus();
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
			(InterruptHandlerProcPtr) RelocFuncPtr(&TSerialChipEinstein::HandleInterrupt),
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
