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
 * HttpEngine
 * 
 * Version: 
 * 1.0
 */
#include "HttpController.h"
#include <httpstringconstants.h>
#include <rhttpheaders.h>
#include <S32STRM.H>
#include "HttpObserver.h"
#include "MultipartEncoder.h"
#include "HttpHeader.h"

_LIT8( KUserAgent, "SymbianOS" );
_LIT8( KContentType, "multipart/form-data; boundary=" KBoundryId );
_LIT8( KUrlEncodedContentType, "application/x-www-form-urlencoded" );
_LIT8( KAccept, "*/*" );
_LIT8( KClose, "Close" );

const TInt KMaxMemoryBuffer = 0x10000;

CHttpController::CHttpController(CHttpDataEncoderBase* aContentEncoder) :
	iOutputEncoder(aContentEncoder)
	{
	}

CHttpController::~CHttpController()
	{
	delete iResponseData;
	delete iOutputEncoder;
	if (iOutputStream)
		{
		iOutputStream->Close();
		iOutputStream = NULL;
		}
	Cancel();
	iSession.Close();
	delete iPersistentHeaders;
	}

CHttpController* CHttpController::NewLC(RConnection& aConnection,
		RSocketServ& aSocketServ, CHttpDataEncoderBase* aContentEncoder)
	{
	CHttpController* self = new (ELeave) CHttpController(aContentEncoder);
	CleanupStack::PushL(self);
	self->ConstructL(aConnection, aSocketServ);
	return self;
	}

CHttpController* CHttpController::NewL(RConnection& aConnection,
		RSocketServ& aSocketServ, CHttpDataEncoderBase* aContentEncoder)
	{
	CHttpController* self = CHttpController::NewLC(aConnection, aSocketServ,
			aContentEncoder);
	CleanupStack::Pop(self);
	return self;
	}

void CHttpController::ConstructL(RConnection& aConnection,
		RSocketServ& aSocketServ)
	{
	iPersistentHeaders = CHttpHeaders::NewL();
	iSession.OpenL();
	RHTTPConnectionInfo connectionInfo = iSession.ConnectionInfo();
	RStringPool pool = iSession.StringPool();

	connectionInfo.SetPropertyL(pool.StringF(HTTP::EHttpSocketServ,
			RHTTPSession::GetTable()), THTTPHdrVal(aSocketServ.Handle()));

	TInt connectionPtr = REINTERPRET_CAST(TInt, &aConnection);
	connectionInfo.SetPropertyL(pool.StringF(HTTP::EHttpSocketConnection,
			RHTTPSession::GetTable()), THTTPHdrVal(connectionPtr));
	iSession.FilterCollection().RemoveFilter(iSession.StringPool().StringF(
			HTTP::ERedirect, RHTTPSession::GetTable()));
	if (!iOutputEncoder)
		{//default content encoder
		iOutputEncoder = CMultipartEncoder::NewL();
		}
	}

void CHttpController::SetHeaderL(RHTTPHeaders& aHeaders, TInt aHdrField,
		const TDesC8& aHdrValue)
	{
	RStringF valStr = iSession.StringPool().OpenFStringL(aHdrValue);
	CleanupClosePushL(valStr);
	THTTPHdrVal val(valStr);
	aHeaders.SetFieldL(iSession.StringPool().StringF(aHdrField,
			RHTTPSession::GetTable()), val);
	CleanupStack::PopAndDestroy(&valStr);
	}

void CHttpController::SetHeaderL(RHTTPHeaders& aHeaders,
		const TDesC8& aHdrName, const TDesC8& aHdrValue)
	{
	RStringF nameStr = iSession.StringPool().OpenFStringL(aHdrName);
	CleanupClosePushL(nameStr);
	RStringF valStr = iSession.StringPool().OpenFStringL(aHdrValue);
	CleanupClosePushL(valStr);
	THTTPHdrVal val(valStr);
	aHeaders.SetFieldL(nameStr, val);
	CleanupStack::PopAndDestroy(2, &nameStr);
	}

