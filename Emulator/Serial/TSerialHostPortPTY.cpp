// ==============================
// File:			TSerialHostPortPTY.cpp
// Project:			Einstein
//
// Copyright 2020 Eckhart Koeppen
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

extern "C" {
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
}

#include <Emulator/TEmulator.h>
#include <Emulator/TInterruptManager.h>
#include "TSerialHostPortPTY.h"

TSerialHostPortPTY::TSerialHostPortPTY(
			TLog *inLog,
			KUInt32 inLocation,
			TEmulator *inEmulator)
	:
		TSerialHostPort( inLog, inLocation, inEmulator )
		, mPath( nullptr )
		, mIOThread( nullptr )
		, mReadMutex( nullptr )
		, mReadBuffer( nullptr )
{
	mPath = static_cast<char*>( malloc( PATH_MAX ) );
	snprintf( mPath, PATH_MAX, "/tmp/einstein-%08x.pty", inLocation );
	CreatePTY();
}

TSerialHostPortPTY::~TSerialHostPortPTY()
{
	if (mMaster != -1)
	{
		close( mMaster );
	}
	if (mSlave != -1)
	{
		close( mSlave );
	}
	delete mIOThread;
	delete mPath;
}

void TSerialHostPortPTY::CreatePTY()
{
	unlink( mPath );
	mMaster = posix_openpt( O_RDWR | O_NOCTTY );
	grantpt( mMaster );
	unlockpt( mMaster );
	const char *slaveName = ptsname( mMaster );
	mSlave = open( slaveName, O_RDWR | O_NOCTTY );
	symlink( slaveName , mPath );

	struct termios options;
	tcgetattr( mMaster , &options );
	cfmakeraw( &options );
	options.c_cflag |= HUPCL | CLOCAL;
	tcsetattr( mMaster, TCSANOW, &options );

	tcgetattr( mSlave, &options);
	cfmakeraw( &options);
	options.c_cflag |= HUPCL | CLOCAL;
	tcsetattr( mSlave, TCSANOW, &options );

	mLog->FLogLine( "[####] TSerialHostPortPTY: port opened: %p", this );
	mReadMutex = new TMutex();
	mReadBuffer = new TCircleBuffer( 8 * 1024 );
	mIOThread = new TThread( this );
}

void TSerialHostPortPTY::PutByte(KUInt8 nextChar)
{
	write( mMaster, &nextChar, 1 );
	mSerialStatus &= ~kSerialTxBufferEmpty;
	mEmulator->GetInterruptManager()->RaiseInterrupt( 0x00100000 );
}

KUInt8 TSerialHostPortPTY::GetByte()
{
	KUInt8 data;
	mReadMutex->Lock();
	mReadBuffer->Consume( &data, sizeof(data) );
	if (mReadBuffer->IsEmpty())
	{
		mSerialStatus &= ~kSerialRxCharAvailable;
	}
	mReadMutex->Unlock();
	return data;
}

void TSerialHostPortPTY::SetSerialOutputs(SerialOutputControl)
{
}

void TSerialHostPortPTY::ClearSerialOutputs(SerialOutputControl)
{
}

void TSerialHostPortPTY::SetInterruptEnable(bool enable)
{
}

void TSerialHostPortPTY::Reset()
{
}

void TSerialHostPortPTY::SetBreak(bool assert)
{
}


InterfaceSpeed TSerialHostPortPTY::SetSpeed(BitRate bitsPerSec)
{
	return 0;
}

void TSerialHostPortPTY::SetIOParms(TCMOSerialIOParms* opt)
{
}

void TSerialHostPortPTY::Reconfigure()
{
}

KSInt32 TSerialHostPortPTY::InitByOption(TOption* initOpt)
{
	return 0;
}

KSInt32 TSerialHostPortPTY::ProcessOption(TOption* opt)
{
	return 0;
}

KSInt32 TSerialHostPortPTY::SetSerialMode(SerialMode mode)
{
	return 0;
}

RxErrorStatus TSerialHostPortPTY::GetByteAndStatus(KUInt8* nextCharPtr)
{
	return 0;
}

bool TSerialHostPortPTY::AllSent()
{
	return true;
}


KSInt32 TSerialHostPortPTY::WaitForAllSent()
{
	return 0;
}

void TSerialHostPortPTY::Run()
{
	fd_set fdSet;
	while (true)
	{
		mLog->LogLine( "[####] TSerialHostPortPTY: Waiting for data..." );
		FD_ZERO( &fdSet );
		FD_SET( mMaster, &fdSet );
		if (select( FD_SETSIZE, &fdSet, 0, 0, 0 ) != -1 &&
				FD_ISSET( mMaster, &fdSet) )
		{
			KUInt8 data;
			mReadMutex->Lock();
			mLog->LogLine( "[####] TSerialHostPortPTY: data available" );
			while (read( mMaster, &data, sizeof(data)) == sizeof(data))
			{
				mReadBuffer->Produce( &data, sizeof(data));
			}
			mReadMutex->Unlock();
			mSerialStatus |= kSerialRxCharAvailable;
			mEmulator->GetInterruptManager()->RaiseInterrupt( 0x00100000 );
		}
	}
}
