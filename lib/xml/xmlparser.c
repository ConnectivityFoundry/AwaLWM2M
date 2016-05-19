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

/*! \file xmlparser.c
 *  \brief LibFlowCore .
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "xmlparser.h"

#define Flow_MemRealloc realloc
#define Flow_MemAlloc malloc
static inline void Flow_MemFree(void **buffer)
{
        if (*buffer)
        {
                free(*buffer);
                *buffer = NULL;
        }
}

static char *FlowString_DuplicateWithLength(const char *text, int textLength)
{
        char *result = NULL;
        if (text)
        {
                result = (char*)Flow_MemAlloc(textLength+1);
                if (result)
                {
                        memcpy(result, text, textLength);
                        result[textLength] = '\0';
                }
        }
        return result;
}

#define DEFAULT_DYNAMIC_STRING_BUFFER_SIZE      (128)
#define MAX_DYNAMIC_STRING_BUFFER_SIZE          (1024)
#define BAD_XML_CHAR(ch) ((ch) < ' ' && (ch) != '\n' && (ch) != '\r' && (ch) != '\t')


/*
 * Character History Buffer
 */
bool charhistoryBuffer_add(XMLParser_Context xmlParser, char newChar);
bool charhistoryBuffer_checkMatch (XMLParser_Context xmlParser, const char* target);
bool charhistoryBuffer_clear(XMLParser_Context xmlParser);
char* charhistoryBuffer_lookBack(XMLParser_Context xmlParser, unsigned int count);


/*
 * Dynamic String Buffer
 */
bool dynamicString_add(XMLParser_Context xmlParser, char newChar);
bool dynamicString_removelast(XMLParser_Context xmlParser, unsigned int count);
bool dynamicString_clear(XMLParser_Context xmlParser);
char* dynamicString_get(XMLParser_Context xmlParser);
unsigned int dynamicString_getLength(XMLParser_Context xmlParser);

/*
 * Attribute* array management
 */
const char** XMLParser_getAttributesArray(XMLParser_Context xmlParser);
bool XMLParser_DestroyAttributesArray(char** attrArray);

/*
 * Attribute list clean-up
 */
bool XMLParser_DestroyAttributeList(XMLParser_Context xmlParser);

/* Unescape incoming XML */
char XMLParser_unescape(XMLParser_Context xmlParser, char latestChar);


bool charhistoryBuffer_add(XMLParser_Context xmlParser, char newChar)
{
    bool result = false;
    if(xmlParser)
    {
        char tmp[CHARHISTORY_LENGTH];
        if(xmlParser->HistoryBuffLen > 0)
            memcpy(tmp, xmlParser->CharHistoryBuffer, xmlParser->HistoryBuffLen);

        memset(xmlParser->CharHistoryBuffer, '\0', CHARHISTORY_LENGTH);

        if(xmlParser->HistoryBuffLen == CHARHISTORY_LENGTH)
            memcpy(xmlParser->CharHistoryBuffer, tmp+1, xmlParser->HistoryBuffLen-1);
        else
            memcpy(xmlParser->CharHistoryBuffer, tmp, xmlParser->HistoryBuffLen);

        // Add new char to end of buffer
        if(xmlParser->HistoryBuffLen < CHARHISTORY_LENGTH)
            xmlParser->HistoryBuffLen++;
        xmlParser->CharHistoryBuffer[xmlParser->HistoryBuffLen-1] = newChar;

        result = true;
    }
    return result;
}

bool charhistoryBuffer_checkMatch(XMLParser_Context xmlParser, const char* target)
{
    bool result = false;
    if(xmlParser && target)
        result = strcmp (charhistoryBuffer_lookBack(xmlParser, strlen(target)), target ) == 0;
    return result;
}

bool charhistoryBuffer_clear(XMLParser_Context xmlParser)
{
    bool result = false;
    if(xmlParser) {
        memset(xmlParser->CharHistoryBuffer, '\0', CHARHISTORY_LENGTH+1);
        xmlParser->HistoryBuffLen = 0;
        result = true;
    }
    return result;
}

