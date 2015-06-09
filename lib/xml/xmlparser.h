/************************************************************************************************************************
 Copyright (c) 2016, Imagination Technologies Limited and/or its affiliated group companies.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
 following conditions are met:
     1. Redistributions of source code must retain the above copyright notice, this list of conditions and the
        following disclaimer.
     2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
        following disclaimer in the documentation and/or other materials provided with the distribution.
     3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote
        products derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE 
 USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
************************************************************************************************************************/

/*! \file xmlparser.h
 *  \brief LibFlowCore .
 */

#ifndef FLOWXMLPARSER_H_
#define FLOWXMLPARSER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#if 0
#include "flow/core/base_types.h"
#endif

typedef enum
{
    XMLParserState_Error = -1,
    XMLParserState_Unknown = 0,
    XMLParserState_Init,                    // Indicates the parser is ready (initialised), but hasn't started running
    XMLParserState_Idle,                    // Indicates the parser is not parsing an element
    XMLParserState_Running,                 // Indicates parser is running through an element (has started)
    XMLParserState_StartTag,                // Indicates we have received the start character '<' of a tag
    XMLParserState_Prolog,                  // Indicates we are currently processing a prolog
    XMLParserState_StartElement,            // Indicates we got a '<' and the next char was not '?' to indicate prolog
    XMLParserState_ParsingElement,          // Indicates we got a start-element tag already
    XMLParserState_AttributeName,           // Indicates we are currently processing the name of an attribute
    XMLParserState_AttributeValue,          // Indicates we are currently processing the value of an attribute
    XMLParserState_ElementData,
    XMLParserState_EndElementData,
    XMLParserState_EndElement,
    XMLParserState_Comment,
    XMLParserState_Done,

    XMLParserState__Max
} XMLParserState;


typedef enum
{
    XMLParserErrorType_Unknown = -1,
    XMLParserErrorType_None = 0,

    XMLParserErrorType__Max
} XMLParserErrorType;

typedef struct XMLParser_attr
{
    char                            *Name;
    char                            *Value;
    struct XMLParser_attr           *Next;
    struct XMLParser_attr           *Last;

    char                            AttributeDelimiter;                     // Used for keeping track of the type of delimiter used for this attribute value

} XMLParser_attribute;

typedef struct
{
    char                            *ElementName;

    XMLParser_attribute             *_Attribute;                                // Temporary internal storage for attribute under construnction

    XMLParser_attribute             *AttributeList;
    unsigned int                    AttributeCount;

} XMLParser_elementInfo;


#define CHARHISTORY_LENGTH          (6)

/* SAX-parser callback function pointer types*/
typedef void (*XMLParser_StartElementHandler) (void *userData, const char *name, const char **atts);
typedef void (*XMLParser_EndElementHandler) (void *userData, const char *name);
typedef void (*XMLParser_CharacterDataHandler) (void *userData, const char *s, int len);
typedef void (*XMLParser_EndOfChunkHandler) (void *userData);

typedef struct
{
    void                            *UserData;
    XMLParserState                  State;
    XMLParserErrorType              ErrorType;
    bool                            GotProlog;

    /* User Callbacks */
    XMLParser_StartElementHandler   StartHandler;
    XMLParser_CharacterDataHandler  CharDataHandler;
    XMLParser_EndElementHandler     EndHandler;
    XMLParser_EndOfChunkHandler     EndOfChunkHandler;

    /* Current Element information */
    XMLParser_elementInfo           CurrentElement;

    /* Dynamic String for general purpose storage */
    char                            *DynamicString;
    unsigned int                    DynamicStringSize;
    unsigned int                    DynamicStringUsed;

    /* Character history */
    char                            CharHistoryBuffer[CHARHISTORY_LENGTH+1];
    int                             HistoryBuffLen;

    uint32_t                    DocIndex;
} XMLParser_ContextStruct;

typedef XMLParser_ContextStruct *XMLParser_Context;


/* XML parser APIs */
XMLParser_Context XMLParser_Create (void);
bool XMLParser_Destroy (XMLParser_Context context);
bool XMLParser_IsFinished(XMLParser_Context xmlParser);
bool XMLParser_Parse (XMLParser_Context xmlParser, const char *doc, unsigned int len, bool lastChunk);
bool XMLParser_SetCharDataHandler(XMLParser_Context myParser, XMLParser_CharacterDataHandler handler);
bool XMLParser_SetEndHandler(XMLParser_Context myParser, XMLParser_EndElementHandler handler);
bool XMLParser_SetUserData(XMLParser_Context myParser, void *userData);
bool XMLParser_SetStartHandler(XMLParser_Context myParser, XMLParser_StartElementHandler handler);
bool XMLParser_SetEndOfChunkHandler(XMLParser_Context myParser, XMLParser_EndOfChunkHandler handler);


uint32_t XMLParser_getDocumentIndex(XMLParser_Context myParser);

#ifdef __cplusplus
}
#endif

#endif /* FLOWXMLPARSER_H_ */

