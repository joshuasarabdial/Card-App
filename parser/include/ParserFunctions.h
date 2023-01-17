/**
 * @file ParserFunctions.h
 * @author Joshua Sarabdial
 * @date October 2018
 **/

#ifndef _PARSERFUNCTIONS_H
#define _PARSERFUNCTIONS_H

#include <ctype.h>
#include "VCardParser.h"

#define BUFFERSIZE 1024
#define TRUE 1
#define FALSE 0

//*****************************************************************
VCardErrorCode readProperty(FILE* fp, char** line);

VCardErrorCode divideProperty(char* line, char** groupName, char** propertyName, char** parameterValue, char** propertyValue);

VCardErrorCode createProperty(Property** newProperty);

VCardErrorCode createParameter(Parameter** newParameter, char* theValue);

VCardErrorCode createDateTime(DateTime** newDateTime, char* parameterValue, char* propertyValue);

VCardErrorCode addParams(Property* theProperty, char* parameterValues);

VCardErrorCode addValues(Property* theProperty, char* propertyValues);

char* duplicateString(const char* theString);

int replaceString(char** toReplace, char* toCopy);

char* strTokenizer(char* str, char delimiter);

int stricasecmp(const char* s1, const char* s2);
//*****************************************************************

#endif