char* charhistoryBuffer_lookBack(XMLParser_Context xmlParser, unsigned int count)
{
    char *result = NULL;
    if(xmlParser && count > 0) {
        if(xmlParser->HistoryBuffLen >= count)
        {
            result = &xmlParser->CharHistoryBuffer[CHARHISTORY_LENGTH-count];               // '-1' because array indexing is 0-based
        }
    }
    return result;
}

bool dynamicString_add(XMLParser_Context xmlParser, char newChar)
{
    bool result = false;
    char* oldBuf = NULL;
    if(xmlParser)
    {
        if( xmlParser->DynamicStringUsed < xmlParser->DynamicStringSize)
        {
            xmlParser->DynamicString[xmlParser->DynamicStringUsed] = newChar;
            xmlParser->DynamicStringUsed++;
            result = true;
        }
        else
        {
            /* Not enough room in current dynamic string buffer, grow it */
            unsigned int newBuffSize = xmlParser->DynamicStringSize*2;

            /* Limit max size of dynamic string buffer */
            if(newBuffSize > MAX_DYNAMIC_STRING_BUFFER_SIZE)
                newBuffSize = MAX_DYNAMIC_STRING_BUFFER_SIZE;

            if(newBuffSize > xmlParser->DynamicStringSize)
            {
                /* Resize the buffer and copy contents (if there is room to grow) */
                oldBuf = xmlParser->DynamicString;
                xmlParser->DynamicString = Flow_MemAlloc(sizeof(char) * (newBuffSize + 1));
                if(xmlParser->DynamicString)
                {
                    memcpy(xmlParser->DynamicString, oldBuf, xmlParser->DynamicStringUsed * sizeof(char));
                    xmlParser->DynamicStringSize = newBuffSize;
                    xmlParser->DynamicString[xmlParser->DynamicStringUsed] = newChar;
                    xmlParser->DynamicStringUsed++;

                    Flow_MemFree((void **) &oldBuf);
                    result = true;
                }
            }
        }
    }
    return result;
}

bool dynamicString_clear(XMLParser_Context xmlParser)
{
    bool result = false;
    if(xmlParser) {
        xmlParser->DynamicStringUsed = 0;
        result = true;
    }
    return result;
}

char* dynamicString_get(XMLParser_Context xmlParser)
{
    char* result = NULL;
    if(xmlParser)
    {
        xmlParser->DynamicString[xmlParser->DynamicStringUsed] = '\0';
        result = xmlParser->DynamicString;
    }
    return result;
}

unsigned int dynamicString_getLength(XMLParser_Context xmlParser)
{
    unsigned int result = 0;
    if(xmlParser)
    {
        result = xmlParser->DynamicStringUsed;
    }
    return result;
}

bool dynamicString_removelast(XMLParser_Context xmlParser, unsigned int count)
{
    bool result = false;
    if(xmlParser)
    {
        if (xmlParser->DynamicStringUsed < count)
            xmlParser->DynamicStringUsed = 0;
        else
            xmlParser->DynamicStringUsed = xmlParser->DynamicStringUsed - count;
        result = true;
    }
    return result;
}



void runStartElementCallback(XMLParser_Context xmlParser)
{
    if(xmlParser)
    {
        xmlParser->State = XMLParserState_Running;

        char** attributes = (char**) XMLParser_getAttributesArray(xmlParser);

        if(xmlParser->StartHandler)
            xmlParser->StartHandler(xmlParser->UserData, xmlParser->CurrentElement.ElementName, (const char**) attributes);

        if(XMLParser_DestroyAttributesArray(attributes) )
        { /* No attributes were freed */ }

        // free attribute list
        XMLParser_DestroyAttributeList(xmlParser);
    }
}

