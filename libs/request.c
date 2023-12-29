#if !defined(INCLUDES)
#define INCLUDES

    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <curl/curl.h>
    #include <cjson/cJSON.h>
    #include <stdarg.h>

    #define STRING_SIZE 200
    #define BUFFER_SIZE 1024
    #define TOKEN_CHAR "\""

#endif // INCLUDES


const int FILE_FLAG = 0;
const int JSON_FLAG = 1;
const int STRING_FLAG = 2;


struct postRequest
{
    size_t header_size;
    size_t body_size;
    char * headers;
    char * body;
};

struct response
{
    char * data;
    size_t size;
};

struct getRequest
{
    char * body;
    size_t bodySize;
};
 
struct string
{
    char * str;
    size_t length;
    size_t max_size;
};

struct fileStruct
{
    FILE * fp;
    char * fileName;
    char * mode;
};


/* INIT Functions */

struct fileStruct initFileStruct(const char * FILE_NAME, const char * mode)
{
    struct fileStruct fl = {NULL, "", ""};
    if ((fl.fp = fopen(FILE_NAME, mode)) == NULL)
        fprintf(stderr, "initFileStruct(): Error while opening the file: %s\n\n", FILE_NAME);
    

    // fl.fileName = (char *)calloc(strlen(FILE_NAME) + 1, sizeof(char));
    // memcpy(fl.fileName, FILE_NAME, strlen(FILE_NAME) + 1);

    
    fl.fileName = FILE_NAME;
    fl.mode = mode;

    return fl;
}

struct response initResponse()
{
    struct response res;
    res.data = (char *)calloc(STRING_SIZE, sizeof(char));
    res.size = STRING_SIZE * sizeof(char);
    return res;
}

struct string initSTRING(const size_t size)
{
    struct string buf;

    if (size <= 0)
    {
        fprintf(stderr, "initBUFFER: size <= 0\n\n");
        exit(-1);
    }

    buf.str = (char *)calloc(size + 1, sizeof(char));
    buf.length = 0;
    buf.max_size = size;

    if (buf.str == NULL)
    {
        fprintf(stderr, "initBUFFER: Out of memory!\n\n");
        exit(-1);
    }

    return buf;
}

struct postRequest initPOST()
{
    struct postRequest x;
    x.body = (char *)calloc(STRING_SIZE, sizeof(char));
    x.body_size = STRING_SIZE;
    x.headers = (char *)calloc(STRING_SIZE, sizeof(char));
    x.header_size = STRING_SIZE;
    return x;
}

struct getRequest initGetRequest()
{
    struct getRequest x;
    x.body = (char *)calloc(STRING_SIZE, sizeof(char));
    x.bodySize = STRING_SIZE * sizeof(char);
    return x;
}


/* PRINT Functions */

void print_response(struct response * res)
{
    printf("data: %s\n", res->data);
    printf("data size: %ld\n", res->size);
}

void print_getRequest(struct getRequest * x)
{
    printf("body: %s\n", x->body);
    printf("body size: %ld\n", x->bodySize);
}


/* DELELTE Functions */

void deleteResponse(struct response * res)
{
    if (res->data != NULL)
        free(res->data);

    res->size = 0;
}

void deleteGetRequest(struct getRequest * x)
{
    if (x->body != NULL)
        free(x->body);
    x->bodySize = 0;
}

void deleteSTRING(struct string * buf)
{
    if (buf->str != NULL)
        free(buf->str);
    buf->max_size = 0;
}

void deleteFileStruct(const struct fileStruct * fl)
{
    if (fl->fp != NULL) 
    {
        fclose(fl->fp);
    } 

    if (fl->fileName != NULL)
    {
        free(fl->fileName);
    }
}


/* LibCurl callback functions */

size_t getResponseToString(void * data, size_t size, size_t nmeb, void * clientp)
{
    struct response * res = (struct response *)clientp;
    size_t realsize = size * nmeb;

    // alocando espaço para os novos caracteres
    char * string = realloc(res->data, res->size + realsize + 1);
    if (string == NULL)
    {
        puts("Out of memory!\n");
        return 0;
    }

    // atualizando o ponteiro para a nova string
    res->data = string;

    // copiando os dados recebidos em data para a response string
    strcat(res->data, data);

    // atualizando o tamanho da response string
    res->size += realsize;

    return realsize;
}

size_t getResponseToFile(void * data, size_t size, size_t nmeb, void * clientp)
{
    size_t realsize = (size * nmeb);
    FILE * arq = (FILE *)clientp;

    if (arq == NULL)
    {   

        fprintf(stderr, "Error while opening file.");
        return 0;
    }
    else
    {
        puts("Writing data...");
        fwrite(data, size, realsize, arq);
    }
    
    return realsize;
}


/* extract, format and dynamic allocate an string from an JSON file */

