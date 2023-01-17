/**
 * @file VCardParser.c
 * @author Joshua Sarabdial
 * @date September 2018
 **/

#include "VCardParser.h"
#include "ParserFunctions.h"

VCardErrorCode createCard(char* fileName, Card** newCardObject) {
    VCardErrorCode theError = OK;
    FILE* fp = NULL;
    char* line = NULL;
    bool isFirstLine = true;
    bool isVersionFour = false;
    bool isEnd = false;

    // Pointers to contentline
    char* groupName = NULL;
    char* propertyName = NULL;
    char* parameterValues = NULL;
    char* propertyValues = NULL;

    // Generic data pointers (to be used)
    Property* aProperty = NULL;
    DateTime* aDateTime = NULL;

    // Check file name validity
    if (!(fileName)) 
        return INV_FILE;
    if (strlen(fileName) < 4) 
        return INV_FILE;
    if (strcmp(&fileName[strlen(fileName) - 4], ".vcf") != 0) 
        return INV_FILE;

    // Open the file
    if (!(fp = fopen(fileName, "r"))) 
        return INV_FILE;

    // Create the card object
    if (!(*newCardObject = malloc(sizeof(Card)))) 
        return OTHER_ERROR;
        
    (*newCardObject)->fn = NULL;
    if (!((*newCardObject)->optionalProperties = initializeList(printProperty,deleteProperty,compareProperties))) 
        return OTHER_ERROR;
    (*newCardObject)->birthday = NULL;
    (*newCardObject)->anniversary = NULL;

    // Read through lines
    while (!(feof(fp))) {
        theError = readProperty(fp, &line);
        if (theError != OK) 
            break;
            
        theError = divideProperty(line, &groupName, &propertyName, &parameterValues, &propertyValues);
        if (theError != OK)
            break;
            
        if (propertyName == NULL) {
            theError = INV_PROP;
            break;
        }
        if (strcmp(propertyName, "") == 0) {
            theError = INV_PROP;
            break;
        }
        if (propertyValues == NULL) {
            theError = INV_PROP;
            break;
        }
        if (strcmp(propertyValues, "") == 0) {
            theError = INV_PROP;
            break;
        }

        /*Special Properties:
         * General - *BEGIN*, *END*, SOURCE, KIND, XML
         * Identification - *FN*, N NICKNAME, PHOTO, *BDAY*, *ANNIVERSARY*, GENDER
         */
        if (isFirstLine) {
            if ((strcmp(propertyName, "BEGIN") == 0) && (strcmp(propertyValues, "VCARD") == 0))
                isFirstLine = false;
            else {
                theError = INV_CARD;
                break;
            }
        }
        else if ((strcmp(propertyName, "VERSION") == 0)) {
            if ((strcmp(propertyValues, "4.0") != 0)) {
                theError = INV_CARD;
                break;
            }
            else {
                isVersionFour = true;
            }
        }
        else if ((stricasecmp(propertyName, "FN") == 0)) {
            theError = createProperty(&aProperty);
            
            if (theError != OK)
                break;
            (*newCardObject)->fn = aProperty;
            
            if (groupName) {
                replaceString(&(aProperty->group), groupName);
                if (!(aProperty->group)) {
                    theError = OTHER_ERROR;
                    break;
                }
            }
            replaceString(&(aProperty->name), propertyName);
            if (!(aProperty->name)) {
                theError = OTHER_ERROR;
                break;
            }
            if (parameterValues) {
                theError = addParams(aProperty, parameterValues);
                if (theError != OK)
                    break;
            }
            theError = addValues(aProperty, propertyValues);
            if (theError != OK)
                break;
        }
        else if ((stricasecmp(propertyName, "BDAY") == 0)) {
            createDateTime(&aDateTime, parameterValues, propertyValues);
            (*newCardObject)->birthday = aDateTime;
        }
        else if ((stricasecmp(propertyName, "ANNIVERSARY") == 0)) {
            createDateTime(&aDateTime, parameterValues, propertyValues);
            (*newCardObject)->anniversary = aDateTime;
        }
        else if (feof(fp)) {
            if ((strcmp(propertyName, "END") == 0) || (strcmp(propertyValues, "VCARD") == 0)) {
                isEnd = true;
            }
        }
        else {
            theError = createProperty(&aProperty);
            
            if (theError != OK)
                break;
            insertBack((*newCardObject)->optionalProperties, aProperty);
            
            if (groupName) {
                replaceString(&(aProperty->group), groupName);
                if (!(aProperty->group)) {
                    theError = OTHER_ERROR;
                    break;
                }
            }
            replaceString(&(aProperty->name), propertyName);
            if (!(aProperty->name)) {
                theError = OTHER_ERROR;
                break;
            }
            if (parameterValues) {
                theError = addParams(aProperty, parameterValues);
                if (theError != OK)
                    break;
            }
            theError = addValues(aProperty, propertyValues);
            if (theError != OK)
                break;
        }

        free(line);
        free(groupName);
        free(propertyName);
        free(parameterValues);
        free(propertyValues);
        line = NULL;
        groupName = NULL;
        propertyName = NULL;
        parameterValues = NULL;
        propertyValues = NULL;
    }

    free(line);
    free(groupName);
    free(propertyName);
    free(parameterValues);
    free(propertyValues);
    fclose(fp);
    if (theError == OK) {
        if (! ((*newCardObject)->fn) ) 
            theError = INV_CARD;
        else if (isVersionFour == false)
            theError = INV_CARD;
        else if (isEnd == false)
            theError = INV_CARD;
    }
    return theError;
}

