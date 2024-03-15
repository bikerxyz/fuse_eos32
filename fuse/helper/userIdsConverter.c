//
// Created by niklas on 10.01.23.
//

#include <limits.h>
#include "userIdsConverter.h"
IdConverter *userIdConverter;
IdConverter *groupIdConverter;
unsigned int userIdConverterSize;
unsigned int userIdConverterMaxSize;
unsigned int groupIdConverterSize;
unsigned int groupIdConverterMaxSize;
#define GET_EOS32_ID true
#define GET_FUSE_ID false

char *completeLine = NULL;

unsigned int searcher(IdConverter *idConverter, unsigned int idConverterSize,bool getEOS32Id, unsigned searchingUserId){
    unsigned int ou = -1;

    /* wir haben 2 ternäre operatoren. 1. in der if bedingung wo verglichen wird und den 2. wo dann im if die gefundene userid übergeben wird
     * diese methode wurde gebaut um mehrfach kopierten code zu vermeiden.
     *
     * wenn diese funktion nichts finden wird das zurückgegeben was übergeben worden ist, also die searchingUserId
     */
    for(unsigned int i1 = 0; i1 < idConverterSize; i1++){
        if((getEOS32Id ? idConverter[i1].linuxId : idConverter[i1].eos32Id) == searchingUserId){
            ou = (getEOS32Id ? idConverter[i1].eos32Id : idConverter[i1].linuxId);
            break;
        }
    }

    return ou;
}

unsigned int getFuseUserIdByEos32IdPrivate(unsigned int userEos32Id){
    return searcher(userIdConverter, userIdConverterSize, GET_FUSE_ID, userEos32Id);
}
unsigned int getEos32UserIdByFuseIdPrivate(unsigned int userFuseId){
    return searcher(userIdConverter, userIdConverterSize, GET_EOS32_ID, userFuseId);
}
unsigned int getFuseGroupIdByEos32IdPrivate(unsigned int groupEos32Id){
    return searcher(groupIdConverter, groupIdConverterSize, GET_FUSE_ID, groupEos32Id);
}
unsigned int getEos32GroupIdByFuseIdPrivate(unsigned int groupFuseId){
return searcher(groupIdConverter, groupIdConverterSize, GET_EOS32_ID, groupFuseId);
}

unsigned int getFuseUserIdByEos32Id(unsigned int userEos32Id){
    unsigned int ou = getFuseUserIdByEos32IdPrivate(userEos32Id);
    if(ou == -1){
        return userEos32Id;
    }else{
        return ou;
    }
}
unsigned int getEos32UserIdByFuseId(unsigned int userFuseId){
    unsigned int ou = getEos32UserIdByFuseIdPrivate(userFuseId);
    if(ou == -1){
        return userFuseId;
    }else{
        return ou;
    }
}
unsigned int getFuseGroupIdByEos32Id(unsigned int groupEos32Id){
    unsigned int ou = getFuseGroupIdByEos32IdPrivate(groupEos32Id);
    if(ou == -1){
        return groupEos32Id;
    }else{
        return ou;
    }
}
unsigned int getEos32GroupIdByFuseId(unsigned int groupFuseId) {
    unsigned int ou = getEos32GroupIdByFuseIdPrivate(groupFuseId);
    if (ou == -1) {
        return groupFuseId;
    } else {
        return ou;
    }
}
#define WHITESPACE_CHARACTER_SIZE 4
unsigned int whitespaceCharactersSize = WHITESPACE_CHARACTER_SIZE;
char whitespaceCharacters[WHITESPACE_CHARACTER_SIZE] = {' ', '\t','\0','\r'};

/*
 * return value is der text wo es weiter geht
 */
char *skipBeginning(char *text){
    char *begin = NULL;
    for(int i1= 0; i1 < strlen(text);i1++){
        bool foundSkippableCharacter = false;
        for(int i2 = 0; i2 < whitespaceCharactersSize;i2++){
            if(text[i1] == whitespaceCharacters[i2]){
                foundSkippableCharacter = true;
            }
        }
        if(!foundSkippableCharacter){
            begin = text + i1;
            return begin;
        }
    }
    return NULL;
}

bool isCheckLineHas2Colons(const char *line){
    int validCountOfColons = 2;

    bool isStop = false;
    int colonsCounter = 0;
    for(int i1 = 0; !isStop;i1++){
        if(line[i1] == '\0' || line[i1] == '#'){
            isStop = true;
        }else{
            if(line[i1] == ':'){
                colonsCounter++;
            }
        }
    }

    if(colonsCounter == validCountOfColons) return true;
    return false;
}
bool isLetter(char character){
    return (character >= 'A' && character <= 'Z') || (character >= 'a' && character <= 'z');
}

