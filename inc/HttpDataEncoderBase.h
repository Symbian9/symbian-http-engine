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
* DISCLAIMED. IN NO EVENT SHALL PIOTR WACH, POLIDEA BE LIABLE FOR ANY
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
* HttpDataEncoderBase.h
* 
* Version: 
* 1.0
*/


#ifndef HTTPDATAENCODERBASE_H
#define HTTPDATAENCODERBASE_H

// INCLUDES
#include <MHTTPDataSupplier.h>

//this is named X class as this ia a M class that has destructor

class XHttpDataEncoderBase: public MHTTPDataSupplier
	{
public:
	virtual ~XHttpDataEncoderBase(){};
	virtual void ResetEncoderL() = 0;
	};

#endif // HTTPDATAENCODERBASE_H
