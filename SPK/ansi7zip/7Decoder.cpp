#include "7Decoder.h"
#include "LzmaDecode.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

unsigned char *LZMADecode_C ( unsigned char *inBuffer, size_t inSize, size_t *outSizeProcessed, int *result )
{
	const UInt32 kPropertiesSize = 5;
	Byte *properties;

    size_t inProcessed;
    CLzmaDecoderState state; 
//    int result;
	size_t outSizeProcessedLoc;
	size_t outSize = 0;
	Byte *outBuffer = NULL;
	int i = 0;

	properties = (Byte *)malloc(sizeof(Byte *) * kPropertiesSize);
	*outSizeProcessed = 0;

	// read properties
	memcpy ( properties, inBuffer, 5 );
	inSize -= 5;
	inBuffer += 5;

	// read size
	for ( i = 0; i < 8; i++ )
	{
		Byte b = inBuffer[0];
		inBuffer++;
		inSize--;

		outSize |= ((size_t)b) << (8 * i);
	}

	outBuffer = (Byte *)malloc ( sizeof(Byte) * outSize );


    if ( LzmaDecodeProperties ( &state.Properties, properties, kPropertiesSize ) != LZMA_RESULT_OK )
	{
		if ( result )
			*result = SZE_FAIL;
		return NULL;
	}

    state.Probs = (CProb *)malloc ( LzmaGetNumProbs ( &state.Properties ) * sizeof (CProb) );
    if (state.Probs == 0)
	{
		if ( result )
			*result = SZE_OUTOFMEMORY;
		return NULL;
	}

    int r = LzmaDecode(&state,
        inBuffer, inSize, &inProcessed,
        outBuffer, outSize, &outSizeProcessedLoc);

    *outSizeProcessed = (size_t)outSizeProcessedLoc;
	free ( state.Probs );

    if ( r == LZMA_RESULT_DATA_ERROR )
	{
		if ( result )
			*result = SZE_DATA_ERROR;
		return NULL;
	}
    if ( r != LZMA_RESULT_OK )
	{
		if ( result )
			*result = SZE_FAIL;
		return NULL;
	}

	if ( result )
		*result = SZ_OK;
	return outBuffer;
}