bool isNumber(char number){
    return (number >= '0' && number <= '9');
}

bool isLetterOrNumber(char input){
    return isLetter(input) || isNumber(input);
}

char *extractUserName(char *text, char  **userName, unsigned int lineNumber){
    char *token = strstr(text, ":");
    if(token == NULL){
        printf("\n%s \"%u\" %s \"%s\" \n",
               "Error: readConverterfile->extractUserName: Zeile: ",
               lineNumber,
               "In der Zeile müsste ein  \":\" sein, um den Username von der UserId zu trennen, jedoch wurde das Zeichen nicht gefunden\nZeile: ",
               completeLine);
        exit(1);
    }

    *token = '\0';
    *userName = text;
    return token + 1;
}
char *extractUserId(char *text, unsigned int *userId,unsigned int lineNumber){
    //strlen +1 , weil es bis zum terminating null byte gehen soll
    for(int i1 = 0; i1 < (strlen(text)+1); i1++){
        for(int i2 = 0; i2 < whitespaceCharactersSize; i2++){
            if(text[i1] == whitespaceCharacters[i2]){
                text[i1] = '\0';
                unsigned int id = atoi(text);
                if(id == 0){
                    printf("\n%s \"%u\" %s %s \"%s\" %s \"%c\" %s \"%u\"\nZeile: \"%s\"\n",
                           "Error: readConverterfile->extractUserId: Zeile: ",
                           lineNumber,
                           "Die Konvertierung der UserId ist entweder fehlgeschlagen(wenn das passiert ist ein bug drin, weil das zuerst gecheckt wurde)",
                           " oder die UserId ist eine 0. UserId:",
                           text,
                           ". Zeichen: ",
                           text[i1],
                           "Stelle: ",
                           i1,
                           completeLine);
                    exit(1);
                }
                *userId = id;
                return text + i1 +1;
            }
        }

        if(!isNumber(text[i1])){
            printf("\n%s \"%u\" %s \"%s\" %s \"%c\" %s \"%u\"\nZeile: \"%s\"\n",
                   "Error: readConverterfile->extractUserId: Zeile: ",
                   lineNumber,
                   "Die UserId enthält einen  Zeichen die keine Zahl ist. UserId:",
                   text,
                   ". Zeichen: ",
                   text[i1],
                   "Stelle: ",
                   i1+1,
                   completeLine);
            exit(1);
        }
    }

    printf("\n%s \"%u\" %s \"%s\" \n",
           "Error: readConverterfile->extractUserId: Zeile: ",
           lineNumber,
           "Die UserId enthält kein Escape Zeichen, weshalb das Programm nicht weiss wann die UserId aufhört. UserId:",
           completeLine);
    exit(1);
}

char *extractIdentifier(char *text, __attribute__((unused)) char **identifier, unsigned int lineNumber){
    char *begin = skipBeginning(text);
    if(begin[1] != '|'){
        printf("Error: readConverterfile->extractIdentifier: Zeilennumer: \"%u\""
               ". Zweites Zeichen muss ein \"|\". Es ist jedoch ein \"%c\"\nZeile: \"%s\"\n",
               lineNumber,
               begin[1],
               completeLine);
        exit(1);
    }
    if(begin[0] == 'u' || begin[0] == 'g'){
        begin[1] = '\0';
        *identifier = begin;//
        return begin + 2;
    }else{
        printf("Error: readConverterfile->extractIdentifier: Zeilennummer:%u"
               ". Erstes Zeichen muss ein \"u\" oder ein \"g\". Es ist jedoch ein \"%c\"\nZeile: \"%s\"\n",
               lineNumber,
               begin[0],
               completeLine);
        exit(1);
    }
}



bool extractIsComment(char *text){
    char *begin = skipBeginning(text);
    if(begin[0] == '#'){
        return true;
    }else{
        return false;
    }

}

void removeCommentInLine(char *text){
    char *pointer = strchr(text,'#');
    //pointer = strtok(text,"#");
    if(pointer != NULL){
        size_t length = strlen(pointer);
        memset(pointer, '\0',length);
    }
}