void deleteCard(Card* obj) {
    if (obj == NULL) {
        return;
    }

    deleteProperty(obj->fn);
    deleteDate(obj->birthday);
    deleteDate(obj->anniversary);
    freeList(obj->optionalProperties);

    free(obj);
}
char* printCard(const Card* obj) {
    if (obj == NULL)
        return NULL;
    
    char* str = malloc(sizeof(char));
    strcpy(str,"");
    char* toCat;
    
    toCat = printProperty(obj->fn);
    if (toCat) {
        str = realloc(str, sizeof(char) * (strlen(str) + strlen(toCat) + 1));
        strcat(str, toCat);
        free(toCat);
    }
    
    str = realloc(str, sizeof(char) * (strlen(str) + 12));
    strcat(str, "\nBirthday:\n");
    toCat = printDate(obj->birthday);
    if (toCat) {
        str = realloc(str, sizeof(char) * (strlen(str) + strlen(toCat) + 1));
        strcat(str, toCat);
        free(toCat);
    }
    
    str = realloc(str, sizeof(char) * (strlen(str) + 15));
    strcat(str, "\nAnniversary:\n");
    toCat = printDate(obj->anniversary);
    if (toCat) {
        if ((str = realloc(str, sizeof(char) * (strlen(str) + strlen(toCat) + 1))))
            strcat(str, toCat);
        free(toCat);
    }
    
    toCat = toString(obj->optionalProperties);
    if (toCat) {
        str = realloc(str, sizeof(char) * (strlen(str) + strlen(toCat) + 1));
        strcat(str, toCat);
        free(toCat);
    }
    
    return str;
}
char* printError(VCardErrorCode err) {
    if (err == OK) 
        return duplicateString("OK");
    else if (err == INV_FILE) 
        return duplicateString("INV_FILE");
    else if (err == INV_CARD) 
        return duplicateString("INV_CARD");
    else if (err == INV_PROP) 
        return duplicateString("INV_PROP");
    else
        return duplicateString("OTHER_ERROR");
}

void deleteProperty(void* toBeDeleted) {
    Property* theProperty = (Property*) toBeDeleted;

    if (theProperty == NULL)
        return;

    free(theProperty->name);
    free(theProperty->group);
    freeList(theProperty->parameters);
    freeList(theProperty->values);
    free(theProperty);
}
int compareProperties(const void* first,const void* second) {
    Property* propertyOne = (Property*) first;
    Property* propertyTwo = (Property*) second;
    
    if (!(propertyOne) || !(propertyTwo))
        return -1;
        
    return (strcmp(propertyOne->name, propertyTwo->name));
}
char* printProperty(void* toBePrinted) {
    Property* theProperty = (Property*) toBePrinted;
    if (theProperty == NULL)
        return NULL;
        
    char* str = malloc(sizeof(char) * 16);
    strcpy(str, "Property Name: ");
    char* toCat = printValue(theProperty->name); 
    str = realloc(str, sizeof(char) * (strlen(str) + strlen(toCat) + 1));
    strcat(str, toCat);
    free(toCat);
    
    str = realloc(str, sizeof(char) * (strlen(str) + 14));
    strcat(str, "\nGroup Name: ");
    toCat = printValue(theProperty->group);
    str = realloc(str, sizeof(char) * (strlen(str) + strlen(toCat) + 1));
    strcat(str, toCat);
    free(toCat);
        
    str = realloc(str, sizeof(char) * (strlen(str) + 14));
    strcat(str, "\nParameters: ");
    toCat = toString(theProperty->parameters);
    str = realloc(str, sizeof(char) * (strlen(str) + strlen(toCat) + 1));
    strcat(str, toCat);
    free(toCat);
    
    str = realloc(str, sizeof(char) * (strlen(str) + 10));
    strcat(str, "\nValues: ");
    toCat = toString(theProperty->values);
    str = realloc(str, sizeof(char) * (strlen(str) + strlen(toCat) + 2));
    strcat(strcat(str, toCat), "\n");
    free(toCat);
        
    return str;
}

void deleteParameter(void* toBeDeleted) {
    if (toBeDeleted == NULL)
        return;

    free(toBeDeleted);
}
int compareParameters(const void* first,const void* second) {
    Parameter* parameterOne = (Parameter*) first;
    Parameter* parameterTwo = (Parameter*) second;
    
    if (!(parameterOne) || !(parameterTwo))
        return -1;
        
    return (strcmp(parameterOne->name, parameterTwo->name));    
}
char* printParameter(void* toBePrinted) {
    Parameter* theParameter = (Parameter*) toBePrinted;
    if (theParameter == NULL)
        return NULL;
        
    char* str = duplicateString(theParameter->name);
    char* toCat = duplicateString(theParameter->value);
    str = realloc(str, sizeof(char) * (strlen(str) + (strlen(toCat)) + 2));
    strcat(strcat(str, "="), toCat);
    free(toCat);
    
    return str;
}