void CHttpController::Error(TInt aError)
	{
	delete iResponseData;
	iResponseData = NULL;
	if (iOutputStream)
		{
		iOutputStream->Close();
		iOutputStream = NULL;
		}
	if (iState < EHttpNotified)
		{
		iObserver->Error(aError);
		iState = EHttpNotified;
		}
	}

void CHttpController::CloseTransaction(RHTTPTransaction& aTransaction)
	{
	aTransaction.Close();
	delete iResponseData;
	iResponseData = NULL;
	if (iOutputStream)
		{
		iOutputStream->Close();
		iOutputStream = NULL;
		}
	iState = EHttpFinished;
	}

void CHttpController::SendRequestL(TInt aMethodIndex, const TDesC8& aUri)
	{
	SendRequestL(aMethodIndex, aUri, NULL );
	}

void CHttpController::SendRequestL(TInt aMethodIndex, const TDesC8& aUri,
		CHttpHeaders* aHeaders)
	{
	if (!iObserver)
		{
		User::Leave(KErrHttpInvalidObserver);
		}
	RStringF method = iSession.StringPool().StringF(aMethodIndex,
			RHTTPSession::GetTable());

	TUriParser8 uri;
	uri.Parse(aUri);
	iTransaction = iSession.OpenTransactionL(uri, *this, method);
	RHTTPHeaders hdr = iTransaction.Request().GetHeaderCollection();

	RArray<TInt> addedElements;
	CleanupClosePushL(addedElements);

	HBufC8* headerName = HeaderNameLC(HTTP::EUserAgent);

	TInt error = iPersistentHeaders->Find(*headerName);
	SetHeaderL(hdr, *headerName, error >= 0 ? iPersistentHeaders->Value(error)
			: TPtrC8(KUserAgent));
	CleanupStack::PopAndDestroy(headerName);
	if (error >= 0)
		{
		addedElements.InsertInOrderL(error);
		}

	headerName = HeaderNameLC(HTTP::EConnection);
	error = iPersistentHeaders->Find(*headerName);
	SetHeaderL(hdr, *headerName, error >= 0 ? iPersistentHeaders->Value(error)
			: TPtrC8(KClose));
	CleanupStack::PopAndDestroy(headerName);
	if (error >= 0)
		{
		addedElements.InsertInOrderL(error);
		}

	headerName = HeaderNameLC(HTTP::EAccept);
	error = iPersistentHeaders->Find(*headerName);
	SetHeaderL(hdr, *headerName, error >= 0 ? iPersistentHeaders->Value(error)
			: TPtrC8(KAccept));
	CleanupStack::PopAndDestroy(headerName);
	if (error >= 0)
		{
		addedElements.InsertInOrderL(error);
		}

	headerName = HeaderNameLC(HTTP::EHost);
	error = iPersistentHeaders->Find(*headerName);
	SetHeaderL(hdr, *headerName, error >= 0 ? iPersistentHeaders->Value(error)
			: TPtrC8(uri.Extract(EUriHost)));
	CleanupStack::PopAndDestroy(headerName);
	if (error >= 0)
		{
		addedElements.InsertInOrderL(error);
		}

		{
		TInt count(iPersistentHeaders->Count());
		for (TInt i(0); i < count; ++i)
			{
			if (addedElements.FindInOrder(i) >= 0)
				{
				continue;
				}
			SetHeaderL(hdr, iPersistentHeaders->Key(i),
					iPersistentHeaders->Value(i));
			}
		}
	CleanupStack::PopAndDestroy(&addedElements);
	if (aHeaders)
		{
		TInt count(aHeaders->Count());
		for (TInt i(0); i < count; ++i)
			{
			SetHeaderL(hdr, aHeaders->Key(i), aHeaders->Value(i));
			}
		}

	if (aMethodIndex == HTTP::EPOST)
		{
		TBuf8<100> contentLength;
		contentLength.Num(iOutputEncoder->OverallDataSize());
		SetHeaderL(hdr, HTTP::EContentLength, contentLength);
		SetHeaderL(hdr, HTTP::EContentType, KUrlEncodedContentType);
		iTransaction.Request().SetBody(*iOutputEncoder);
		}
	iTransaction.SubmitL();
	iState = EHttpActive;
	}

