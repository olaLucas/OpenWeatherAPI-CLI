#if !defined(INCLUDES)
#define INCLUDES

    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <curl/curl.h>
    #include <cjson/cJSON.h>

    #define STRING_SIZE 200
    #define BUFFER_SIZE 1024

#endif // INCLUDES

extern const int FILE_FLAG;
extern const int JSON_FLAG;
extern const int STRING_FLAG;

struct response
{
    char * data;
    size_t size;
};

struct postRequest
{
    size_t header_size;
    size_t body_size;
    char * headers;
    char * body;
};

struct token
{
    char * access_token;
    char * token_type;
    size_t expires_in;
};

struct getRequest
{
    char * body;
    size_t bodySize;
};

struct fileStruct
{
    FILE * fp;
    char * fileName;
    char * mode;
};


/*  
    INIT Functions
*/
extern struct fileStruct initFileStruct(const char * FILE_NAME, const char * mode);
extern struct postRequest initPOST();
extern struct response initResponse();
extern struct getRequest initGetRequest();


/* 
    PRINT Functions
*/
extern void print_response(struct response * res);
extern void print_getRequest(struct getRequest * gt);


/*
    DELETE Functions
*/
extern void deletePOST(struct postRequest * htp);
extern void deleteResponse(struct response * res);
extern void deleteGetRequest(struct getRequest * gt);
extern void deleteFileStruct(const struct fileStruct * fl);


/* 
    extract, format and dynamic allocate an string from an JSON file 
*/
extern char * getDynamicJSONString(const char key[], const char * json_input);


/* 
    POST methods
*/
extern char * postToString(const char URL[], const char body[], const char headers[]);


/* 
    Like printf(), you pass the basic url with the type identifiers where it should be replaced by the respective variable passed to the function.
*/
extern char * makeURL(const char strArg[], ...);

/* 
    wrapper of the GET methods. Pass the URL, the type of output ((void *)&fileStruct, (void *)&response) and the flag (JSON, FILE, STRING) 
*/
extern void GETRequest(const char URL[], void * output, const int flag);


/* 
    get the size of an file
*/
extern size_t file_size(FILE * arq);