void deleteValue(void* toBeDeleted) {
    if (toBeDeleted == NULL)
        return;

    free(toBeDeleted);
}
int compareValues(const void* first,const void* second) {
    char* valueOne = (char*) first;
    char* valueTwo = (char*) second;
    
    if (!(valueOne) || !(valueTwo))
        return -1;
        
    return (strcmp(valueOne, valueTwo));
}
char* printValue(void* toBePrinted) {
    char* theValue = (char*) toBePrinted;
    if (theValue == NULL) 
        return NULL;
        
    return duplicateString(theValue);
}

void deleteDate(void* toBeDeleted) {
    if (toBeDeleted == NULL)
        return;
    
    free(toBeDeleted);
}
int compareDates(const void* first,const void* second) {return 0;}
char* printDate(void* toBePrinted) {
    DateTime* theDate = (DateTime*) toBePrinted;
    if (theDate == NULL)
        return NULL;
        
    char* str = malloc(sizeof(char));
    strcpy(str, "");

    if (theDate->isText) {
        str = realloc(str, sizeof(char) * (strlen(theDate->text) + 7));
        strcpy(str, "Text: ");
        strcat(str, theDate->text);
    }
    else {
        str = realloc(str, sizeof(char) * 33);
        strcpy(str, "Date: ");
        strncat(str, theDate->date, 9);
        strcat(str, "\nTime: ");
        strncat(str, theDate->time, 7);
        strcat(str, "\n");
    }
    
    return str;
}

VCardErrorCode writeCard(const char* fileName, const Card* obj) {
    FILE* fp = NULL;
    void* elem = NULL;
    void* elem2 = NULL;
    Property* prop = NULL;
    Parameter* param = NULL;
    
    if (!(fp = fopen(fileName, "w"))) return WRITE_ERROR;
    
    if (!(fputs("BEGIN:VCARD\r\n", fp))) return WRITE_ERROR;
    
    if (!(fputs("VERSION:4.0\r\n", fp))) return WRITE_ERROR;
    
    if (obj) {
        // Adds FN property
        if (obj->fn) {
            if (obj->fn->group) {
                if (strcmp(obj->fn->group, "") != 0) {
                    if (!(fputs(obj->fn->group, fp))) return WRITE_ERROR;
                    if (!(fputc('.', fp))) return WRITE_ERROR;
                }
                if (strcmp(obj->fn->name, "") != 0) {
                    if (!(fputs(obj->fn->name, fp))) return WRITE_ERROR;
                }
                if (obj->fn->parameters) {
                    ListIterator iter = createIterator(obj->fn->parameters);
                    while ((elem = nextElement(&iter)) != NULL) {
                        param = (Parameter*) elem;
                        if (!(fputc(';', fp))) return WRITE_ERROR;
                        if (!(fputs(param->name, fp))) return WRITE_ERROR;
                        if (!(fputc('=', fp))) return WRITE_ERROR;
                        if (!(fputs(param->value, fp))) return WRITE_ERROR;
                    }
                }
                if (!(fputc(':', fp))) return WRITE_ERROR;
                if (obj->fn->values) {
                    ListIterator iter = createIterator(obj->fn->values);
                    elem = nextElement(&iter);
                    if (!(fputs((char*)elem, fp))) return WRITE_ERROR;
                    while ((elem = nextElement(&iter)) != NULL) {
                        if (!(fputc(';', fp))) return WRITE_ERROR;
                        if (!(fputs((char*)elem, fp))) return WRITE_ERROR;
                    }
                }
            }
            if (!(fputs("\r\n", fp))) return WRITE_ERROR;
        }
        // Adds BDAY
        if (obj->birthday) {
            if (obj->birthday->isText == true) {
                if (!(fputs("BDAY;VALUE=text:", fp))) return WRITE_ERROR;
                if (!(fputs(obj->birthday->text, fp))) return WRITE_ERROR;
                if (!(fputs("\r\n", fp))) return WRITE_ERROR;
            }
            else if ((strcmp(obj->birthday->date, "") != 0) || (strcmp(obj->birthday->time, "") != 0)) {
                if (!(fputs("BDAY:", fp))) return WRITE_ERROR;
                if (!(fputs(obj->birthday->date, fp))) return WRITE_ERROR;
                if (strcmp(obj->birthday->time, "") != 0) {
                    if (!(fputc('T', fp))) return WRITE_ERROR;
                    if (!(fputs(obj->birthday->time, fp))) return WRITE_ERROR;
                }
                if (obj->birthday->UTC == true) {
                    if (!(fputc('Z', fp))) return WRITE_ERROR;
                }
                if (!(fputs("\r\n", fp))) return WRITE_ERROR;
            }
        }
        // Adds ANNIVERSARY
        if (obj->anniversary) {
            if (obj->anniversary->isText == true) {
                if (!(fputs("ANNIVERSARY;VALUE=text:", fp))) return WRITE_ERROR;
                if (!(fputs(obj->anniversary->text, fp))) return WRITE_ERROR;
                if (!(fputs("\r\n", fp))) return WRITE_ERROR;
            }
            else if ((strcmp(obj->anniversary->date, "") != 0) || (strcmp(obj->anniversary->time, "") != 0)) {
                if (!(fputs("ANNIVERSARY:", fp))) return WRITE_ERROR;
                if (!(fputs(obj->anniversary->date, fp))) return WRITE_ERROR;
                if (strcmp(obj->anniversary->time, "") != 0) {
                    if (!(fputc('T', fp))) return WRITE_ERROR;
                    if (!(fputs(obj->anniversary->time, fp))) return WRITE_ERROR;
                }
                if (obj->anniversary->UTC == true) {
                    if (!(fputc('Z', fp))) return WRITE_ERROR;
                }
                if (!(fputs("\r\n", fp))) return WRITE_ERROR;
            }
        }
        // Adds other properties
        if (obj->optionalProperties) {
            ListIterator iter = createIterator(obj->optionalProperties);
            while ((elem = nextElement(&iter)) != NULL) {
                prop = (Property*) elem;
                if (prop) {
                    if (prop->group) {
                        if (strcmp(prop->group, "") != 0) {
                            if (!(fputs(prop->group, fp))) return WRITE_ERROR;
                            if (!(fputc('.', fp))) return WRITE_ERROR;
                        }
                    }
                    if (strcmp(prop->name, "") != 0) {
                        if (!(fputs(prop->name, fp))) return WRITE_ERROR;
                    }
                    if (prop->parameters) {
                        ListIterator iter2 = createIterator(prop->parameters);
                        while ((elem2 = nextElement(&iter2)) != NULL) {
                            param = (Parameter*) elem2;
                            if (!(fputc(';', fp))) return WRITE_ERROR;
                            if (!(fputs(param->name, fp))) return WRITE_ERROR;
                            if (!(fputc('=', fp))) return WRITE_ERROR;
                            if (!(fputs(param->value, fp))) return WRITE_ERROR;
                        }
                    }
                    if (!(fputc(':', fp))) return WRITE_ERROR;
                    if (prop->values) {
                        ListIterator iter2 = createIterator(prop->values);
                        elem2 = nextElement(&iter2);
                        if (!(fputs((char*)elem2, fp))) return WRITE_ERROR;
                        while ((elem2 = nextElement(&iter2)) != NULL) {
                            if (!(fputc(';', fp))) return WRITE_ERROR;
                            if (!(fputs((char*)elem2, fp))) return WRITE_ERROR;
                        }
                    }
                    if (!(fputs("\r\n", fp))) return WRITE_ERROR;
                }
            }
        }
    }
    
    if (!(fputs("END:VCARD\r\n", fp))) return WRITE_ERROR;
    
    return OK;
}