void CHttpController::GetL(const TDesC8& aUri, CHttpHeaders* aHeaders,
		RWriteStream* aBodyFile)
	{
	Cancel();
	if (iOutputStream)
		{
		iOutputStream->Close();
		}
	iOutputStream = aBodyFile;
	SendRequestL(HTTP::EGET, aUri, aHeaders);
	}

void CHttpController::PostL(const TDesC8& aUri, CHttpHeaders* aHeaders,
		RWriteStream* aBodyFile)
	{
	Cancel();
	if (iOutputStream)
		{
		iOutputStream->Close();
		}
	iOutputStream = aBodyFile;
	SendRequestL(HTTP::EPOST, aUri, aHeaders);
	}

void CHttpController::AddPersistentHeaderL(TInt aHeaderId, const TDesC8& aValue)
	{
	RStringF string = iSession.StringPool().StringF(aHeaderId,
			RHTTPSession::GetTable());
	CleanupClosePushL(string);
	iPersistentHeaders->AddL(string.DesC(), aValue);
	CleanupStack::PopAndDestroy(&string);
	}

void CHttpController::ResetPersistentHeaders()
	{
	iPersistentHeaders->Reset();
	}

void CHttpController::ParseHeadersL(RHTTPTransaction& aTransaction)
	{
	const TInt KMaxNumericLen = 32;
	const TInt KMaxDateLen = 50;

	RStringPool stringPool = aTransaction.Session().StringPool();
	RHTTPHeaders header = aTransaction.Response().GetHeaderCollection();
	THTTPHdrFieldIter iterator = header.Fields();

	HBufC8* fieldName8 = NULL;
	HBufC8* fieldVal8 = NULL;
	CHttpHeaders* headers = CHttpHeaders::NewLC();
	while (!iterator.AtEnd())
		{
		RStringTokenF fieldName = iterator();
		RStringF fieldNameStr = stringPool.StringF(fieldName);
		THTTPHdrVal fieldVal;
		if (header.GetField(fieldNameStr, 0, fieldVal) == KErrNone)
			{
			fieldName8 = fieldNameStr.DesC().AllocLC();
			switch (fieldVal.Type())
				{
				case THTTPHdrVal::KTIntVal:
					{
					fieldVal8 = HBufC8::NewLC(KMaxNumericLen);
					TPtr8 ptr(fieldVal8->Des());
					ptr.Num(fieldVal.Int());
					break;
					}
				case THTTPHdrVal::KStrFVal:
					{
					RStringF fieldValStr = stringPool.StringF(fieldVal.StrF());
					fieldVal8 = fieldValStr.DesC().AllocLC();
					break;
					}
				case THTTPHdrVal::KStrVal:
					{
					RString fieldValStr = stringPool.String(fieldVal.Str());
					fieldVal8 = fieldValStr.DesC().AllocLC();
					break;
					}
				default:
					break;
				}
			if (!fieldVal8)
				{
				CleanupStack::PopAndDestroy(fieldName8);
				}
			else
				{
				headers->AddL(*fieldName8, *fieldVal8);
				CleanupStack::PopAndDestroy(2, fieldName8);
				}
			fieldVal8 = NULL;
			fieldName8 = NULL;
			}
		++iterator;
		}

	iObserver->HeadersReceivedL(headers);

	CleanupStack::Pop(headers);
	}

TInt CHttpController::ContentLength(RHTTPResponse& aResponse,
		RHTTPSession& aSession)
	{
	RStringF contetnLength = aSession.StringPool().StringF(
			HTTP::EContentLength, RHTTPSession::GetTable());
	RHTTPHeaders responseHeaders = aResponse.GetHeaderCollection();
	THTTPHdrVal contetnLengthValue;
	TInt error(responseHeaders.GetField(contetnLength, 0, contetnLengthValue));
	if (error == KErrNone)
		{
		return contetnLengthValue.Int();
		}
	return error;
	}

