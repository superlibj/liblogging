/*! \file beepprofile.c
 *  \brief Implementation of the BEEP Profile Object.
 *
 * \author  Rainer Gerhards <rgerhards@adiscon.com>
 * \date    2003-08-04
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
#include "beepprofile.h"

/* ################################################################# *
 * private members                                                   *
 * ################################################################# */


/* ################################################################# *
 * public members                                                    *
 * ################################################################# */


char* sbProfGetURI(sbProfObj* pThis)
{
	sbProfCHECKVALIDOBJECT(pThis);
	return(pThis->pszProfileURI);
}


srRetVal sbProfConstruct(sbProfObj** ppThis, char *pszURI)
{
	char* pszURIBuf;
	int iURILen;

	assert(ppThis != NULL);

	/* try to allocate buffers and abort if not successful */
	if((*ppThis = calloc(1, sizeof(sbProfObj))) == NULL)
		return SR_RET_OUT_OF_MEMORY;

	if(pszURI == NULL)
		(*ppThis)->pszProfileURI = NULL;
	else
	{
		iURILen = (int) strlen(pszURI);
		if((pszURIBuf = malloc((iURILen + 1) * sizeof(char))) == NULL)
		{
			sbProfDestroy(*ppThis);
			return SR_RET_OUT_OF_MEMORY;
		}
		memcpy(pszURIBuf, pszURI, iURILen + 1);
		(*ppThis)->pszProfileURI = pszURIBuf;
	}

	/* OK, we got all buffers, now intialize the rest */
	(*ppThis)->OID = OIDsbProf;
	(*ppThis)->OnMesgRecv = NULL;
#	if FEATURE_LISTENER == 1
	(*ppThis)->pAPI = NULL;
	(*ppThis)->OnChanCreate = NULL;
	(*ppThis)->OnMesgRecv = NULL;
	(*ppThis)->bDestroyOnChanClose = FALSE;
#	endif

	return(SR_RET_OK);
}


void sbProfDestroy(sbProfObj* pThis)
{
	sbProfCHECKVALIDOBJECT(pThis);

	if(pThis->pszProfileURI != NULL)
		free(pThis->pszProfileURI);
	SRFREEOBJ(pThis);
}


#if FEATURE_LISTENER == 1

srRetVal sbProfSetAPIObj(sbProfObj *pThis, srAPIObj *pAPI)
{
	sbProfCHECKVALIDOBJECT(pThis);

	pThis->pAPI = pAPI;

	return SR_RET_OK;
}


srRetVal sbProfSetEventHandler(sbProfObj* pThis, sbProfEvent iEvent, srRetVal (*handler)())
{
	sbProfCHECKVALIDOBJECT(pThis);

	switch(iEvent)
	{
	case sbPROFEVENT_ONCHANCREAT:
		pThis->OnChanCreate = (void*) handler;
		break;
	case sbPROFEVENT_ONMESGRECV:
		pThis->OnMesgRecv = (void*) handler;
		break;
	default:
		return SR_RET_ERR;
	}
	return SR_RET_OK;
}


#endif /* #if FEATURE_LISTENER == 1 */