char * getDynamicJSONString(const char key[], const char * json_input)
{
    cJSON * json;
    char * value;
    char * valueFormated;
    char buffer[BUFFER_SIZE] = "";

    json = cJSON_Parse(json_input);
    
    value = cJSON_Print(cJSON_GetObjectItem(json, key));
    if (value == NULL)
    {
        fprintf(stderr, "getDynamicJSONString(): Key not finded: %s\n\n", key);
        return NULL;
    }
    else
    {
        strcpy(buffer, value);
        valueFormated = strtok(buffer, TOKEN_CHAR);

        char * newString = (char *)calloc(strlen(valueFormated) + 1, sizeof(char));
        if (newString == NULL)
        {
            fprintf(stderr, "getDynamicJSONString(): Out of memory!");
            return NULL;
        }
        else
        {
            strcpy(newString, valueFormated);
            return newString;
        }
    }
}


/* POST methods */

char * postToString(const char URL[], const char body[], const char headers[])
{
    struct response res = initResponse();

    CURL * handle = curl_easy_init();
    curl_easy_setopt(handle, CURLOPT_URL, URL);
    curl_easy_setopt(handle, CURLOPT_POST, 1L); // 1L == x-www-form-urlencoded
    curl_easy_setopt(handle, CURLOPT_POSTFIELDS, body);

    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, getResponseToString);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void *)&res);

    CURLcode result = curl_easy_perform(handle);
    if (result != CURLE_OK)
    {
        fprintf(stderr, "Error: %s\n", curl_easy_strerror(result));
        return NULL;
    }
    else
    {
        printf("Token obtained: %s\n", res.data);
        return res.data;
    }
}


/* GET methods */

void getToFile(const char URL[], const struct fileStruct * fl)
{
    CURL * handle = curl_easy_init();
    curl_easy_setopt(handle, CURLOPT_URL, URL);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, getResponseToFile);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, fl->fp);

    CURLcode result = curl_easy_perform(handle);
    if (result != CURLE_OK)
    {
        fprintf(stderr, "Error: %s\n", curl_easy_strerror(result));
        return;
    }

    fflush(fl->fp);
    curl_easy_cleanup(handle);
}

void getToJSONFile(const char URL[], const struct fileStruct * fl)
{
    cJSON * json;
    struct response res = initResponse();

    CURL * handle = curl_easy_init();
    curl_easy_setopt(handle, CURLOPT_URL, URL);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, getResponseToString);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void *)&res);

    CURLcode result = curl_easy_perform(handle);
    if (result != CURLE_OK)
    {
        fprintf(stderr, "Error: %s\n", curl_easy_strerror(result));
        return;
    }

    json = cJSON_Parse(res.data);
    fprintf(fl->fp, cJSON_Print(json));

    deleteResponse(&res);
    cJSON_Delete(json);
    curl_easy_cleanup(handle);
    fflush(fl->fp);
}

void getToString(const char URL[], struct response * res)
{
    CURL * handle = curl_easy_init();
    curl_easy_setopt(handle, CURLOPT_URL, URL);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, getResponseToString);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, res);

    CURLcode result = curl_easy_perform(handle);
    if (result != CURLE_OK)
    {
        fprintf(stderr, "Error: %s\n", curl_easy_strerror(result));
        return;
    }

    curl_easy_cleanup(handle);
}


/* realloc the fstr->str to fit new amount of data */

char * resize(char * str, const size_t length)
{
    if (str != NULL && length > 0)
    {
        char * tempPtr = (char *)realloc(str, length + 1);
        if (tempPtr != NULL)
            return tempPtr;
        else
        {
            fprintf(stderr, "resize(): tempPtr == NULL");
            exit(-1);
        }

    }
    else if (str == NULL)
    {
        fprintf(stderr, "resize(): str == NULL");
        return str;
    }
    else
    {
        fprintf(stderr, "resize(): length <= 0");
        return str;
    }
}


/* Copy an string (str) into the formated string (fstr->str) used in makeURL(). Also verify if it fits, if not, it call resize() to realloc fstr.str; */

void copystr(struct string * fstr, const char * str, const size_t str_length)
{
    if ((fstr->length + str_length) >= fstr->max_size)
    {
        fstr->str = resize(fstr->str, fstr->max_size + str_length);
        fstr->max_size += str_length;
    }

    strcat(fstr->str, str);
    fstr->length += str_length;
}


