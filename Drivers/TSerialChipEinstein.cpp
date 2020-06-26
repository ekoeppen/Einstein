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

PROTOCOL_IMPL_SOURCE_MACRO(TSerialChipEinstein)	// Magic stuff, do not touch

// -------------------------------------------------------------------------- //
// Constantes
// -------------------------------------------------------------------------- //

// -------------------------------------------------------------------------- //
//  * New( void )
// -------------------------------------------------------------------------- //
TSerialChip*
TSerialChipEinstein::New( void )
{
	return this;
}

// -------------------------------------------------------------------------- //
//  * Delete( void )
// -------------------------------------------------------------------------- //
void
TSerialChipEinstein::Delete( void )
{
}

// -------------------------------------------------------------------------- //
//  * InitByOption( TOption* )
// -------------------------------------------------------------------------- //
NewtonErr
TSerialChipEinstein::InitByOption( TOption* inOption )
{
	mLocationID = ((THMOSerialEinsteinHardware*) inOption)->fLocationID;
	return noErr;
}

// ======================================================================= //
// Every program has at least one bug and can be shortened by at least one //
// instruction -- from which, by induction, one can deduce that every      //
// program can be reduced to one instruction which doesn't work.           //
// ======================================================================= //