XMLParser_Context XMLParser_Create (void)
{
    XMLParser_Context newParser = NULL;
    size_t parserSize = sizeof(XMLParser_ContextStruct);
    newParser = (XMLParser_Context) Flow_MemAlloc(parserSize);

    if(newParser)
    {
        memset(newParser,0, parserSize);

        bool initError = false;
        newParser->State = XMLParserState_Unknown;
        newParser->ErrorType = XMLParserErrorType_None;
        newParser->GotProlog = false;
        newParser->StartHandler = NULL;
        newParser->EndHandler = NULL;
        newParser->CharDataHandler = NULL;
        newParser->UserData = NULL;

        charhistoryBuffer_clear(newParser);

        /* Initialise dynamic string storage */
        newParser->DynamicStringSize = DEFAULT_DYNAMIC_STRING_BUFFER_SIZE;
        newParser->DynamicStringUsed = 0;
        newParser->DynamicString = (char*) Flow_MemAlloc(sizeof(char) * (newParser->DynamicStringSize + 1));
        if(!newParser->DynamicString)
            initError = true;
        // Initialise current element storage
        newParser->CurrentElement.ElementName = NULL;
        newParser->CurrentElement.AttributeCount = 0;
        newParser->CurrentElement.AttributeList = NULL;


        /* Error recovery */
        if(initError)
        {
            if(newParser->DynamicString)
            {
                Flow_MemFree((void **) &newParser->DynamicString);
            }

            if(newParser)
            {
                Flow_MemFree((void **) &newParser);
            }
        }
        else
        {
            newParser->State = XMLParserState_Init;
        }

    }
    else {
        Flow_MemFree((void **) &newParser);
    }

    return newParser;
}

bool XMLParser_Destroy (XMLParser_Context xmlParser)
{
    bool result = false;
    if(xmlParser) {

        xmlParser->State = XMLParserState_Unknown;

        if(xmlParser->DynamicString)
        {
            Flow_MemFree((void **) &xmlParser->DynamicString);
        }

        if(xmlParser->CurrentElement.ElementName)
        {
            Flow_MemFree((void **) &xmlParser->CurrentElement.ElementName);
        }

        XMLParser_DestroyAttributeList(xmlParser);

        Flow_MemFree((void **) &xmlParser);

        result = true;
    }
    return result;
}

bool XMLParser_IsFinished(XMLParser_Context xmlParser)
{
    bool result = false;
    if(xmlParser)
        result = xmlParser->State == XMLParserState_Done;
    return result;
}

const char** XMLParser_getAttributesArray(XMLParser_Context xmlParser)
{
    unsigned int arraySize = sizeof(char*) * (xmlParser->CurrentElement.AttributeCount*2 + 1);
    char** newArray = Flow_MemAlloc(arraySize);

    // The format of an attributes array is as follows:
    //  -- The attributes are stored in an array of char* where:
    //  -- the nth element is the 'key' (char*)
    //  -- the n+1 element is the 'value' (char*)
    //  -- the final element is NULL
    if(newArray)
    {
        memset(newArray, '\0', arraySize);

        XMLParser_attribute* attribute = xmlParser->CurrentElement.AttributeList;
        unsigned int attrIndex = 0;
        char** ptr = newArray;
        for(attrIndex = 0; attrIndex < xmlParser->CurrentElement.AttributeCount; attrIndex++)
        {
            *ptr = strdup(attribute->Name);
            ptr++;
            *ptr = strdup(attribute->Value);
            ptr++;
            attribute = attribute->Next;
        }
    }
    return (const char**) newArray;
}