/* Like printf(), you pass the basic url with the type identifiers where it should be replaced by the respective variable passed to the function. */
char * makeURL(const char strArg[], ...)
{
    size_t ARG_Size = strlen(strArg);
    size_t buffer_length = 0;
    struct string fstr = initSTRING(ARG_Size + 1);
    char * buffer = (char *)calloc(BUFFER_SIZE + 1, sizeof(char));

    if (buffer == NULL) {
        fprintf(stderr, "makestr(): buffer == NULL");
        exit(-1);
    }

    va_list list;
    va_start(list, strArg);

    char * ch;
    int Findex = 0;
    for (size_t i = 0; i < ARG_Size; i++)
    {
        ch = &strArg[i];
        if (*ch != '%')
        {
            snprintf(buffer, BUFFER_SIZE, "%c", *(&strArg[i]));
            copystr(&fstr, buffer, 1);
            fstr.length++;
            Findex++;
            continue;
        }

        ch++; i++;
        switch (*ch)
        {
        case 'd':
            snprintf(buffer, BUFFER_SIZE, "%d", va_arg(list, int));
            buffer_length = strlen(buffer);
            copystr(&fstr, buffer, buffer_length);
            Findex = fstr.length; // updating the index
            break;

        case 'f':
            snprintf(buffer, BUFFER_SIZE, "%f", va_arg(list, double));
            buffer_length = strlen(buffer);
            copystr(&fstr, buffer, buffer_length);
            Findex = fstr.length; // updating the index
            break;

        case 's':

            char * tempStr = va_arg(list, char *);
            if (strlen(tempStr) > 0)
            {
                size_t temp_length = strlen(tempStr);
                copystr(&fstr, tempStr, temp_length);
                Findex = fstr.length; // updating the index
            }


            break;

        case 'c':
            snprintf(buffer, BUFFER_SIZE, "%c", va_arg(list, int));
            buffer_length = strlen(buffer);
            copystr(&fstr, buffer, buffer_length);
            Findex = fstr.length; // updating the index
            break;

        case 'l':

            switch (*(ch + 1))
            {
            case 'd':
                snprintf(buffer, BUFFER_SIZE, "%ld", va_arg(list, long int));
                buffer_length = strlen(buffer);
                copystr(&fstr, buffer, buffer_length);
                Findex = fstr.length; // updating the index
                break;

            case 'f':
                snprintf(buffer, BUFFER_SIZE, "%lf", va_arg(list, double));
                buffer_length = strlen(buffer);
                copystr(&fstr, buffer, buffer_length);
                Findex = fstr.length; // updating the index
                break;

            default:
                fprintf(stderr, "makestr(): invalid indentifier: %c%c%c\n\n", *(ch - 2), *(ch - 1), *ch);
                break;
            }

            ch++;
            break;

        default:
            fprintf(stderr, "makestr(): invalid indentifier: %c%c\n\n", *(ch - 1), *ch);
            break;
        }

        // cleaning buffer
        buffer_length = 0;
        strcpy(buffer, "");
    }

    free(buffer);
    va_end(list);
    return fstr.str;
}


/* wrapper of the GET methods. Pass the URL, the type of output (fileStruct, response) and the flag (JSON, FILE, STRING) */
void GETRequest(const char URL[], void * output, const int flag)
{
    if (FILE_FLAG == flag || JSON_FLAG == flag || STRING_FLAG == flag)
    {
        if (FILE_FLAG == flag || JSON_FLAG == flag) // checking for files
        {
            struct fileStruct * fl = (struct fileStruct *)output;
            if (fl->fp == NULL)
            {
                fprintf(stderr, "File was not initialized.\n\n");
                if (fl->fileName != NULL)
                {
                    printf("Trying to open the file.\n\n");
                    FILE * fileTemp = fopen(fl->fileName, fl->mode);
                    if (fileTemp == NULL)
                    {
                        fprintf(stderr, "Was not possible to open the file.\n\n");
                        exit(-1);
                    }
                }
                else
                {
                    fprintf(stderr, "None file name was provided.\n\n");
                    exit(-1);
                }
            }
            else if (strcmp(fl->mode, "w") == 0 || strcmp(fl->mode, "a") == 0)
            {
                if (FILE_FLAG == flag) {
                    getToFile(URL, fl);
                }
                else if (JSON_FLAG == flag) {
                    getToJSONFile(URL, fl);
                }               
            }
            else
            {
                fprintf(stderr, "Could not write in the file (might be opened in readmode)\n\n.");
                fprintf(stderr, "fl->mode value: %s\n\n", fl->mode);
                return;
            }
        }
        else if (STRING_FLAG == flag) {
            getToString(URL, (struct response *)output);
        }
    }
    else // wrong flag
    {
        fprintf(stderr, "Invalid flag was passed to the function: %d \n\n", flag);
        exit(-1);
    }
}


/* get the size of an file */

size_t file_size(FILE * arq)
{
    if (arq == NULL)
    {
        fprintf(stderr, "File pointer == NULL");
        return -1;
    }

    size_t size = 0;
    fseek(arq, SEEK_SET, SEEK_END);
    size = ftell(arq); rewind(arq);

    return size;
}