#if !defined(INCLUDES)
    #define INCLUDES
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <cjson/cJSON.h>

    #define STRING_SIZE 300
    #define BUFFER_SIZE 1024

#endif // INCLUDES

#include <stdbool.h>
#include <time.h>
#include "../libs/request.h"

struct GeocodingData_t
{
    char * city;
    char * country; /* ISO code */
    char * state;
    char * lon;
    char * lat;
};

struct APIData_t
{   
    struct GeocodingData_t geo;
    char * appid;
    char * units;
};

struct currentWeather_t
{
    char * date;
    char * name;
    char * country;
    char * weather_main;
    char * weather_description;
    char * sunset;
    char * sunrise;

    float temp;
    float feels_like;
    float temp_min;
    float temp_max;
    float humidity;
};


struct GeocodingData_t initGeo()
{
    struct GeocodingData_t geo;
    geo.city = (char *)calloc(STRING_SIZE + 1, sizeof(char));
    geo.country = (char *)calloc(STRING_SIZE + 1, sizeof(char));
    geo.state = (char *)calloc(STRING_SIZE + 1, sizeof(char));
    geo.lon = (char *)calloc(STRING_SIZE + 1, sizeof(char));
    geo.lat = (char *)calloc(STRING_SIZE + 1, sizeof(char));

    return geo;
}

struct APIData_t initData()
{
    struct APIData_t apidata;
    apidata.geo = initGeo();

    apidata.appid = (char *)calloc(STRING_SIZE + 1, sizeof(char));
    apidata.units = (char *)calloc(STRING_SIZE + 1, sizeof(char));

    return apidata;
}


void deleteGeo(struct GeocodingData_t * geo)
{   
    if (geo->city != NULL)
        free(geo->city);
    
    if (geo->state != NULL)
        free(geo->state);
    
    if (geo->country != NULL)
        free(geo->country);
    
    if (geo->lat != NULL)
        free(geo->lat);
    
    if (geo->lon != NULL)
        free(geo->lon);
}

void deleteAPIData_t(struct APIData_t * data)
{
    if (data->appid != NULL)
        free(data->appid);

    if (data->units != NULL)
        free(data->units);
    
    deleteGeo(&data->geo);
}

void deleteCurrentWeather_t(struct currentWeather_t * cW)
{
    if (cW->name != NULL)
        free(cW->name);

    if (cW->country != NULL)
        free(cW->country);

    if (cW->weather_main != NULL)
        free(cW->weather_main);
    
    if (cW->weather_description != NULL)
        free(cW->weather_description);

    // if (cW->date != NULL)
    //     free(cW->date);

    // if (cW->sunrise != NULL)
    //     free(cW->sunrise);
    
    // if (cW->sunset != NULL)
    //     free(cW->sunrise);

    cW->temp = 0.0f;
    cW->feels_like = 0.0f;
    cW->temp_min = 0.0f;
    cW->temp_max = 0.0f;
    cW->humidity = 0.0f;
}

/* search for an specific object in info.json */

void requestForJSON(char str[], const char key[])
{
    char * returnString = NULL;
    FILE * arq = fopen("/app/info.json", "r");
    if (arq == NULL)
    {
        fprintf(stderr, "info.json was not found.\n\n");
        return;
    }

    size_t buffer_size = file_size(arq);
    if (buffer_size <= 0)
    {   
        fprintf(stderr, "info.json is empty.\n\n");
        fclose(arq);
        return;
    }
    
    char * buffer = (char *)calloc(buffer_size + 1, sizeof(char));
    fread(buffer, buffer_size, 1, arq);

    cJSON * info = cJSON_Parse(buffer);

    if (cJSON_HasObjectItem(info, key))
    {
        returnString = cJSON_Print(cJSON_GetObjectItemCaseSensitive(info, key));
        strcln(returnString);
        strcpy(str, returnString);
    }
    else
    {
        fprintf(stderr, "%s was not found in info.json.\n\n", key);
    }
    

    fclose(arq);
    free(returnString);
    free(buffer);
    cJSON_Delete(info);
}


/* GET Requests */

struct string GETCurrentWeather(const struct APIData_t apidata)
{ 
    char * URL = makeURL(
        "https://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s&units=%s",
        apidata.geo.lat, apidata.geo.lon, apidata.appid, apidata.units
    );

    struct string res = initSTRING(STRING_SIZE + 1);
    GETRequest(URL, (void *)&res, STRING_FLAG);

    free(URL);
    return res;
}

struct string GETGeocoding(const struct APIData_t data)
{
    char * URL = makeURL(
        "http://api.openweathermap.org/geo/1.0/direct?q=%s,%s&appid=%s",
        data.geo.city, data.geo.country, data.appid
    );

    struct string res = initSTRING(STRING_SIZE);
    GETRequest(URL, (void *)&res, STRING_FLAG);