bool XMLParser_Parse(XMLParser_Context xmlParser, const char *doc, unsigned int len, bool lastChunk)
{
    bool result = true;

    if(xmlParser != NULL &&
       doc != NULL &&
       len > 0)
    {
        //unsigned int chIndex = 0;
        xmlParser->DocIndex = 0;
        for(xmlParser->DocIndex=0; xmlParser->DocIndex<len; xmlParser->DocIndex++)
        {
            char ch = doc[xmlParser->DocIndex];         // Receive new char from doc

            //Todo add check
            if( !BAD_XML_CHAR(ch) )
            {

                if(!charhistoryBuffer_add(xmlParser, ch))
                {  /* Todo Do something on error if can't? */ }

                // FSM
                switch (xmlParser->State )
                {
                    /* Idle at present */
                    case XMLParserState_Init:
                    case XMLParserState_Idle:
                    case XMLParserState_Running:
                        {
                            if(ch == '<')
                            {
                                xmlParser->State = XMLParserState_StartTag;
                            }
                            else
                            {
                                if(xmlParser->State == XMLParserState_Running)
                                {
                                    xmlParser->State = XMLParserState_ElementData;
                                    if(!dynamicString_add(xmlParser, ch))
                                    { /* Todo error handling for when dynamic array can't grow */ }
                                }
                            }
                        }
                        break;


                    /* Received a start-tag */
                    case XMLParserState_StartTag:
                        {
                            if(ch == '?')
                            {
                                xmlParser->State = XMLParserState_Prolog;
                            }
                            else if(ch == '!' || ch == '-')
                            {
                                if(charhistoryBuffer_checkMatch(xmlParser, "<!--"))
                                    xmlParser->State = XMLParserState_Comment;
                            }
                            else if(ch == '/')
                            {
                                char emptyString = '\0';
                                if(xmlParser->CharDataHandler)
                                    xmlParser->CharDataHandler(xmlParser->UserData, &emptyString, 0);

                                if(charhistoryBuffer_checkMatch(xmlParser, "</"))
                                    xmlParser->State = XMLParserState_EndElement;
                            }
                            else
                            {
                                /* Must be a new element name */
                                xmlParser->State = XMLParserState_StartElement;
                                if (!dynamicString_add(xmlParser, ch) )
                                {
                                    // Todo error handling for when dynamic array can't grow
                                }
                            }
                        }
                        break;


                    /* Started processing a start-element tag */
                    case XMLParserState_StartElement:
                        {
                            if((ch != '>') && (ch != ' ') && (ch != '/'))
                            {
                                // Parsing element name
                                if (!dynamicString_add(xmlParser, ch) )
                                { /* Todo Handle error when building element name */ }
                            }
                            else
                            {
                                if((ch == '>') || (ch == ' '))
                                {
                                    if(!xmlParser->CurrentElement.ElementName)
                                    {
                                        /* Got the end of an element name; no attributes */

                                        if(!dynamicString_add(xmlParser, '\0'))                             // Null-terminate the name
                                        { /* Todo error handling for when dynamic array can't grow */ }

                                        if(xmlParser->CurrentElement.ElementName)
                                        {
                                            Flow_MemFree((void **) &xmlParser->CurrentElement.ElementName);
                                        }

                                        xmlParser->CurrentElement.ElementName = FlowString_DuplicateWithLength(dynamicString_get(xmlParser), dynamicString_getLength(xmlParser));

                                        dynamicString_clear(xmlParser);


                                        if(ch == ' ')
                                        {
                                            /* Got the end of an element name; expect attributes */
                                            xmlParser->State = XMLParserState_AttributeName;
                                        }
                                    }

                                    if(ch == '>')
                                    {
                                        runStartElementCallback(xmlParser);
                                        if(charhistoryBuffer_checkMatch(xmlParser, "/>"))
                                        {
                                            if(xmlParser->EndHandler)
                                                xmlParser->EndHandler(xmlParser->UserData, xmlParser->CurrentElement.ElementName);
                                        }
                                        if(xmlParser->CurrentElement.ElementName)
                                        {
                                            Flow_MemFree((void **) &xmlParser->CurrentElement.ElementName);
                                            xmlParser->CurrentElement.ElementName = NULL;
                                        }
                                        xmlParser->State = XMLParserState_Running;
                                    }

                                }


                            }

                        }
                        break;

                    /* Started processing an end element tag */
                    case XMLParserState_EndElement:
                    {
                        if(ch != '>')
                        {
                            // Parsing end-tag
                            if (!dynamicString_add(xmlParser, ch) )
                            { /* Todo Handle error when building element name */ }
                        }
                        else
                        {
                            if(!dynamicString_add(xmlParser, '\0'))
                            { /* Todo error handling for when dynamic array can't grow */ }

                            char* endTagName = FlowString_DuplicateWithLength(dynamicString_get(xmlParser), dynamicString_getLength(xmlParser));

                            dynamicString_clear(xmlParser);

                            if(xmlParser->EndHandler)
                                xmlParser->EndHandler(xmlParser->UserData, endTagName);

                            if(endTagName)
                            {
                                Flow_MemFree((void **) &endTagName);
                            }

                            xmlParser->State = XMLParserState_Idle;
                        }

                    }
                    break;

                    /* Started processing a element text data */
                    case XMLParserState_ElementData:
                        {
                            if(ch == '<')
                            {
                                xmlParser->State = XMLParserState_EndElementData;
                            }
                            else
                            {
                                /* Parsing element text */
                                ch = XMLParser_unescape(xmlParser, ch);
                                unsigned int textLength = dynamicString_getLength(xmlParser);
                                if (textLength >= DEFAULT_DYNAMIC_STRING_BUFFER_SIZE)
                                {
                                    char *position = strchr(xmlParser->CharHistoryBuffer,'&');
                                    if (!position)
                                    {
                                        if(xmlParser->CharDataHandler)
                                            xmlParser->CharDataHandler(xmlParser->UserData, dynamicString_get(xmlParser), textLength);
                                        dynamicString_clear(xmlParser);
                                    }
                                }
                                if (!dynamicString_add(xmlParser, ch) )
                                { /* Todo Handle error when building element text string */ }
                            }

                        }
                        break;

                    /* Finished processing element text data, decide what to do with it */
                    case XMLParserState_EndElementData:
                        {
                            // Fire the chardata callback if the next char is a '/' to indicate end of an element's text
                            if(ch == '/')
                            {
                                unsigned int textLength = dynamicString_getLength(xmlParser);
                                if(xmlParser->CharDataHandler)
                                    xmlParser->CharDataHandler(xmlParser->UserData, dynamicString_get(xmlParser), textLength);
                                dynamicString_clear(xmlParser);
                                xmlParser->State = XMLParserState_EndElement;
                            }
                            // otherwise throw it away as it is the start of a new element
                            else
                            {
                                dynamicString_clear(xmlParser);

                                if(ch == '?')
                                {
                                    xmlParser->State = XMLParserState_Prolog;
                                }
                                else if(ch == '!' || ch == '-')
                                {
                                    if(charhistoryBuffer_checkMatch(xmlParser, "<!--"))
                                        xmlParser->State = XMLParserState_Comment;
                                }
                                else
                                {
                                    /* Must be a new element name */
                                    xmlParser->State = XMLParserState_StartElement;
                                    if (!dynamicString_add(xmlParser, ch) )
                                    {
                                        // Todo error handling for when dynamic array can't grow
                                    }
                                }
                            }
                        }
                        break;

                    /* Started processing an attribute name */
                    case XMLParserState_AttributeName:
                        {
                            if(ch == '/')
                            {
                                /* Got the end of a start-element tag (self-closing) */

                                xmlParser->State = XMLParserState_AttributeValue;

                            }
                            else if(ch != '=')
                            {
                                if(!dynamicString_add(xmlParser, ch))
                                { /* Todo error handling for when dynamic array can't grow */ }
                            }
                            else
                            {
                                /* Got the end of an attribute name */

                                if(!dynamicString_add(xmlParser, '\0'))
                                { /* Todo error handling for when dynamic array can't grow */ }

                                if(xmlParser->CurrentElement._Attribute)
                                {
                                    Flow_MemFree((void **) &xmlParser->CurrentElement._Attribute);
                                }

                                xmlParser->CurrentElement._Attribute = Flow_MemAlloc(sizeof(XMLParser_attribute));
                                memset(xmlParser->CurrentElement._Attribute, '\0', sizeof(XMLParser_attribute));
                                xmlParser->CurrentElement._Attribute->AttributeDelimiter = '\0';

                                if(!xmlParser->CurrentElement._Attribute)
                                { /* Todo handle error when mallocing new attribute */ }
                                else
                                {
                                    xmlParser->CurrentElement._Attribute->Name = FlowString_DuplicateWithLength(dynamicString_get(xmlParser), dynamicString_getLength(xmlParser));

                                    dynamicString_clear(xmlParser);
                                }
                                xmlParser->State = XMLParserState_AttributeValue;
                            }
                        }
                        break;

                    /* Started processing an attribute value */
                    case XMLParserState_AttributeValue:
                        {

                            if(xmlParser->CurrentElement._Attribute)
                            {

                                /* Look for delimiter */
                                if(xmlParser->CurrentElement._Attribute->AttributeDelimiter == '\0')
                                {
                                    if((ch == '\'') || (ch == '\"'))
                                        xmlParser->CurrentElement._Attribute->AttributeDelimiter = ch;
                                }
                                else
                                {

                                    if( (ch != '>') && (ch != xmlParser->CurrentElement._Attribute->AttributeDelimiter) )
                                    {
                                        /* Parsing attribute value */

                                        ch = XMLParser_unescape(xmlParser, ch);

                                        if (!dynamicString_add(xmlParser, ch) )
                                        { /* Todo Handle error when building element name */ }

                                    }
                                    else
                                    {
                                        /* Got the end of an attribute value */

                                        if(!dynamicString_add(xmlParser, '\0'))
                                        { /* Todo error handling for when dynamic array can't grow */ }

                                        // End of attribute value found -- save it
                                        if(xmlParser->CurrentElement._Attribute)
                                        {
                                            xmlParser->CurrentElement._Attribute->Value = FlowString_DuplicateWithLength(dynamicString_get(xmlParser), dynamicString_getLength(xmlParser));

                                            dynamicString_clear(xmlParser);

                                            /* Find end of attribute list and add attribute... */

                                            if(xmlParser->CurrentElement.AttributeList)
                                            {
                                                XMLParser_attribute* end = xmlParser->CurrentElement.AttributeList;
                                                while(end->Next)
                                                    end = end->Next;

                                                xmlParser->CurrentElement._Attribute->Last = end;
                                                end->Next = xmlParser->CurrentElement._Attribute;
                                            }
                                            else {
                                                xmlParser->CurrentElement.AttributeList = xmlParser->CurrentElement._Attribute;
                                            }
                                            xmlParser->CurrentElement.AttributeCount++;
                                            xmlParser->CurrentElement._Attribute = NULL;
                                        }
                                    }
                                }
                            }
                            else
                            {
                                if(ch == ' ')
                                {
                                    /* Expecting another attribute name */
                                    xmlParser->State = XMLParserState_AttributeName;
                                }
                                else if(ch == '>')
                                {
                                    /* Got the end of a start-element tag */

                                    runStartElementCallback(xmlParser);
                                    if(charhistoryBuffer_checkMatch(xmlParser, "/>"))
                                    {
                                        if(xmlParser->EndHandler)
                                            xmlParser->EndHandler(xmlParser->UserData, xmlParser->CurrentElement.ElementName);
                                    }
                                    if(xmlParser->CurrentElement.ElementName)
                                    {
                                        Flow_MemFree((void **) &xmlParser->CurrentElement.ElementName);
                                    }
                                    xmlParser->State = XMLParserState_Running;
                                }

                            }


                        }
                        break;


                    /* Started processing a prolog */
                    case XMLParserState_Prolog:
                        {
                            if(ch == '>')
                            {
                                if(charhistoryBuffer_checkMatch(xmlParser, "?>"))
                                {
                                    xmlParser->State = XMLParserState_Running;
                                    xmlParser->GotProlog = true;
                                }
                            }
                        }
                        break;

                    /* Started processing a comment */
                    case XMLParserState_Comment:
                        {
                            if(ch == '>')
                            {
                                if(charhistoryBuffer_checkMatch(xmlParser, "-->"))
                                    xmlParser->State = XMLParserState_Running;
                            }
                        }
                        break;

                    /* Invalid state */
                    case XMLParserState_Unknown:
                    default:
                        {
                            xmlParser->State = XMLParserState_Unknown;
                        }
                        break;
                }

            }

        }

        // Finished parsing buffer. Check if we are done.
        if(lastChunk)
            xmlParser->State = XMLParserState_Done;
        else
        {
            if(xmlParser->EndOfChunkHandler)
                xmlParser->EndOfChunkHandler(xmlParser->UserData);
        }

    }
    return result;
}