VCardErrorCode validateCard(const Card* obj) {    
    char propNames[][11] = {"SOURCE", "XML", "NICNNAME", "PHOTO",
            "EMAIL", "IMPP", "LANG", "TZ", "GEO", "TITLE", "ROLE",
            "LOGO", "MEMBER", "RELATED", "CATEGORIES", "NOTE",
            "SOUND", "URL", "KEY", "FBURL", "CALADRURI", "CALURI"};    // Other property names
    Property* prop = NULL;
    ListIterator iter;
    
    // Flags
    bool validName = true;
    bool fKIND = false;
    bool fN = false;
    bool fGENDER = false;
    bool fPRODID = false;
    bool fREV = false;
    bool fUID = false;
    
    // Check card object validity
    if (obj == NULL) return INV_CARD;
    if (obj->fn == NULL) return INV_CARD;
    if (obj->optionalProperties == NULL) return INV_CARD;
    
    // Check fn validity
    if (obj->fn->name == NULL) return INV_PROP;
    if (stricasecmp(obj->fn->name, "FN") != 0) return INV_CARD;
    if (obj->fn->group == NULL) return INV_PROP;
    if (obj->fn->parameters == NULL) return INV_PROP;
    if (obj->fn->values == NULL) return INV_PROP;
    if (getLength(obj->fn->values) != 1) return INV_PROP;
    
    
    // Check birthday
    if (obj->birthday) {
        if (obj->birthday->isText) {
            if (strcmp(obj->birthday->date, "") != 0) return INV_DT;
            if (strcmp(obj->birthday->time, "") != 0) return INV_DT;
        }
    }
    // Check anniversary
    if (obj->anniversary) {
        if (obj->anniversary->isText) {
            if (strcmp(obj->anniversary->date, "") != 0) return INV_DT;
            if (strcmp(obj->anniversary->time, "") != 0) return INV_DT;
        }
    }
    
    // Check optional properties
    iter = createIterator(obj->optionalProperties);
    while ((prop = (Property*) nextElement(&iter)) != NULL) {
        // Check property validity
        if (prop == NULL) return INV_PROP;
        if (prop->name == NULL) return INV_PROP;
        if (prop->group == NULL) return INV_PROP;
        if (prop->parameters == NULL) return INV_PROP;
        if (prop->values == NULL) return INV_PROP;
        
        // Check name and values
        if (stricasecmp(prop->name, "KIND") == 0) {
            if (fKIND) return INV_PROP;
            fKIND = true;
            if (getLength(prop->values) != 1) return INV_PROP;
        }
        else if (stricasecmp(prop->name, "N") == 0) {
            if (fN) return INV_PROP;
            fN = true;
            if (getLength(prop->values) != 5) return INV_PROP;
        }
        else if (stricasecmp(prop->name, "GENDER") == 0) {
            if (fGENDER) return INV_PROP;
            fGENDER = true;
            if (getLength(prop->values) != 1 && getLength(prop->values) != 2) return INV_PROP;
        }
        else if (stricasecmp(prop->name, "PRODID") == 0) {
            if (fPRODID) return INV_PROP;
            fPRODID = true;
            if (getLength(prop->values) != 1) return INV_PROP;
        }
        else if (stricasecmp(prop->name, "REV") == 0) {
            if (fREV) return INV_PROP;
            fREV = true;
            if (getLength(prop->values) != 1) return INV_PROP;
        }
        else if (stricasecmp(prop->name, "UID") == 0) {
            if (fUID) return INV_PROP;
            fUID = true;
            if (getLength(prop->values) != 1) return INV_PROP;
        }
        else if (stricasecmp(prop->name, "ADR") == 0) {
            if (getLength(prop->values) != 7) return INV_PROP;
        }
        else if (stricasecmp(prop->name, "TEL") == 0) {
            if (getLength(prop->values) != 1 && getLength(prop->values) != 2) return INV_PROP;
        }
        else if (stricasecmp(prop->name, "ORG") == 0) {
            if (getLength(prop->values) == 0) return INV_PROP;
        }
        else if (stricasecmp(prop->name, "CLIENTPIDMAP") == 0) {
            if (getLength(prop->values) != 2) return INV_PROP;
        }
        else if (stricasecmp(prop->name, "BEGIN") == 0) return INV_CARD;
        else if (stricasecmp(prop->name, "END") == 0) return INV_CARD;
        else if (stricasecmp(prop->name, "VERSION") == 0) return INV_CARD;
        else if (stricasecmp(prop->name, "BDAY") == 0) return INV_DT;
        else if (stricasecmp(prop->name, "ANNIVERSARY") == 0) return INV_DT;
        else {
            validName = false;
            for (int i = 0; i < 22; i++) {
                if (stricasecmp(prop->name, propNames[i]) == 0) {
                    validName = true;
                    if (getLength(prop->values) != 1) return INV_PROP;
                    break;
                }
            }
        }
        
        if (!(validName)) return INV_PROP;
    }
    return OK;
}

