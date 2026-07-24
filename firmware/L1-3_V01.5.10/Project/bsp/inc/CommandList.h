
#ifndef  __COMMANDLIST_H__
#define  __COMMANDLIST_H__

#include "SerialPort.h"

typedef struct 
{
	const char     *cmdStr;
          char     (*func)(void *p,...);	
	const char     *rChar;	
}cmdStructure;

//*******************************************************************************************//
/*
 * `cmdMessage` is defined in CommandList.c to avoid multiple-definition
 * and large initialized-data duplication when this header is included
 * across multiple translation units. Use `extern` to reference it.
 */
extern const cmdStructure cmdMessage[];
extern const int cmdStrLen;



#endif