    free(URL);
    return res;
}


/* Read responses */

struct currentWeather_t readCurrentWeather(struct string res)
{
    cJSON * json;
    cJSON * main;
    cJSON * weather;
    cJSON * sys;

    struct currentWeather_t cW;

    json = cJSON_Parse(res.str);

    if (json == NULL) {
        fprintf(stderr, "readCurrentWeather(): json == NULL\n\n"); exit(-1); 
    }
    main = cJSON_GetObjectItem(json, "main");
    sys = cJSON_GetObjectItem(json, "sys");

    // weather info comes into an array, so we need to get the first element
    // first to have access to the data
    weather = cJSON_GetObjectItem(json, "weather");
    weather = cJSON_GetArrayItem(weather, 0);

    cW.name = cJSON_Print(cJSON_GetObjectItem(json, "name"));
    cW.country = cJSON_Print(cJSON_GetObjectItem(sys, "country"));
    cW.weather_main = cJSON_Print(cJSON_GetObjectItem(weather, "main"));
    cW.weather_description = cJSON_Print(cJSON_GetObjectItem(weather, "description"));

    strcln(cW.name);
    strcln(cW.country);
    strcln(cW.weather_main);
    strcln(cW.weather_description);

    /* avoiding data leak */
    char * dt_buff = cJSON_Print(cJSON_GetObjectItem(json, "dt"));

    char * temp_buff = cJSON_Print(cJSON_GetObjectItem(main, "temp"));
    char * feels_like_buff = cJSON_Print(cJSON_GetObjectItem(main, "feels_like"));
    char * temp_min_buff = cJSON_Print(cJSON_GetObjectItem(main, "temp_min"));
    char * temp_max_buff = cJSON_Print(cJSON_GetObjectItem(main, "temp_max"));
    char * humidity_buff = cJSON_Print(cJSON_GetObjectItemCaseSensitive(main, "humidity"));
    
    char * sunset_buff = cJSON_Print(cJSON_GetObjectItem(sys, "sunset"));
    char * sunrise_buff = cJSON_Print(cJSON_GetObjectItem(sys, "sunrise"));

    cW.temp = atof(temp_buff);
    cW.feels_like = atof(feels_like_buff);
    cW.temp_min = atof(temp_min_buff);
    cW.temp_max = atof(temp_max_buff);
    cW.humidity = atof(humidity_buff);
    

    time_t sunset_t = atoll(sunset_buff);
    cW.sunset = ctime(&sunset_t);

    time_t sunrise_t = atoll(sunrise_buff);
    cW.sunrise = ctime(&sunrise_t);


    time_t dt_ = atoll(dt_buff);
    cW.date = ctime(&dt_);

    free(temp_buff);
    free(feels_like_buff);
    free(temp_min_buff);
    free(temp_max_buff);
    free(humidity_buff);
    free(dt_buff);
    free(sunset_buff);
    free(sunrise_buff);
    cJSON_Delete(json);

    return cW;
}

struct GeocodingData_t readGeocoding(struct string res)
{
    struct GeocodingData_t geo = {NULL};
    cJSON * arrayJSON = cJSON_Parse(res.str);
    cJSON * responseJSON = cJSON_GetArrayItem(arrayJSON, 0);

    if (responseJSON == NULL) {
        fprintf(stderr, "readGeocoding(): responseJSON == NULL.\n\n");
        return geo;
    }

    geo.city = cJSON_Print(cJSON_GetObjectItemCaseSensitive(responseJSON, "name"));
    geo.state = cJSON_Print(cJSON_GetObjectItemCaseSensitive(responseJSON, "state"));
    geo.country = cJSON_Print(cJSON_GetObjectItemCaseSensitive(responseJSON, "country"));
    geo.lon = cJSON_Print(cJSON_GetObjectItemCaseSensitive(responseJSON, "lon"));
    geo.lat = cJSON_Print(cJSON_GetObjectItemCaseSensitive(responseJSON, "lat"));
    
    strcln(geo.city);
    strcln(geo.country);
    strcln(geo.state);

    cJSON_Delete(arrayJSON);
    return geo;
}


/* 
[
    {
        "name": "Caieiras",
        "lat": -23.3644621,
        "lon": -46.7484765,
        "country": "BR",
        "state": "São Paulo"
    }
]
*/

void printCurrentWeather(const struct APIData_t data, const struct currentWeather_t cW)
{
    char * ch;
    if (strcmp(data.units, "metric") == 0) {
        ch = "°C";
    } else {
        ch = "°F";
    }

    printf(" %s | %s\n", cW.name, cW.country);
    printf(" %s\n", cW.date);
    printf(" %s: %s\n\n", cW.weather_main, cW.weather_description);

    printf(" Temperature: %f %s \t| Feels like: %f %s\n", cW.temp, ch, cW.feels_like, ch);
    printf(" Temperature min: %f %s \t| Temperature Max: %f %s\n", cW.temp_min, ch, cW.temp_max, ch);
    printf(" Humidity: %f\n\n", cW.humidity);

    printf(" Sunset: %s | Sunrise: %s\n", cW.sunset, cW.sunrise);
}



