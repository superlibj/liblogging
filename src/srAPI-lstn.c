/*! \file srAPI-lstn.c
 *  \brief Implementation of the listener part of the API
 *
 * \author  Rainer Gerhards <rgerhards@adiscon.com>
 * \date    2003-08-26
 *          begun coding on initial version.
 *
 * Copyright 2002-2003 
 *     Rainer Gerhards and Adiscon GmbH. All Rights Reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 * 
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 * 
 *     * Neither the name of Adiscon GmbH or Rainer Gerhards
 *       nor the names of its contributors may be used to
 *       endorse or promote products derived from this software without
 *       specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <assert.h>
#include "config.h"
#include "liblogging.h"
#include "beepsession.h"
#include "beepprofile.h"
#include "beeplisten.h"
#include "lstnprof-3195raw.h"
#include "srAPI.h"
#include "syslogmessage.h"

/* ################################################################# *
 * private members                                                   *
 * ################################################################# */


/* ################################################################# *
 * public members                                                    *
 * ################################################################# */

srRetVal srAPISetMsgRcvCallback(srAPIObj* pThis, void(*NewHandler)(srAPIObj*, srSLMGObj*))
{
	if((pThis == NULL) || (pThis->OID != OIDsrAPI))
		return SR_RET_INVALID_HANDLE;

	pThis->OnSyslogMessageRcvd = NewHandler;

	return SR_RET_OK;
}


srRetVal srAPISetupListener(srAPIObj* pThis, void(*NewHandler)(srAPIObj*, srSLMGObj*))
{
	srRetVal iRet;
	sbProfObj *pProf;

	if((pThis == NULL) || (pThis->OID != OIDsrAPI))
		return SR_RET_INVALID_HANDLE;

	if(pThis->pLstn != NULL)
		return SR_RET_ALREADY_LISTENING;

	if((iRet = sbLstnInit(&(pThis->pLstn))) != SR_RET_OK)
		return iRet;

	/* set up the rfc 3195/raw listener */
	if((iRet = sbProfConstruct(&pProf, "http://xml.resource.org/profiles/syslog/RAW")) != SR_RET_OK)
	{
		sbLstnDestroy(pThis->pLstn);
		return iRet;
	}

	if((iRet = sbProfSetAPIObj(pProf, pThis)) != SR_RET_OK)
	{
		sbLstnDestroy(pThis->pLstn);
		sbProfDestroy(pProf);
		return iRet;
	}

	if((iRet = srAPISetMsgRcvCallback(pThis, NewHandler)) != SR_RET_OK)
	{
		sbLstnDestroy(pThis->pLstn);
		sbProfDestroy(pProf);
		return iRet;
	}

	if((iRet = sbProfSetEventHandler(pProf, sbPROFEVENT_ONCHANCREAT, (void*) psrrOnChanCreate)) != SR_RET_OK)
	{
		sbLstnDestroy(pThis->pLstn);
		sbProfDestroy(pProf);
		return iRet;
	}

	if((iRet = sbProfSetEventHandler(pProf, sbPROFEVENT_ONMESGRECV, (void*) psrrOnMesgRecv)) != SR_RET_OK)
	{
		sbLstnDestroy(pThis->pLstn);
		sbProfDestroy(pProf);
		return iRet;
	}

	if((iRet = sbLstnAddProfile(pThis->pLstn, pProf)) != SR_RET_OK)
	{
		sbLstnDestroy(pThis->pLstn);
		sbProfDestroy(pProf);
		return iRet;
	}

	return SR_RET_OK;
}


srRetVal srAPIRunListener(srAPIObj *pThis)
{
	if((pThis == NULL) || (pThis->OID != OIDsrAPI))
		return SR_RET_INVALID_HANDLE;

	return sbLstnRun(pThis->pLstn);
}


srRetVal srAPIShutdownListener(srAPIObj *pThis)
{
	if((pThis == NULL) || (pThis->OID != OIDsrAPI))
		return SR_RET_INVALID_HANDLE;
	
	pThis->pLstn->bRun = FALSE;

	return SR_RET_OK;
}