bool XMLParser_SetCharDataHandler(XMLParser_Context myParser, XMLParser_CharacterDataHandler handler)
{
    bool result = false;
    if(myParser)
    {
        myParser->CharDataHandler = handler;
        result = true;
    }
    return result;
}

bool XMLParser_SetEndHandler(XMLParser_Context myParser, XMLParser_EndElementHandler handler)
{
    bool result = false;
    if(myParser)
    {
        myParser->EndHandler = handler;
        result = true;
    }
    return result;
}

bool XMLParser_SetStartHandler(XMLParser_Context myParser, XMLParser_StartElementHandler handler)
{
    bool result = false;
    if(myParser)
    {
        myParser->StartHandler = handler;
        result = true;
    }
    return result;
}

bool XMLParser_SetEndOfChunkHandler(XMLParser_Context myParser, XMLParser_EndOfChunkHandler handler)
{
    bool result = false;
    if(myParser)
    {
        myParser->EndOfChunkHandler = handler;
        result = true;
    }
    return result;
}

bool XMLParser_SetUserData(XMLParser_Context parser, void *userData)
{
    bool result = false;
    if(parser && userData) {
        parser->UserData = userData;
        result = true;
    }
    return result;
}

uint32_t XMLParser_getDocumentIndex(XMLParser_Context myParser)
{
    unsigned int result = 0;
    if (myParser)
        result = myParser->DocIndex;
    return result;
}