void saveNewArgs(struct APIData_t data)
{
    cJSON * infoJSON = NULL;

    FILE * arq = fopen("info.json", "r");
    if (arq == NULL) {
        fprintf(stderr, "info.json not found. Creating it...\n\n");
        infoJSON = cJSON_CreateObject();
    }
    else 
    {
        size_t buffer_size = file_size(arq);
        char * buffer = (char *)calloc(buffer_size + 1, sizeof(char));
        fread(buffer, buffer_size, 1, arq);

        infoJSON = cJSON_Parse(buffer);

        fclose(arq);
        free(buffer);
    }

    
    cJSON * geoJSON = cJSON_CreateObject();

    if (cJSON_HasObjectItem(infoJSON, "Geocoding")) 
    {
        cJSON_ReplaceItemInObjectCaseSensitive(infoJSON, "Geocoding", geoJSON);
    } 
    else
    {
        cJSON_AddItemToObject(infoJSON, "Geocoding", geoJSON);
    }

    cJSON_AddStringToObject(geoJSON, "city", data.geo.city);
    cJSON_AddStringToObject(geoJSON, "lat", data.geo.lat);
    cJSON_AddStringToObject(geoJSON, "lon", data.geo.lon);
    cJSON_AddStringToObject(geoJSON, "country", data.geo.country);
    cJSON_AddStringToObject(geoJSON, "state", data.geo.state);


    if (cJSON_HasObjectItem(infoJSON, "appid")) 
    {
        cJSON * appidJSON = cJSON_CreateString(data.appid);
        cJSON_ReplaceItemInObjectCaseSensitive(infoJSON, "appid", appidJSON);
    }
    else
    {
        cJSON_AddStringToObject(infoJSON, "appid", data.appid);
    }


    if (cJSON_HasObjectItem(infoJSON, "units"))
    {
        cJSON * units = cJSON_CreateString(data.units);
        cJSON_ReplaceItemInObjectCaseSensitive(infoJSON, "units", units);
    }
    else
    {
        cJSON_AddStringToObject(infoJSON, "units", data.units);
    }

    arq = fopen("info.json", "w");
    if (arq == NULL)
    {
        fprintf(stderr, "wasn't possible to open info.json in write mode.\n\n");
        return;
    }

    char * puts_buffer = cJSON_Print(infoJSON);
    fputs(puts_buffer, arq);

    fclose(arq);
    free(puts_buffer);
    cJSON_Delete(infoJSON);
}

void SearchInCache(struct APIData_t * data)
{   
    FILE * info = fopen("info.json", "r");
    if (info == NULL) {
        fprintf(stderr, "info.json not found.\n\n");
        return;
    }

    cJSON * infoJSON = NULL;
    size_t buffer_size = file_size(info);
    char * buffer = (char *)calloc(buffer_size + 1, sizeof(char));

    fread(buffer, buffer_size, 1, info);

    infoJSON = cJSON_Parse(buffer);
    if (cJSON_HasObjectItem(infoJSON, "appid"))
    {
        data->appid = cJSON_Print(cJSON_GetObjectItemCaseSensitive(infoJSON, "appid"));
        strcln(data->appid);
    }
    
    if (cJSON_HasObjectItem(infoJSON, "Geocoding"))
    {   
        cJSON * geoJSON = cJSON_GetObjectItemCaseSensitive(infoJSON, "Geocoding");

        data->geo.city = cJSON_Print(cJSON_GetObjectItemCaseSensitive(geoJSON, "city"));
        data->geo.state = cJSON_Print(cJSON_GetObjectItemCaseSensitive(geoJSON, "state"));
        data->geo.country = cJSON_Print(cJSON_GetObjectItemCaseSensitive(geoJSON, "country"));
        data->geo.lat = cJSON_Print(cJSON_GetObjectItemCaseSensitive(geoJSON, "lat"));
        data->geo.lon = cJSON_Print(cJSON_GetObjectItemCaseSensitive(geoJSON, "lon"));

        strcln(data->geo.city);
        strcln(data->geo.state);
        strcln(data->geo.country);
        strcln(data->geo.lat);
        strcln(data->geo.lon);
    }

    if (cJSON_HasObjectItem(infoJSON, "units"))
    {
        data->units = cJSON_Print(cJSON_GetObjectItemCaseSensitive(infoJSON, "units"));
        strcln(data->units);
    }
    else
    {
        data->units = NULL;
    }

    fclose(info);
    cJSON_Delete(infoJSON);
    free(buffer);
}