char* strListToJSON(const List* strList) {
    ListIterator iter;
    char* JSONstr = NULL;
    char* aValue = NULL;
    
    JSONstr = malloc(sizeof(char) * 3);
    strcpy(JSONstr,"[");
    
    if (strList != NULL) {
        iter = createIterator((List*)strList);
        aValue = (char*) nextElement(&iter);
        if (aValue != NULL) {
            JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + strlen(aValue) + 4));
            strcat(JSONstr, "\"");
            strcat(JSONstr, aValue);
            strcat(JSONstr, "\"");
            while ((aValue = (char*) nextElement(&iter)) != NULL) {
                strcat(JSONstr, ",");
                JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + strlen(aValue) + 4));
                strcat(JSONstr, "\"");
                strcat(JSONstr, aValue);
                strcat(JSONstr, "\"");
            }
        }
    }
    strcat(JSONstr, "]");
    
    return JSONstr;
}

List* JSONtoStrList(const char* str) {
    char* JSONstr;
    char* p1;
    char* p2;
    
    if (str == NULL) return NULL;
    if (strlen(str) < 2) return NULL;
    List* aList = initializeList(printValue, deleteValue, compareValues);
    if (!(JSONstr = duplicateString(str))) {
        freeList(aList);
        return NULL;
    }
    
    p1 = JSONstr;
    if (*p1++ != '[') {
        free(JSONstr);
        freeList(aList);
        return NULL;
    }
    if (*p1 == ']') {
        free(JSONstr);
        return aList;
    }
    
    p1 = strTokenizer(JSONstr, '"');
    while(p1 != NULL) {
        if (!(p2 = strTokenizer(p1, '"'))) {
            free(JSONstr);
            freeList(aList);
            return NULL;
        }
        insertBack(aList, duplicateString(p1));
        p1 = strTokenizer(p2, '"');
    }
    
    free(JSONstr);
    return aList;
}

char* propToJSON(const Property* prop) {
    char* JSONstr = malloc(sizeof(char));
    char* aString = NULL;
    
    strcpy(JSONstr, "");
    if (prop == NULL) return JSONstr;
    
    JSONstr = realloc(JSONstr, sizeof(char) * 11);
    strcat(JSONstr, "{\"group\":\"");
    if (prop->group != NULL) {
        JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + strlen(prop->group) + 1));
        strcat(JSONstr, prop->group);
    }
    JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + 11));
    strcat(JSONstr, "\",\"name\":\"");
    if (prop->name) {
        JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + strlen(prop->name) + 1));
        strcat(JSONstr, prop->name);
    }
    JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + 12));
    strcat(JSONstr, "\",\"values\":");
    if (prop->values) {
        aString = strListToJSON(prop->values);
        JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + strlen(aString) + 1));
        strcat(JSONstr, aString);
        free(aString);    
    }
    JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + 2));
    strcat(JSONstr, "}");
    
    return JSONstr;
}

