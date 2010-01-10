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
 * MultipartField.cpp
 * 
 * Version: 
 * 1.0
 */
#include "MultipartField.h"

_LIT8( KContentStart, "\r\n\r\n" );

CMultipartFieldPlain::CMultipartFieldPlain( )
	{
	}

CMultipartFieldPlain::~CMultipartFieldPlain( )
	{
	iContent.Close( );
	}

CMultipartFieldPlain* CMultipartFieldPlain::NewLC( const TDesC8& aFieldName, const TDesC8& aContent )
	{
	CMultipartFieldPlain* self = new ( ELeave ) CMultipartFieldPlain( );
	CleanupStack::PushL( self );
	self->ConstructL( aFieldName, aContent );
	return self;
	}

CMultipartFieldPlain* CMultipartFieldPlain::NewL( const TDesC8& aFieldName, const TDesC8& aContent )
	{
	CMultipartFieldPlain* self = CMultipartFieldPlain::NewLC( aFieldName, aContent );
	CleanupStack::Pop( self );
	return self;
	}

void CMultipartFieldPlain::ConstructL( const TDesC8& aFieldName, const TDesC8& aContent )
	{
	CMultipartFieldBase::SetFieldNameL( aFieldName );
	iContent.CreateL( KContentStart( ).Size( ) + aContent.Size( ) );
	iContent = KContentStart;
	iContent += aContent;
	}

TBool CMultipartFieldPlain::ReturnNextDataPart( TDes8& aDataPart )
	{
	TInt maxAmount( aDataPart.MaxSize( ) - aDataPart.Size( ) );
	TInt addition( iContent.Size( ) - iOffset );
	if ( addition > maxAmount )
		{
		aDataPart += iContent.Mid( iOffset, maxAmount );
		iOffset += maxAmount;
		}
	else
		{
		aDataPart += iContent.Mid( iOffset, addition );
		iOffset += addition;
		}
	return iOffset >= iContent.Size( );
	}

TInt CMultipartFieldPlain::AllDataSize( )
	{
	return iContent.Size( );
	}

void CMultipartFieldPlain::DoResetL( )
	{
	iOffset = 0;
	}