bool XMLParser_DestroyAttributesArray(char** attrArray)
{
    bool result = false;
    if(attrArray)
    {
        char** ptr = attrArray;
        while(*ptr)
        {
            Flow_MemFree((void **) ptr);
            ptr++;
        }

        Flow_MemFree((void **) &attrArray);
        result = true;
    }
    return result;
}

bool XMLParser_DestroyAttributeList(XMLParser_Context xmlParser)
{
    bool result = false;
    if(xmlParser)
    {
        if(xmlParser->CurrentElement.AttributeList)
        {
            XMLParser_attribute* end = xmlParser->CurrentElement.AttributeList;

            while(end->Next)
                end = end->Next;

            while(end)
            {
                Flow_MemFree((void **) &end->Name);
                Flow_MemFree((void **) &end->Value);

                if(end != xmlParser->CurrentElement.AttributeList)
                {
                    end = end->Last;
                    Flow_MemFree((void *) &end->Next);
                    xmlParser->CurrentElement.AttributeCount--;
                }
                else
                {
                    Flow_MemFree((void *) &xmlParser->CurrentElement.AttributeList);
                    xmlParser->CurrentElement.AttributeCount = 0;
                    end = NULL;

                    result = true;
                }

            }

        }
    }
    return result;
}


char XMLParser_unescape(XMLParser_Context xmlParser, char latestChar)
{
    char result = latestChar;
    if(xmlParser && latestChar == ';')
    {
        if( charhistoryBuffer_checkMatch(xmlParser, "&lt;"))
        {
            if (dynamicString_removelast(xmlParser, strlen("&lt;")-1))              // NB: -1 because the semicolon hasn't been added to the string yet
                result = '<';
        }
        else if( charhistoryBuffer_checkMatch(xmlParser, "&gt;"))
        {
            if (dynamicString_removelast(xmlParser, strlen("&gt;")-1))
                result = '>';
        }
        else if( charhistoryBuffer_checkMatch(xmlParser, "&quot;"))
        {
            if (dynamicString_removelast(xmlParser, strlen("&quot;")-1))
                result = '\"';
        }
        else if( charhistoryBuffer_checkMatch(xmlParser, "&apos;"))
        {
            if (dynamicString_removelast(xmlParser, strlen("&apos;")-1))
                result = '\'';
        }
        else if( charhistoryBuffer_checkMatch(xmlParser, "&amp;"))
        {
            if (dynamicString_removelast(xmlParser, strlen("&amp;")-1))
                result = '&';
        }
    }
    return result;
}