void SearchInTerminalArgs(struct APIData_t * data, int argc, char * argv[])
{
    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "--appid") == 0 || strcmp(argv[i], "-a") == 0)
        {
            strcpy(data->appid, argv[i + 1]);
            i++;
        }
        else if (strcmp(argv[i], "--city") == 0 || strcmp(argv[i], "-c") == 0)
        {
            strcpy(data->geo.city, argv[i + 1]);
            i++;
        }
        else if (strcmp(argv[i], "--country") == 0 || strcmp(argv[i], "-C") == 0)
        {
            strcpy(data->geo.country, argv[i + 1]);
            i++;
        }
        else if (strcmp(argv[i], "--units") == 0 || strcmp(argv[i], "-u") == 0)
        {
          strcpy(data->units, argv[i + 1]);
          i++;
        }
    }

    if (data->geo.city == NULL || data->geo.country == NULL)
    {
        if (data->geo.city == NULL) {
            fprintf(stderr, "City was not provided, impossible to continue.\n\n");
        }

        if (data->geo.country == NULL) {
            fprintf(stderr, "ISO Code was not provided, impossible to continue.\n\n");
        }
    }

    if (strcmp(data->units, "") == 0)
    {
        requestForJSON(data->units, "units");
        if (strcmp(data->units, "") == 0)
        {
            strcpy(data->units, "metric");
        }
    }

    if (strcmp(data->appid, "") == 0)
    {
        requestForJSON(data->appid, "appid");
        if (data->appid == NULL)
        {
            fprintf(stderr, "Appid was not provided and also not found in info.json.\n\n");
        }
    }
}

int checkArgs(const struct APIData_t data, const bool cache)
{
    int returnValue = 1;
    if (data.appid == NULL || strcmp(data.appid, "") == 0)
    {
        fprintf(stderr, "appid is missing...\n");
        returnValue = -1;
    }
    
    if (cache == false) 
    {
        if (data.geo.city == NULL || strcmp(data.geo.city, "") == 0)
        {
            fprintf(stderr, "city is missing...\n");
            returnValue = -1;
        }

        if (data.geo.country == NULL || strcmp(data.geo.country, "") == 0)
        {
            fprintf(stderr, "country is missing...\n");
            returnValue = -1;
        }
    }
   
    if (cache == true)
    {
        if (data.geo.lat == NULL || strcmp(data.geo.lat, "") == 0)
        {
            fprintf(stderr, "latitude is missing in the info.json ...\n");
            returnValue = -1;
        }
        
        if (data.geo.lon == NULL || strcmp(data.geo.lon, "") == 0)
        {
            fprintf(stderr, "longitute is missing in the info.json ...\n");
            returnValue = -1;
        }
    }

    return returnValue;
}


/*  TERMINAL ARGS
 *  
 *  --appid   or  -a  : OpenWeather API key
 *  --city    or  -c  : the city you live in (if it constais space, put it between "")
 *  --country or  -C  : ISO 3166 code for your country
 *  --units   or  -u  : choose between celsius and fahrenheit (default: celsius)
 * */


int main(int argc, char * argv[])
{   
    struct APIData_t data = {NULL};
    if (argc < 2) 
    {   
        printf("None args was passed, checking cache...\n\n");
        SearchInCache(&data);
        if (checkArgs(data, true) == 1)
        {
            struct string res = GETCurrentWeather(data);
            struct currentWeather_t cW = readCurrentWeather(res);

            printCurrentWeather(data, cW);
            
            deleteSTRING(&res);
            deleteAPIData_t(&data);
            deleteCurrentWeather_t(&cW);
        }
        else // important args missing (appid and geocoding) 
        {
            char * error_msg = "Some needed data was not found in cache. Please run the application again passing that data.\n\n";
            fprintf(stderr, error_msg);

            deleteAPIData_t(&data);
            return -1;
        }    
    }
    else
    {   
        data = initData();
        SearchInTerminalArgs(&data, argc, argv);
        if (checkArgs(data, false) == 1)
        {
            struct string resGeocoding = GETGeocoding(data);

            data.geo = readGeocoding(resGeocoding);

            struct string resCurrentWeather = GETCurrentWeather(data);
            struct currentWeather_t cW = readCurrentWeather(resCurrentWeather);

            printCurrentWeather(data, cW);

            saveNewArgs(data);

            deleteSTRING(&resCurrentWeather);
            deleteSTRING(&resGeocoding);
            deleteAPIData_t(&data);
            deleteCurrentWeather_t(&cW);
        }
        else 
        {
            char * error_msg = "Some importante arguments was not passed to the program. Please read the docs or pass the argument --help next time to get some instructions\n\n";
            fprintf(stderr, error_msg);
            deleteAPIData_t(&data);
            return -1;
        }
    }

    return 0;
}