Property* JSONtoProp(const char* str) {
    Property* aProperty = NULL;
    char* JSONstr;
    char* p1;
    char* p2;
    
    if (str == NULL) return NULL;
    if (!(JSONstr = duplicateString(str))) return NULL;
    
    if (*str != '{') return NULL;
    if (str[strlen(str) - 1] != '}') {
        free(JSONstr);
        return NULL;
    }
    
    // Extract group
    if (!(p1 = strTokenizer(JSONstr, '"'))) {
        free(JSONstr);
        return NULL;
    }
    if (!(p2 = strTokenizer(p1, '"'))) {
        free(JSONstr);
        return NULL;
    }
    if (stricasecmp(p1,"group") != 0) {
        free(JSONstr);
        return NULL;
    }
    if (!(p1 = strTokenizer(p2, '"'))) {
        free(JSONstr);
        return NULL;
    }
    if (!(p2 = strTokenizer(p1, '"'))) {
        free(JSONstr);
        return NULL;
    }
    if ((createProperty(&aProperty)) != OK) {
        free(JSONstr);
        return NULL;
    }
    if (!(replaceString(&aProperty->group, p1))) {
        free(JSONstr);
        deleteProperty(aProperty);
        return NULL;
    }
    
    // Extract name
    if (!(p1 = strTokenizer(p2, '"'))) {
        free(JSONstr);
        deleteProperty(aProperty);
        return NULL;
    }
    if (!(p2 = strTokenizer(p1, '"'))) {
        free(JSONstr);
        deleteProperty(aProperty);
        return NULL;
    }
    if (stricasecmp(p1,"name") != 0) {
        free(JSONstr);
        deleteProperty(aProperty);
        return NULL;
    }
    if (!(p1 = strTokenizer(p2, '"'))) {
        free(JSONstr);
        deleteProperty(aProperty);
        return NULL;
    }
    if (!(p2 = strTokenizer(p1, '"'))) {
        free(JSONstr);
        deleteProperty(aProperty);
        return NULL;
    }
    if (!(replaceString(&aProperty->name, p1))) {
        free(JSONstr);
        deleteProperty(aProperty);
        return NULL;
    }
    
    // Extract values
    if (!(p1 = strTokenizer(p2, '"'))) {
        free(JSONstr);
        deleteProperty(aProperty);
        return NULL;
    }
    if (!(p2 = strTokenizer(p1, '"'))) {
        free(JSONstr);
        deleteProperty(aProperty);
        return NULL;
    }
    if (stricasecmp(p1,"values") != 0) {
        free(JSONstr);
        deleteProperty(aProperty);
        return NULL;
    }
    if (!(p1 = strTokenizer(p2, ':'))) {
        free(JSONstr);
        deleteProperty(aProperty);
        return NULL;
    }
    if (!(p2 = strTokenizer(p1, '}'))) {
        free(JSONstr);
        deleteProperty(aProperty);
        return NULL;
    }
    freeList(aProperty->values);
    if (!(aProperty->values = JSONtoStrList(p1))) {
        free(JSONstr);
        deleteProperty(aProperty);
        return NULL;
    }
    
    free(JSONstr);
    return aProperty;
}

char* dtToJSON(const DateTime* prop) {
    char* JSONstr = malloc(sizeof(char));

    strcpy(JSONstr, "");
    if (prop == NULL) return JSONstr;
    
    JSONstr = realloc(JSONstr, sizeof(char) * 11);
    strcat(JSONstr, "{\"isText\":");
    if (prop->isText) {
        JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + 5));
        strcat(JSONstr, "true");
    }
    else {
        JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + 6));
        strcat(JSONstr, "false");
    }
    JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + 10));
    strcat(JSONstr, ",\"date\":\"");
    JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + strlen(prop->date) + 1));
    strcat(JSONstr, prop->date);
    JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + 11));
    strcat(JSONstr, "\",\"time\":\"");
    JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + strlen(prop->time) + 1));
    strcat(JSONstr, prop->time);
    JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + 11));
    strcat(JSONstr, "\",\"text\":\"");
    JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + strlen(prop->text) + 1));
    strcat(JSONstr, prop->text);
    JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + 11));
    strcat(JSONstr, "\",\"isUTC\":");
    if (prop->UTC) {
        JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + 6));
        strcat(JSONstr, "true}");
    }
    else {
        JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + 7));
        strcat(JSONstr, "false}");
    }
    
    return JSONstr;
}

