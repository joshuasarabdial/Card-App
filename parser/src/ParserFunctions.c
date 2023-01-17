/**
 * @file ParserFunctions.c
 * @author Joshua Sarabdial
 * @date October 2018
 **/
 
#include "ParserFunctions.h"

VCardErrorCode readProperty(FILE* fp, char** line) {
    char buffer[BUFFERSIZE];
    int c;
    int checkNext = TRUE;

    if (!feof(fp)) {
        fgets(buffer, BUFFERSIZE, fp);
        // Check line ending for CRLF
        if (strlen(buffer) > 1)
            if (buffer[strlen(buffer) - 2] == '\r' && buffer[strlen(buffer) - 1] == '\n') 
                buffer[strlen(buffer) - 2] = '\0';
            else
                return INV_PROP;
        else 
            return INV_PROP;

        if (!(*line = duplicateString(buffer))) {
            return OTHER_ERROR;
        }
    }

    // Line unfolding
    while (!feof(fp) && checkNext) {
        c = fgetc(fp);
        if (((char) c) == ' ') {
            fgets(buffer, BUFFERSIZE, fp);
            // Check line ending for CRLF
            if (strlen(buffer) > 1)
                if (buffer[strlen(buffer) - 2] == '\r' && buffer[strlen(buffer) - 1] == '\n') 
                    buffer[strlen(buffer) - 2] = '\0';
                else
                    return INV_PROP;
            else 
                return INV_PROP;

            if (!(*line = realloc(*line, sizeof(char) * (strlen(*line) + strlen(buffer) + 1)))) {
                return OTHER_ERROR;
            }
            strcat(*line, buffer);
        }
        else {
            ungetc(c, fp);
            checkNext = FALSE;
        }
    }

    return OK;
}

VCardErrorCode divideProperty(char* line, char** groupName, char** propertyName, char** parameterValue, char** propertyValue) {
    char* strptr1 = NULL;
    char* strptr2 = NULL;
    char* strptr3 = NULL;
    char* strptr4 = NULL;

    strptr4 = strTokenizer(line, ':');
    strptr3 = strTokenizer(line, ';');
    strptr2 = strTokenizer(line, '.');
    if (strptr2 == NULL) {
        strptr2 = line;
    }
    else {
        strptr1 = line;
    }
        
    if (strptr1) {
        if (!(*groupName = duplicateString(strptr1))) {
            return OTHER_ERROR;
        }
    }
    if (strptr2) {
        if (!(*propertyName = duplicateString(strptr2))) {
            return OTHER_ERROR;
        }
    }
    if (strptr3) {
        if (!(*parameterValue = duplicateString(strptr3))) {
            return OTHER_ERROR;
        }
    }
    if (strptr4) {
        if (!(*propertyValue = duplicateString(strptr4))) {
            return OTHER_ERROR;
        }
    }
    //printf("%s || %s || %s || %s\n", *groupName, *propertyName, *parameterValue, *propertyValue);
    return OK;
}

VCardErrorCode createProperty(Property** newProperty) {
    if (!(*newProperty = malloc(sizeof(Property)))) {
        return OTHER_ERROR;
    }

    if (!((*newProperty)->name = duplicateString(""))) {
        return OTHER_ERROR;
    }

    if (!((*newProperty)->group = duplicateString(""))) {
        return OTHER_ERROR;
    }

    if (!((*newProperty)->parameters = initializeList(printParameter, deleteParameter, compareParameters))) {
        return OTHER_ERROR;
    }

    if (!((*newProperty)->values = initializeList(printValue, deleteValue, compareValues))) {
        return OTHER_ERROR;
    }

    return OK;
}

VCardErrorCode createParameter(Parameter** newParameter, char* theValue) {
    if (!(*newParameter = malloc(sizeof(Parameter) + (sizeof(char) * (strlen(theValue) + 1))))) {
        return OTHER_ERROR;
    }

    return OK;
}

