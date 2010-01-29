/*
 * Copyright (c) 2009, Pawe� Pola�ski
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
 * DISCLAIMED. IN NO EVENT SHALL PAWE� POLA�SKI BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * Initial Contributors:
 * Pawe� Pola�ski - Initial contribution
 * Tomasz Pola�ski
 * 
 * Contributors:
 *
 * Description:
 * HttpHeader.cpp
 * 
 * Version: 
 * 1.0
 */

#include "HttpHeader.h"
#include <BADESCA.H>

CHttpHeaders::CHttpHeaders( )
	{
	}

CHttpHeaders::~CHttpHeaders( )
	{
	delete iKeyArray;
	delete iValueArray;
	}

CHttpHeaders* CHttpHeaders::NewLC( )
	{
	CHttpHeaders* self = new ( ELeave ) CHttpHeaders( );
	CleanupStack::PushL( self );
	self->ConstructL( );
	return self;
	}

CHttpHeaders* CHttpHeaders::NewL( )
	{
	CHttpHeaders* self = CHttpHeaders::NewLC( );
	CleanupStack::Pop( self );
	return self;
	}

void CHttpHeaders::ConstructL( )
	{
	const TInt KDefaultArraySize = 10;
	iKeyArray = new ( ELeave ) CDesC8ArrayFlat( KDefaultArraySize );
	iValueArray = new ( ELeave ) CDesC8ArrayFlat( KDefaultArraySize );
	}

void CHttpHeaders::AddL( const TDesC8& aKey, const TInt& aValue )
	{
	const TInt KMaxBuffLen = 20;
	TBuf8< KMaxBuffLen > buf;
	buf.Num( aValue );
	AddL( aKey, buf );
	}

void CHttpHeaders::AddL( const TDesC8& aKey, const TInt64& aValue )
	{
	const TInt KMaxBuffLen = 40;
	TBuf8< KMaxBuffLen > buf;
	buf.Num( aValue );
	AddL( aKey, buf );
	}

void CHttpHeaders::AddL( const TDesC8& aKey, const TDesC8& aValue )
	{
	TInt index = iKeyArray->InsertIsqL( aKey );
	iValueArray->InsertL( index, aValue );
	}

TPtrC8 CHttpHeaders::Key( TInt aIndex ) const
	{
	return TPtrC8( ( *iKeyArray )[ aIndex ] );
	}

TPtrC8 CHttpHeaders::Value( TInt aIndex ) const
	{
	return TPtrC8( ( *iValueArray )[ aIndex ] );
	}

TInt CHttpHeaders::Count( ) const
	{
	return iKeyArray->Count( );
	}

TInt CHttpHeaders::Find( const TDesC8& aKey ) const
	{
	TInt pos;
	TInt error( iKeyArray->FindIsq( aKey, pos ) );
	return error == 0 ? pos : KErrNotFound;
	}

void CHttpHeaders::Reset( )
	{
	iKeyArray->Reset( );
	iValueArray->Reset( );
	}