DateTime* JSONtoDT(const char* str) {
    DateTime* aDT = NULL;
    char* JSONstr;
    char* p1;
    char* p2;
    
    if (str == NULL) return NULL;
    if (!(JSONstr = duplicateString(str))) return NULL;
    
    if (*str != '{') return NULL;
    if (str[strlen(str) - 1] != '}') {
        free(JSONstr);
        return NULL;
    }
    
    // Extract isText
    if (!(p1 = strTokenizer(JSONstr, '"'))) {
        free(JSONstr);
        return NULL;
    }
    if (!(p2 = strTokenizer(p1, '"'))) {
        free(JSONstr);
        return NULL;
    }
    if (stricasecmp(p1,"isText") != 0) {
        free(JSONstr);
        return NULL;
    }
    if (!(p1 = strTokenizer(p2, ':'))) {
        free(JSONstr);
        return NULL;
    }
    if (!(p2 = strTokenizer(p1, ','))) {
        free(JSONstr);
        return NULL;
    }
    if (!(aDT = malloc(sizeof(DateTime) + sizeof(char)))) {
        free(JSONstr);
        return NULL;
    }
    if (stricasecmp(p1, "false") == 0) aDT->isText = false;
    else if (stricasecmp(p1, "true") == 0) aDT->isText = true;
    else {
        free(JSONstr);
        free(aDT);
        return NULL;
    }
    
    // Extract date
    if (!(p1 = strTokenizer(p2, '"'))) {
        free(JSONstr);
        free(aDT);
        return NULL;
    }
    if (!(p2 = strTokenizer(p1, '"'))) {
        free(JSONstr);
        free(aDT);
        return NULL;
    }
    if (stricasecmp(p1,"date") != 0) {
        free(JSONstr);
        free(aDT);
        return NULL;
    }
    if (!(p1 = strTokenizer(p2, '"'))) {
        free(JSONstr);
        free(aDT);
        return NULL;
    }
    if (!(p2 = strTokenizer(p1, '"'))) {
        free(JSONstr);
        free(aDT);
        return NULL;
    }
    if (strlen(p1) > 8) {
        free(JSONstr);
        free(aDT);
        return NULL;
    }
    strcpy(aDT->date, p1);
    
    // Extract time
    if (!(p1 = strTokenizer(p2, '"'))) {
        free(JSONstr);
        free(aDT);
        return NULL;
    }
    if (!(p2 = strTokenizer(p1, '"'))) {
        free(JSONstr);
        free(aDT);
        return NULL;
    }
    if (stricasecmp(p1,"time") != 0) {
        free(JSONstr);
        free(aDT);
        return NULL;
    }
    if (!(p1 = strTokenizer(p2, '"'))) {
        free(JSONstr);
        free(aDT);
        return NULL;
    }
    if (!(p2 = strTokenizer(p1, '"'))) {
        free(JSONstr);
        free(aDT);
        return NULL;
    }
    if (strlen(p1) > 6) {
        free(JSONstr);
        free(aDT);
        return NULL;
    }
    strcpy(aDT->time, p1);
    
    // Extract text
    if (!(p1 = strTokenizer(p2, '"'))) {
        free(JSONstr);
        free(aDT);
        return NULL;
    }
    if (!(p2 = strTokenizer(p1, '"'))) {
        free(JSONstr);
        free(aDT);
        return NULL;
    }
    if (stricasecmp(p1,"text") != 0) {
        free(JSONstr);
        free(aDT);
        return NULL;
    }
    if (!(p1 = strTokenizer(p2, '"'))) {
        free(JSONstr);
        free(aDT);
        return NULL;
    }
    if (!(p2 = strTokenizer(p1, '"'))) {
        free(JSONstr);
        free(aDT);
        return NULL;
    }
    if (!(aDT = realloc(aDT, sizeof(DateTime) + (sizeof(char) * (strlen(p1) +1))))) {
        free(JSONstr);
        free(aDT);
        return NULL;
    }
    strcpy(aDT->text, p1);
    
    // Extract isUTC
    if (!(p1 = strTokenizer(p2, '"'))) {
        free(JSONstr);
        free(aDT);
        return NULL;
    }
    if (!(p2 = strTokenizer(p1, '"'))) {
        free(JSONstr);
        free(aDT);
        return NULL;
    }
    if (stricasecmp(p1,"isUTC") != 0) {
        free(JSONstr);
        free(aDT);
        return NULL;
    }
    if (!(p1 = strTokenizer(p2, ':'))) {
        free(JSONstr);
        free(aDT);
        return NULL;
    }
    if (!(p2 = strTokenizer(p1, '}'))) {
        free(JSONstr);
        free(aDT);
        return NULL;
    }
    if (stricasecmp(p1, "false") == 0) aDT->UTC = false;
    else if (stricasecmp(p1, "true") == 0) aDT->UTC = true;
    else {
        free(JSONstr);
        free(aDT);
        return NULL;
    }
    
    free(JSONstr);
    return aDT;
}

Card* JSONtoCard(const char* str) {
    Card* aCard = NULL;
    Property* aProperty = NULL;
    char* JSONstr;
    char* p1;
    char* p2;
    
    if (str == NULL) return NULL;
    if (!(JSONstr = duplicateString(str))) return NULL;
    
    if (*str != '{') return NULL;
    if (str[strlen(str) - 1] != '}') {
        free(JSONstr);
        return NULL;
    }
    
    // Extract FN
    if (!(p1 = strTokenizer(JSONstr, '"'))) {
        free(JSONstr);
        return NULL;
    }
    if (!(p2 = strTokenizer(p1, '"'))) {
        free(JSONstr);
        return NULL;
    }
    if (stricasecmp(p1,"FN") != 0) {
        free(JSONstr);
        return NULL;
    }
    if (!(p1 = strTokenizer(p2, '"'))) {
        free(JSONstr);
        return NULL;
    }
    if (!(p2 = strTokenizer(p1, '"'))) {
        free(JSONstr);
        return NULL;
    }
    if (!(aCard = malloc(sizeof(Card)))) {
        free(JSONstr);
        return NULL;
    }
    aCard->fn = NULL;
    aCard->optionalProperties = NULL;
    aCard->birthday = NULL;
    aCard->anniversary = NULL;
    if (!(aCard->optionalProperties = initializeList(printProperty, deleteProperty, compareProperties))) {
        free(JSONstr);
        deleteCard(aCard);
        return NULL;
    }
    if (createProperty(&aProperty) != OK) {
        free(JSONstr);
        deleteCard(aCard);
        return NULL;
    }
    if (!(replaceString(&aProperty->name, "FN"))) {
        free(JSONstr);
        deleteCard(aCard);
        deleteProperty(aProperty);
        return NULL;
    }
    if (addValues(aProperty, p1) != OK) {
        free(JSONstr);
        deleteCard(aCard);
        deleteProperty(aProperty);
        return NULL;
    }
    aCard->fn = aProperty;
    
    free(JSONstr);
    return aCard;
}