void readConverterFile(char *path){
    bool isAtEnd = false;
    FILE *fp = fopen(path,"r");
    if(fp == NULL){
        printf("problem\n");
    }

    size_t readedItems;
    unsigned int wi1 = 0;

    unsigned int gesamtTextMaxSize = 1000;
    unsigned int gesamtTextSize = 0;
    char *gesamtText = malloc(gesamtTextMaxSize);
    unsigned int sizeForARun = 1000;

    while(!isAtEnd){
        //


        /*im if(nr 1) wird zuerst kontrolliert ob gesamTextMaxSize gleich gross wie UINT_MAX ist, wenn ja wird ein fehler ausgegeben dass die datei zu gross ist.,
         * weil es der Buffer gesamtText müsste über diese Grenze(UINT_MAX) erhöht werden und das muss verhinder werden weil hier alles auf 4 Byte limitiert ist.
         *
         * für gesamtTextMaxSize kann auch nicht größer als UINT_MAX werden, dass ist sichergestellt in den ifs nr 1.1 und nr 1.2.1
         * in if nr 1.1 wird sichergestellt dass gesamtTextSize + sizeForARun maximal UINT_MAX betragen kann, diese prüfung ist vor einem wrap around sicher,
         * weil ich prüfe ob der rest aussreicht.
         * in if nr 1.2.1 wird geprüft ob die verdoppelung von gesamtTextMaxSize über die Grenze von UINT_MAX hinausgeht, wenn ja
         * wird gesamtTextMaxSize auf UINT_MAX festgelegt ansonsten wird es verdoppelt und UINT_MAX bestimmt die allokierte und
         * dadurch maximale Größe von gesamtText
         *
         * im if nr 1.2 wird geprüft ob gesamtTextSize + sizeForARun größer wird als gesamtTextMaxSize, wenn dass passiert
         * muss der Buffer gesamtText vergrößert werden.
         *
         * sizeForARun ist sicher vor einem wrap arround. Weil es entweder eine fest eingetragene Zahl ist oder es bekommt
         * eine Zahl die kleiner als  UINT_MAX  zugewiesen, es kann in diesem Fall auch kein negativer wrap around stattfinden,
         * weil gesamtTextSize auch nicht größer als UINT_MAX werden kann.
         * in if nr 1.1
         *
         * gesamtTextSize ist sicher vor einem wrap arround, weil es entweder immer um sizeForARun erhöht wird und
         *  wenn gesamtTextSize + sizeForARun größer wird als UINT_MAX, wird sizeForARun so verkleinert, dass
         *  gesamtTextSize + sizeForARun = UINT_MAX ist, dadurcg wird gesamtTextMaxSize auch auf UINT_MAX gesetzt
         *  und im nächsten durchlauf wird es im if nr 1 bemerkt und den fehler zu grosse datei ausgeben
         */
        if(! (gesamtTextMaxSize == UINT_MAX) ) {//if: nr 1

            if((UINT_MAX - gesamtTextSize) < sizeForARun){//if: nr 1.1
                sizeForARun = UINT_MAX - gesamtTextSize;
            }


            if (gesamtTextSize + sizeForARun > gesamtTextMaxSize) {//if nr 1.2
                unsigned int oldGesamtTextMaxSize = gesamtTextMaxSize;
                if ((UINT_MAX - gesamtTextMaxSize) < gesamtTextMaxSize) {//if nr 1.2.1
                    gesamtTextMaxSize = UINT_MAX;
                }else {
                    gesamtTextMaxSize = gesamtTextMaxSize * 2;
                }
                gesamtText = realloc(gesamtText, sizeof(char) * gesamtTextMaxSize);
                if (gesamtText == NULL) {
                    printf("\n\nEs kam zu einem unbekannten Problem mit realloc. %s. Errno Code: %i",
                           "Es konnte kein weiterer Speicher für das einlesen der Konverter Datei allokiert werden",
                           errno);
                }
                memset(gesamtText + (oldGesamtTextMaxSize), 0, gesamtTextMaxSize - oldGesamtTextMaxSize);
            }

            readedItems = fread(gesamtText + (wi1 * sizeForARun), sizeof(char), sizeForARun, fp);
            if (readedItems < sizeForARun) {
                if (feof(fp) != 0) {
                    isAtEnd = true;
                }
                if (ferror(fp) != 0) {
                    printf("\n\nError: Beim einlesen der Konverter Datei ist in der Funktion fread ein Fehler aufgetreten. Errno % d",
                           errno);
                    exit(1);
                }
            }
            gesamtTextSize = gesamtTextSize + readedItems;

        }else{
            printf("\n\nError: Die Konverter Datei ist zu gross, sie darf nicht 4294967295 Zeichen überschreiten.\n");
            exit(1);
        }
        wi1++;
    }
    printf("\n%s", gesamtText);


    /* der konverter wurde komplett eingelesen jetzt wird der text zu zeilen konvertiert
     */
    wi1 = 0;

    unsigned int linesMaxCount = 10;
    unsigned int linesCount = 0;
    char **lines = malloc(sizeof(char *) * linesMaxCount);
    char *token;
    token = strtok(gesamtText,"\n");
    while(token != NULL){
        if((linesCount + 1) == linesMaxCount){
            linesMaxCount = linesMaxCount * 2;
            lines = realloc(lines,sizeof(char *) * linesMaxCount);
            if(lines == NULL){
                printf("Error: Zeile: \"%u\":readConverterFile->realloc\n", wi1);
                exit(1);
            }
        }

        lines[linesCount] = token;
        linesCount++;
        token = strtok(NULL, "\n");
        wi1++;
    }

    userIdConverter = malloc(sizeof(IdConverter) * userIdConverterMaxSize);
    groupIdConverter = malloc(sizeof(IdConverter) * groupIdConverterMaxSize);
    userIdConverterSize = 0;
    groupIdConverterSize = 0;
    userIdConverterMaxSize = 10;
    groupIdConverterMaxSize = 10;


    for(unsigned int i1 = 0;i1 < linesCount;i1++){
        char *line = malloc(sizeof(char)*(strlen(lines[i1] + 1)));
        char *linePointerForFree = line;
        strcpy(line, lines[i1]);
        completeLine = lines[i1];
        if(!extractIsComment(line)){
            char *identifier = NULL;
            char *linuxUserName;
            unsigned int linuxId;
            char *eos32UserName;
            unsigned int eos32Id;

            removeCommentInLine(line);

            if(!isCheckLineHas2Colons(line)){
                printf("Error(readConverterFile):Zeilennumer:\"%u\". Zeile muss 2x ein Doppelpunkt(\":\") Zeichen haben, als Trenner zwischen "
                       "Benutzername und der Benutzerid\nZeile: \"%s\"\n",
                       i1+1,
                       line);
                exit(1);
            }

            line = skipBeginning(line);
            line = extractIdentifier(line, &identifier, i1+1);
            line = extractUserName(line, &linuxUserName, i1+1);
            line = extractUserId(line, &linuxId, i1+1);
            line = skipBeginning(line);
            line = extractUserName(line, &eos32UserName, i1+1);
            line = extractUserId(line, &eos32Id, i1+1);

            IdConverter **idConverter;
            unsigned int *idConverterMaxSize;
            unsigned int *idConverterSize;

            if(*identifier == 'u'){
                idConverter = &userIdConverter;
                idConverterSize = &userIdConverterSize;
                idConverterMaxSize = &userIdConverterMaxSize;
            }else{
                idConverter = &groupIdConverter;
                idConverterSize = &groupIdConverterSize;
                idConverterMaxSize = &groupIdConverterMaxSize;
            }

            if((*idConverterSize +1) == *idConverterMaxSize){
                *idConverterMaxSize = *idConverterMaxSize * 2;
                *idConverter = realloc(*idConverter, sizeof(IdConverter) * *idConverterMaxSize);
            }

            if(*identifier == 'u'){
                if(getFuseUserIdByEos32IdPrivate(eos32Id) != -1){
                    printf("%s \"%d\" %s \"%u\"\n",
                           "Error:readConverterFile: Zeile:",
                           i1,
                           "Die UserId(eos32/rechte hälfte) ist doppelt und kann nicht ein weiteres mal verwendet werden. UserId:",
                           eos32Id);
                    exit(1);
                }
                if(getEos32UserIdByFuseIdPrivate(linuxId) != -1){
                    printf("%s \"%d\" %s \"%u\"\n",
                           "Error:readConverterFile: Zeile:",
                           i1,
                           "Die UserId(linux/linke hälfte) ist doppelt und kann nicht ein weiteres mal verwendet werden. UserId:",
                           linuxId);
                    exit(1);
                }
            }else{
                if(getFuseGroupIdByEos32IdPrivate(eos32Id) != -1){
                    printf("%s \"%d\" %s \"%u\"\n",
                           "Error:readConverterFile: Zeile:",
                           i1,
                           "Die GroupId(eos32/rechte hälfte) ist doppelt und kann nicht ein weiteres mal verwendet werden. GroupId:",
                           eos32Id);
                    exit(1);
                }
                if(getEos32GroupIdByFuseIdPrivate(linuxId) != -1){
                    printf("%s \"%d\" %s \"%u\"\n",
                           "Error:readConverterFile: Zeile:",
                           i1,
                           "Die GroupId(linux/linke hälfte) ist doppelt und kann nicht ein weiteres mal verwendet werden. GroupId:",
                           linuxId);
                    exit(1);
                }
            }


            (*idConverter)[*idConverterSize].eos32Id = eos32Id;
            (*idConverter)[*idConverterSize].linuxId = linuxId;
            *idConverterSize = userIdConverterSize + 1;
        }
        free(linePointerForFree);
    }


    free(gesamtText);
    free(lines);
    int fcloseReturn = fclose(fp);
    if(fcloseReturn != 0 && fcloseReturn != EOF){
        printf("Error: readConverterFile: Stream konnte nicht geschlossen werden\nErrno: %d\n", fcloseReturn);
        exit(1);
    }

}
