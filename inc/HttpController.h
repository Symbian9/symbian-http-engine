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
* HttpController.h - is handling all HTTP events
* 
* Version: 
* 1.0
*/

#ifndef HTTPCONTROLLER_H
#define HTTPCONTROLLER_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>
#include <f32file.h>
#include <mhttptransactioncallback.h>
#include <ES_SOCK.H>
#include <chttpformencoder.h>
#include <http/rhttpsession.h> 

class CHttpHeaders;
class CHttpDataEncoderBase;
class RWriteStream;
class MHttpObserver;

const TInt KHttpErrorBase = -100000;
const TInt KOwnHttpErrorBase = -1000;

const TInt KErrHttpInvalidObserver = KHttpErrorBase + KOwnHttpErrorBase + 1;

class CHttpController: public CBase, public MHTTPTransactionCallback
	{
public:
	~CHttpController();
	//id content encoder is not specyfied then the default CMultipartEncoder is used
	static CHttpController
			* NewL( RConnection& aConnection, RSocketServ& aSocketServ,
					CHttpDataEncoderBase* aContentEncoder = NULL ); //passes ownership
	static CHttpController
			* NewLC( RConnection& aConnection, RSocketServ& aSocketServ,
					 CHttpDataEncoderBase* aContentEncoder = NULL ); //passes ownership

public:
	void GetL( const TDesC8& aUri, CHttpHeaders* aHeaders = NULL,
			   RWriteStream* aBodyFile = NULL );

	void PostL( const TDesC8& aUri, CHttpHeaders* aHeaders = NULL,
				RWriteStream* aBodyFile = NULL );

	void Cancel();

	void AddPersistentHeaderL( TInt aHeaderId, const TDesC8& aValue );
	void ResetPersistentHeaders();

	inline void SetObserver( MHttpObserver& aObserver );
	inline CHttpDataEncoderBase& ContentEncoder() const;

protected:
	void SetHeaderL( RHTTPHeaders& aHeaders, TInt aHdrField,
					 const TDesC8& aHdrValue );
	void SetHeaderL( RHTTPHeaders& aHeaders, const TDesC8& aHdrName,
					 const TDesC8& aHdrValue );

	TInt ContentLength( RHTTPResponse& aResponse, RHTTPSession& aSession );

	void SendRequestL( TInt aMethodIndex, const TDesC8& aUri );
	void SendRequestL( TInt aMethodIndex, const TDesC8& aUri,
	                   CHttpHeaders* aHeaders );

	void Error( TInt aError );
	void CloseTransaction( RHTTPTransaction& aTransaction );
	void ParseHeadersL( RHTTPTransaction& aTransaction );
	HBufC8* HeaderNameLC( TInt aId );
	
protected:
	//from MHTTPTransactionCallback
	void MHFRunL( RHTTPTransaction aTransaction, const THTTPEvent& aEvent );
	TInt MHFRunError( TInt aError, RHTTPTransaction aTransaction,
					  const THTTPEvent& aEvent );

private:
	CHttpController( CHttpDataEncoderBase* aContentEncoder );
	void ConstructL( RConnection& aConnection, RSocketServ& aSocketServ );

	enum THttpState
		{
		EHttpNone, EHttpActive, EHttpFinished, EHttpNotified
		};

private:
	MHttpObserver* iObserver;

	HBufC8* iResponseData;
	CHttpDataEncoderBase* iOutputEncoder;

	RHTTPSession iSession;
	RHTTPTransaction iTransaction;
	THttpState iState;
	RWriteStream* iOutputStream;

	CHttpHeaders* iPersistentHeaders;
	};

#include "HttpController.inl"

#endif // HTTPCONTROLLER_H