void CHttpController::MHFRunL(RHTTPTransaction aTransaction,
		const THTTPEvent& aEvent)
	{
	if (iState >= EHttpFinished)
		{
		if (iState == EHttpFinished)
			{
			CloseTransaction(aTransaction);
			Error(KErrCancel);
			}
		return;
		}

	switch (aEvent.iStatus)
		{
		case THTTPEvent::EGotResponseHeaders:
			{
			RHTTPResponse responce = aTransaction.Response();
			TInt httpStatus(responce.StatusCode());//text add
			if (httpStatus / 100 != 2) //responce 200 - 299
				{
				MHFRunError(KHttpErrorBase - httpStatus, aTransaction, aEvent);
				return;
				}
			ParseHeadersL(aTransaction);
			if (!iOutputStream)
				{
				delete iResponseData;
				iResponseData = NULL;
				TInt len = ContentLength(responce, iSession);
				iResponseData = HBufC8::NewL(len > 0 ? len : KMaxMemoryBuffer);
				}
			break;
			}
		case THTTPEvent::EGotResponseBodyData:
			{
			MHTTPDataSupplier* body = aTransaction.Response().Body();

			TPtrC8 dataPart;
			TBool isLastPart = body->GetNextDataPart(dataPart);

			if (iOutputStream)
				{
				iOutputStream->WriteL(dataPart);
				}
			else
				{
				TInt bufLength(iResponseData->Des().Length());
				TInt maxBufLength(iResponseData->Des().MaxLength());
				if (bufLength + dataPart.Length() > maxBufLength)
					{
					iResponseData = iResponseData->ReAllocL(bufLength
							+ dataPart.Length());
					}
				iResponseData->Des().Append(dataPart);
				}
			body->ReleaseData();
			break;
			}
		case THTTPEvent::EResponseComplete:
			{
			TInt httpStatus(aTransaction.Response().StatusCode());
			if (httpStatus / 100 != 2)
				{
				MHFRunError(KHttpErrorBase - httpStatus, aTransaction, aEvent);
				return;
				}
			if (iOutputStream)
				{

				}
			iState = EHttpFinished;
			aTransaction.Close();
			if (iOutputStream)
				{
				iOutputStream->CommitL();
				iOutputStream->Close();
				iOutputStream = NULL;
				iObserver->StreamReceived();
				}
			else
				{
				iObserver->ContentReceived(iResponseData);
				iResponseData = NULL;
				}
			delete iResponseData;
			iResponseData = NULL;
			iObserver->TransactionSucceeded();
			iState = EHttpNotified;
			break;
			}
		case THTTPEvent::ESucceeded:
			break;
		case THTTPEvent::EFailed:
			{
			CloseTransaction(aTransaction);
			iObserver->Error(KErrGeneral);
			iState = EHttpNotified;
			break;
			}
		default:
			{
			if (aEvent.iStatus < 0)
				{
				CloseTransaction(aTransaction);
				iObserver->Error(aEvent.iStatus);
				iState = EHttpNotified;
				}
			}
			break;
		}
	}

TInt CHttpController::MHFRunError(TInt aError, RHTTPTransaction aTransaction,
		const THTTPEvent& /*aEvent*/)
	{
	CloseTransaction(aTransaction);
	Error(aError);
	return KErrNone;
	}

void CHttpController::Cancel()
	{
	if (iState > EHttpFinished)
		{
		iState = EHttpFinished;
		}
	else
		{
		if ( iState == EHttpActive )
			{
			CloseTransaction(iTransaction);
			}
		}
	}

HBufC8* CHttpController::HeaderNameLC(TInt aId)
	{
	RStringF string = iSession.StringPool().StringF(aId,
			RHTTPSession::GetTable());
	CleanupClosePushL(string);
	HBufC8* buffer = string.DesC().AllocL();
	CleanupStack::PopAndDestroy(&string);
	CleanupDeletePushL(buffer);
	return buffer;
	}
