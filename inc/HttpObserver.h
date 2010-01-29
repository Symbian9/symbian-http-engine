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
* DISCLAIMED. IN NO EVENT SHALL PAWE£ POLAÑSKI BE LIABLE FOR ANY
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
* HttpObserver.h - is handling an observer for http HttpController responces
* 
* Version: 
* 1.0
*/

#ifndef HTTPOBSERVER_H_
#define HTTPOBSERVER_H_

class CHttpHeaders;

class MHttpObserver
	{
public:
	virtual void TransactionSucceeded() = 0;
	virtual void StreamReceived() = 0;
	virtual void Error( TInt aError ) = 0;
	virtual void
			HeadersReceivedL( CHttpHeaders* aHeaders ) = 0;//gets ownership
	virtual void ContentReceived( HBufC8* aData ) = 0;
	};

#endif /* HTTPOBSERVER_H_ */