void addProperty(Card* card, const Property* toBeAdded) {
    if (card == NULL || toBeAdded == NULL) return;
    if (card->optionalProperties == NULL) return;
    
    insertBack(card->optionalProperties, (void*) toBeAdded);
     
    return;
}

char* getSummaryFromFile(char* fileName) {
    Card* myCard = NULL;
    char* text = malloc(sizeof(char) * 50);
    strcpy(text, "Error");

    FILE* file;
    if (!(file = fopen(fileName, "r"))) {
        snprintf(text, 49, "Error: \"%s\" not found", fileName);
        text[49] = '\0';
        return text;
    }
    fclose(file);

    if ((createCard(fileName, &myCard)) != OK) {
        deleteCard(myCard);
        strcpy(text, "Error: Could not create card");
        return text;
    }
    if (validateCard(myCard) != OK) {
        deleteCard(myCard);
        strcpy(text, "Error: Not a valid card");
        return text;
    }
    int length = getLength(myCard->optionalProperties);
    if (myCard->birthday) length++;
    if (myCard->anniversary) length++;
    char* JSONstr = malloc(sizeof(char));
    strcpy(JSONstr, "");
    char num[5];
    char *theFile = duplicateString(fileName);
    char *p1 = strTokenizer(theFile, '/');
    
    JSONstr = realloc(JSONstr, sizeof(char) * 10);
    strcat(JSONstr, "{\"file\":\"");
    JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + strlen(p1) + 1));
    strcat(JSONstr, p1);
    free(theFile);
    JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + 12));
    strcat(JSONstr, "\", \"name\":\"");
    char* name = getFromFront(myCard->fn->values);
    JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + strlen(name) + 1));
    strcat(JSONstr, name);
    JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + 16));
    strcat(JSONstr, "\", \"opLength\":\"");
    sprintf(num, "%d", length);
    JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + 5));
    strcat(JSONstr, num);
    JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + 3));
    strcat(JSONstr, "\"}");
    
    deleteCard(myCard);
    free(text);
    return JSONstr;
}

char* getPropertiesFromFile(char* fileName) {
    Card* myCard = NULL;
    char* text = malloc(sizeof(char) * 50);
    strcpy(text, "Error");

    FILE* file;
    if (!(file = fopen(fileName, "r"))) {
        snprintf(text, 49, "Error: \"%s\" not found", fileName);
        text[49] = '\0';
        return text;
    }
    fclose(file);

    if ((createCard(fileName, &myCard)) != OK) {
        deleteCard(myCard);
        strcpy(text, "Error: Could not create card");
        return text;
    }
    if (validateCard(myCard) != OK) {
        deleteCard(myCard);
        strcpy(text, "Error: Not a valid card");
        return text;
    }
    
    char* JSONstr = malloc(sizeof(char));
    strcpy(JSONstr, "");
    
    JSONstr = realloc(JSONstr, sizeof(char) * 38);
    strcat(JSONstr, "[{\"number\":\"1\",\"name\":\"FN\",\"values\":\"");
    char* values = valuesToJSON(myCard->fn->values);
    JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + strlen(values) + 1));
    strcat(JSONstr, values);
    free(values);
    JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + 3));
    strcat(JSONstr, "\"}");
    
    char num[5];
    int n = 1;
    ListIterator iter = createIterator((List*)myCard->optionalProperties);
    Property *aProperty;
    while((aProperty = (Property*) nextElement(&iter)) != NULL) {
        JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + 13));
        strcat(JSONstr, ",{\"number\":\"");
        n++;
        sprintf(num, "%d", n);
        JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + strlen(num) + 1));
        strcat(JSONstr, num);
        JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + 11));
        strcat(JSONstr, "\",\"name\":\"");
        JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + strlen(aProperty->name) + 1));
        strcat(JSONstr, aProperty->name);
        JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + 13));
        strcat(JSONstr, "\",\"values\":\"");
        values = valuesToJSON(aProperty->values);
        JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + strlen(values) + 1));
        strcat(JSONstr, values);
        free(values);
        JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + 3));
        strcat(JSONstr, "\"}");
    }
    JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + 2));
    strcat(JSONstr, "]");
    
    deleteCard(myCard);
    free(text);
    return JSONstr;
}

char* valuesToJSON(const List* strList) {
    ListIterator iter;
    char* JSONstr = NULL;
    char* aValue = NULL;
    
    JSONstr = malloc(sizeof(char));
    
    if (strList != NULL) {
        iter = createIterator((List*)strList);
        aValue = (char*) nextElement(&iter);
        if (aValue != NULL) {
            JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + strlen(aValue) + 1));
            strcat(JSONstr, aValue);
            while ((aValue = (char*) nextElement(&iter)) != NULL) {
                JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + 3));
                strcat(JSONstr, ", ");
                JSONstr = realloc(JSONstr, sizeof(char) * (strlen(JSONstr) + strlen(aValue) + 1));
                strcat(JSONstr, aValue);
            }
        }
    }
    
    return JSONstr;
}