VCardErrorCode createDateTime(DateTime** newDateTime, char* parameterValue, char* propertyValue) {
    
    if (parameterValue) {
        if ((strcmp(parameterValue, "VALUE=text") == 0)) {
            if (!(*newDateTime = malloc(sizeof(DateTime) + (sizeof(char) * (strlen(propertyValue) + 1))))) {
                return OTHER_ERROR;
            }
            (*newDateTime)->UTC = false;
            (*newDateTime)->isText = true;
            strcpy((*newDateTime)->date, "");
            strcpy((*newDateTime)->time, "");
            strcpy((*newDateTime)->text, propertyValue);
        }
        else {
            return INV_PROP;
        }
    }
    else {
        if (!(*newDateTime = malloc(sizeof(DateTime) + (sizeof(char))))) {
            return OTHER_ERROR;
        }
        strcpy((*newDateTime)->date, "");
        strcpy((*newDateTime)->time, "");
        strcpy((*newDateTime)->text, "");
        (*newDateTime)->isText = false;
        
        if (propertyValue[strlen(propertyValue) - 1] == 'Z') {
            (*newDateTime)->UTC = true;
            propertyValue[strlen(propertyValue) - 1] = '\0';
        }
        else {
            (*newDateTime)->UTC = false;
        }
        
        char* strptr = NULL;
        //printf("%s\n", propertyValue);
        strptr = strTokenizer(propertyValue, 'T');
        // Is date only
        if (strptr == NULL) {
            if (strlen(propertyValue) > 8) {
                propertyValue[8] = '\0';
            }
            strcpy((*newDateTime)->date, propertyValue);
        }
        // Is date and time
        else {
            if (strlen(propertyValue) > 8) {
                propertyValue[8] = '\0';
            }
            strcpy((*newDateTime)->date, propertyValue);
            strTokenizer(strptr, '-');
            if (strlen(strptr) > 6) {
                strptr[6] = '\0';
            }
            strcpy((*newDateTime)->time, strptr);
        } 
    }
    
    return OK;
}

VCardErrorCode addParams(Property* theProperty, char* parameterValues) {
    VCardErrorCode err = OK;
    if (parameterValues == NULL) {
        return err;
    }

    char* strptr1;
    char* strptr2 = parameterValues;
    char* strptr3;
    Parameter* aParameter;

    strptr1 = strTokenizer(strptr2, ';');

    do {
        strptr3 = strTokenizer(strptr2, '=');
        if (strptr3 == NULL) return INV_PROP;
        if (strcmp(strptr3, "") == 0) return INV_PROP;
        err = createParameter(&aParameter, strptr3);
        if (err != OK) return err;
        insertBack(theProperty->parameters, aParameter);
        strncpy(aParameter->name, strptr2, 200);
        strcpy(aParameter->value, strptr3);
        strptr2 = strptr1;
        strptr1 = strTokenizer(strptr2, ';');
    } while (strptr2);

    return err;
}

VCardErrorCode addValues(Property* theProperty, char* propertyValues) {
    if (propertyValues == NULL) {
        return OK;
    }

    char* strptr1;
    char* strptr2 = propertyValues;
    char* toInsert;

    strptr1 = strTokenizer(strptr2, ';');

    do {
        if (!(toInsert = duplicateString(strptr2))) {
            return OTHER_ERROR;
        }

        insertBack(theProperty->values, toInsert);
        strptr2 = strptr1;
        strptr1 = strTokenizer(strptr2, ';');
    } while (strptr2);

    return OK;
}

char* duplicateString(const char* theString) {
    if (theString == NULL) {
        return NULL;
    }

    char* aString = malloc(sizeof(char) * (strlen(theString) + 1));
    strcpy(aString, theString);

    return aString;
}

int replaceString(char** toReplace, char* toCopy) {
    if (toCopy == NULL) {
        free(*toReplace);
        return 1;
    }

    if (!(*toReplace = realloc(*toReplace, sizeof(char) * (strlen(toCopy) + 1)))) {
        return -1;
    }
    strcpy(*toReplace, toCopy);
    return 1;
}

char* strTokenizer(char* str, char delimiter) {
    if (str == NULL) {
        return NULL;
    }

    if (str[0] == delimiter) {
        str[0] = '\0';
        return &(str[1]);
    }

    for (int i = 1; i < strlen(str) + 1; i++) {
        if (str[i] == delimiter && str[i - 1] != '\\') {
            str[i] = '\0';
            return &(str[i + 1]);
        }
    }

    return NULL;
}

/* Based on strcasecmp() function from GNU C Library.
 * Original code from https://code.woboq.org/userspace/glibc/string/strcasecmp.c.html
 * Date Viewed: October 22, 2018
 */
int stricasecmp(const char* s1, const char* s2) {	
    int result;
    
    if (strlen(s1) != strlen(s2)) return -1;

    while ((result = tolower(*s1) - tolower(*s2++)) == 0) {
        if (*s1++ == '\0') 
			break;
    }
    
    return result;
}
/* End of code reuse. */
