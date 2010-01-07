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
*
* Contributors:
*
* Description:
* MultipartFieldFile.h
* 
* Version: 
* 1.0
*/

#include "MultipartFieldFile.h"
#include <utf.h>

_LIT8(KHeader, "; filename=\"%S\"\r\n\r\n");
_LIT8(KHeaderContetnType, "Content-Type: %S\r\n\r\n");

CMultipartFieldFile::CMultipartFieldFile( RFs& aFs ) :
	iFs( aFs )
	{
	}

CMultipartFieldFile::~CMultipartFieldFile()
	{
	delete iFileName;
	delete iMimeType;
	iFile.Close();
	}

CMultipartFieldFile* CMultipartFieldFile::NewLC( const TDesC8& aFieldName,
          									 const TDesC& aFileName, RFs& aFs, const TDesC8& aMimeType )
	{
	CMultipartFieldFile* self = new ( ELeave ) CMultipartFieldFile( aFs );
	CleanupStack::PushL( self );
	self->ConstructL( aFieldName, aFileName, aMimeType );
	return self;
	}

CMultipartFieldFile* CMultipartFieldFile::NewL( const TDesC8& aFieldName,
         									 const TDesC& aFileName, RFs& aFs, const TDesC8& aMimeType )
	{
	CMultipartFieldFile* self = CMultipartFieldFile::NewLC( aFieldName,
														  aFileName, aFs, aMimeType );
	CleanupStack::Pop( self );
	return self;
	}

void CMultipartFieldFile::ConstructL( const TDesC8& aFieldName,
									 const TDesC& aFileName, const TDesC8& aMimeType )
	{
	CMultipartFieldBase::SetFieldNameL( aFieldName );
	TParse parse;
	parse.Set( aFileName, NULL, NULL );
	iFileName = CnvUtfConverter::ConvertFromUnicodeToUtf8L( parse.NameAndExt() );
	User::LeaveIfError( iFile.Open( iFs, aFileName, EFileRead | EFileShareAny
			| EFileStream ) );
	iSize = iFile.Source()->SizeL();
	iMimeType = aMimeType.AllocL();
	}

TBool CMultipartFieldFile::ReturnNextDataPart( TDes8& aDataPart )
	{
	TInt ss(0);
	if( !iOffset )
		{
		TPtrC8 ptr( *iFileName );
		aDataPart.AppendFormat( KHeader, &ptr );
		if( iMimeType->Des().Length() )
			{
			ptr.Set( *iMimeType );
			aDataPart.AppendFormat( KHeaderContetnType, &ptr );
			}
		}
	TInt maxAmount( aDataPart.MaxSize() - aDataPart.Size() );
	HBufC8* buffer = HBufC8::New( maxAmount );
	if( !buffer )
		{
		User::InfoPrint(_L("Buffer error"));
		return ETrue;
		}
	TPtr8 ptr( buffer->Des() );
	TRAPD( error, iFile.ReadL( ptr, Min( (iSize - iOffset), maxAmount ) ) );
	if( error != KErrEof && error != KErrNone )
		{
		User::InfoPrint(_L("File error"));
		delete buffer;
		return ETrue;
		}
	iOffset += ptr.Size();
	aDataPart.Append( ptr );
	delete buffer;
	return iOffset >= iSize;
	}

TInt CMultipartFieldFile::AllDataSize()
	{
	TInt size( iSize );
	size += KHeader().Size() - 2 + iFileName->Des().Size();//for %S
	if( iMimeType->Des().Size() )
		{
		size += KHeaderContetnType().Size() - 2 + iMimeType->Des().Size();// 2 is for %S
		}
	return size;
	}

void CMultipartFieldFile::DoResetL()
	{
	TInt pos(0);
	iFile.Source()->SeekL( EStreamBeginning, pos );
	iOffset = 0;
	}
