/*
 * Copyright (c) 2009, Pawe³ Polañski
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * the names of its contributors may not be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ''AS IS'' 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL PIOTR WACH, POLIDEA BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * Initial Contributors:
 * Pawe³ Polañski - Initial contribution
 * Tomasz Polañski
 * 
 * Contributors:
 *
 * Description:
 * MultipartEncoder.cpp
 * 
 * Version: 
 * 1.0
 */
#include "MultipartEncoder.h"
#include "MultipartFieldBase.h"


_LIT8(KStartBoundary, "--" KBoundryId "\r\n" );
_LIT8(KNextBoundary, "\r\n--" KBoundryId "\r\n" );
_LIT8(KLastBoundary, "\r\n--" KBoundryId "--\r\n");

const TInt KDefaultBufferSize = 0x5000;

CMultipartEncoder::CMultipartEncoder( )
	{
	}

CMultipartEncoder::~CMultipartEncoder( )
	{
	iData.Close( );
	iFieldArray.ResetAndDestroy( );
	}

CMultipartEncoder* CMultipartEncoder::NewLC( )
	{
	CMultipartEncoder* self = new ( ELeave ) CMultipartEncoder( );
	CleanupStack::PushL( self );
	self->ConstructL( );
	return self;
	}

CMultipartEncoder* CMultipartEncoder::NewL( )
	{
	CMultipartEncoder* self = CMultipartEncoder::NewLC( );
	CleanupStack::Pop( self );
	return self;
	}

void CMultipartEncoder::ConstructL( )
	{
	TInt error( KErrNoMemory );
	TInt bufferSize( KDefaultBufferSize );
	TInt i( 5 );
	do
		{
		iData.Close( );
		error = iData.Create( bufferSize );
		bufferSize /= 2;
		}
	while ( error && --i );
	User::LeaveIfError( error );
	}

void CMultipartEncoder::ResetEncoderL( )
	{
	iData.Zero( );
	iFieldArray.ResetAndDestroy( );
	iCurrentField = 0;
	iHasData = EFalse;
	iSavedResponse = EFalse;
	}

void CMultipartEncoder::AddFieldL( CMultipartFieldBase* aField )
	{
	iFieldArray.AppendL( aField );
	}

TBool CMultipartEncoder::GetNextDataPart( TPtrC8& aDataPart )
	{
	__ASSERT_ALWAYS( iFieldArray.Count()> iCurrentField, User::Panic( _L("Field"), KErrUnderflow ) );
	if ( !iHasData )
		{
		iData = KLastBoundary;
		iData += iCurrentField ? KNextBoundary( ) : KStartBoundary( );
		iSavedResponse = iFieldArray[ iCurrentField ]->GetNextDataPart( iData );
		iData.Delete( 0, KLastBoundary( ).Length( ) );
		if ( iCurrentField + 1 >= iFieldArray.Count( ) && iSavedResponse )
			{
			iData += KLastBoundary;
			}

		iHasData = ETrue;
		}
	aDataPart.Set( iData );
	/*	TInt sizzz( aDataPart.Size() );
	 RFs fs;
	 fs.Connect();
	 RFile file;//TODO remove
	 TInt error( file.Replace( fs, _L("c:\\Data\\sss.txt"), EFileWrite ) );
	 User::LeaveIfError( error );
	 CleanupClosePushL( file );
	 TInt position = 0;
	 User::LeaveIfError( file.Seek( ESeekEnd, position ) );
	 TTime timeee;
	 timeee.UniversalTime();
	 User::LeaveIfError( file.Write( aDataPart ) );
	 CleanupStack::PopAndDestroy( &file );
	 fs.Close();*/
	return iCurrentField + 1 >= iFieldArray.Count( ) && iSavedResponse;
	}

void CMultipartEncoder::ReleaseData( )
	{
	iFieldArray[ iCurrentField++ ]->ReleaseData( );
	}

TInt CMultipartEncoder::OverallDataSize( )
	{
	if ( iWholeSize )
		{
		return iWholeSize;
		}
	for ( TInt i( 0 ); i < iFieldArray.Count( ); ++i )
		{
		iWholeSize += iFieldArray[ i ]->OverallDataSize( );
		}
	iWholeSize += KStartBoundary( ).Size( ) + KNextBoundary( ).Size( )
			* ( iFieldArray.Count( ) > 1 ? iFieldArray.Count( ) - 1 : 0 ) + KLastBoundary( ).Size( );
	return iWholeSize;
	}

TInt CMultipartEncoder::Reset( )
	{
	TInt error( KErrNone );
	for ( TInt i( iCurrentField ); i >= 0; --i )
		{
		if ( iFieldArray.Count( ) > i )
			{
			error = iFieldArray[ i ]->Reset( );
			if ( error )
				{
				return error;
				}
			}
		}
	iData.Zero( );
	iHasData = EFalse;
	iCurrentField = 0;
	iSavedResponse = EFalse;
	iWholeSize = 0;
	return error;
	}
