#if !defined(INCLUDES)
    #define INCLUDES
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <cjson/cJSON.h>

    #define STRING_SIZE 300
    #define BUFFER_SIZE 1024

#endif // INCLUDES

#include <time.h>
#include "../libs/request.h"

struct GeocodingData_t
{
    char * city;
    char * country;
    char * state;
    char * iso; /* ISO 3166 country codes. */
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

    float temp;
    float feels_like;
    float temp_min;
    float temp_max;
    float humidity;

    time_t sunset;
    time_t sunrise;
    time_t dt;
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
    free(geo->city);
    free(geo->state);
    free(geo->country);
    free(geo->lat);
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

    cW->temp = 0.0f;
    cW->feels_like = 0.0f;
    cW->temp_min = 0.0f;
    cW->temp_max = 0.0f;
    cW->humidity = 0.0f;
    cW->sunset = 0;
    cW->dt = 0;
    cW->sunrise = 0;
}


void checkCache(struct APIData_t * data)
{   
    FILE * info = fopen("/media/dio/code/git repositories/OpenWeatherAPI-CLI/info.json", "r");
    if (info == NULL) {
        fprintf(stderr, "info.json not found.\n\n");
        exit(-1);
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
    else
    {
        fprintf(stderr, "appid not found in cache. please inform it using --appid or -a flags next time.\n\n");
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

/* search for an specific object in info.json */

char * requestForJSON(const char key[])
{
    char * returnString = NULL;
    FILE * arq = fopen("/media/dio/code/git repositories/OpenWeatherAPI-CLI/info.json", "r");
    if (arq == NULL)
    {
        fprintf(stderr, "info.json was not found, may be your first time using the application. If so, you'll need to provide some informations first, check the documentation.\n\n");
        return NULL;
    }

    size_t buffer_size = file_size(arq);
    if (buffer_size <= 0)
    {   
        fprintf(stderr, "info.json is empty.\n\n");
        return NULL;
    }
    
    char * buffer = (char *)calloc(buffer_size + 1, sizeof(char));
    fread(buffer, buffer_size, 1, arq);

    cJSON * info = cJSON_Parse(buffer);

    if (cJSON_HasObjectItem(info, key))
    {
        returnString = cJSON_Print(cJSON_GetObjectItemCaseSensitive(info, key));
    }
    else
    {
        fprintf(stderr, "%s was not found in info.json.\n\n", key);
    }
    

    fclose(arq);
    free(buffer);
    cJSON_Delete(info);

    return returnString;
}


/* GET Requests */

struct string GETCurrentWeather(struct APIData_t apidata)
{
    char * URL = makeURL(
        "https://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s&units=%s",
        apidata.geo.lat, apidata.geo.lon, apidata.appid, apidata.units
    );

    struct string res = initSTRING(STRING_SIZE + 1);
    GETRequest(URL, (void *)&res, STRING_FLAG);

    return res;
}

struct string GETGeocoding(struct APIData_t data)
{
    char * URL = makeURL(
        "http://api.openweathermap.org/geo/1.0/direct?q=%s,%s&appid=%s",
        data.geo.city, data.geo.iso, data.appid);

    struct string res = initSTRING(STRING_SIZE);
    GETRequest(URL, (void *)&res, STRING_FLAG);

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
        fprintf(stderr, "readCurrentWeather(): json == NULL\n\n");
        exit(-1);
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
    char * dt = cJSON_Print(cJSON_GetObjectItem(json, "dt"));

    char * temp = cJSON_Print(cJSON_GetObjectItem(main, "temp"));
    char * feels_like = cJSON_Print(cJSON_GetObjectItem(main, "feels_like"));
    char * temp_min = cJSON_Print(cJSON_GetObjectItem(main, "temp_min"));
    char * temp_max = cJSON_Print(cJSON_GetObjectItem(main, "temp_max"));
    char * humidity = cJSON_Print(cJSON_GetObjectItemCaseSensitive(main, "humidity"));
    
    char * sunset = cJSON_Print(cJSON_GetObjectItem(sys, "sunset"));
    char * sunrise = cJSON_Print(cJSON_GetObjectItem(sys, "sunrise"));

    cW.temp = atof(temp);
    cW.feels_like = atof(feels_like);
    cW.temp_min = atof(temp_min);
    cW.temp_max = atof(temp_max);
    cW.humidity = atof(humidity);
    cW.dt = atoll(dt);
    cW.sunset = atoll(sunset);
    cW.sunrise = atoll(sunrise);
    cW.date = ctime(&cW.dt);

    free(temp);
    free(feels_like);
    free(temp_min);
    free(temp_max);
    free(humidity);
    free(dt);
    free(sunset);
    free(sunrise);
    cJSON_Delete(json);

    return cW;
}

struct GeocodingData_t readGeocoding(struct string res)
{
    struct GeocodingData_t geo = {NULL};
    cJSON * responseJSON = cJSON_Parse(res.str);

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
    strcln(geo.lon);
    strcln(geo.lat);
    strcln(geo.state);

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

void saveNewArgs(struct APIData_t data)
{
    FILE * arq = fopen("/media/dio/code/git repositories/OpenWeatherAPI-CLI/info.json", "r");
    if (arq == NULL) {
        fprintf(stderr, ".\n\n");
        return;
    }

    size_t buffer_size = file_size(arq);
    char * buffer = (char *)calloc(buffer_size + 1, sizeof(char));

    fread(buffer, buffer_size, 1, arq);

    cJSON * arrayJSON = cJSON_Parse(buffer);
    cJSON * infoJSON = cJSON_GetArrayItem(arrayJSON, 0);
    
    cJSON * geoJSON = cJSON_CreateObject();

    if (cJSON_HasObjectItem(infoJSON, "Geocoding")) 
    {
        cJSON_ReplaceItemInObjectCaseSensitive(infoJSON, "Geocoding", geoJSON);
    } 
    else
    {
        cJSON_AddItemToObject(infoJSON, "Geocoding", geoJSON);
    }

    cJSON_AddStringToObject(geoJSON, "name", data.geo.city);
    cJSON_AddStringToObject(geoJSON, "lat", data.geo.lat);
    cJSON_AddStringToObject(geoJSON, "lon", data.geo.lon);
    cJSON_AddStringToObject(geoJSON, "country", data.geo.state);
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

    fclose(arq);

    arq = fopen("/media/dio/code/git repositories/OpenWeatherAPI-CLI/info.json", "w");
    if (arq == NULL)
    {
        fprintf(stderr, "wasn't possible to open info.json in write mode.\n\n");
        return;
    }

    char * puts_buffer = cJSON_Print(infoJSON);
    fputs(puts_buffer, arq);


    free(buffer);
    free(puts_buffer);
    cJSON_Delete(infoJSON);
}

void checkTerminalArgs(struct APIData_t * data, int argc, char * argv[])
{
    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "--appid") == 0 || strcmp(argv[i], "-a") == 0)
        {
            data->appid = argv[i + 1];
            i++;
        }
        else if (strcmp(argv[i], "--city") == 0 || strcmp(argv[i], "-c") == 0)
        {
            data->geo.city = argv[i + 1];
            i++;
        }
        else if (strcmp(argv[i], "--country") == 0 || strcmp(argv[i], "-C") == 0)
        {
            data->geo.country = argv[i + 1];
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

        exit(-1);
    }

    if (data->appid == NULL)
    {
        data->appid = requestForJSON("appid");
        if (data->appid == NULL)
        {
            fprintf(stderr, "Appid was not provided and also not found in info.json.\n\n");
            exit(-1);
        }
    }
}

void printCurrentWeather(const struct APIData_t data, const struct currentWeather_t cW)
{
    char * ch;
    if (strcmp(data.units, "metric") == 0) {
        ch = "°C";
    } else {
        ch = "°F";
    }

    printf("%s | %s\n", cW.name, cW.country);
    printf("%s\n", cW.date);
    printf("%s: %s\n\n", cW.weather_main, cW.weather_description);

    printf("Temperature: %f %s| Feels like: %f %s\n", cW.temp, ch, cW.feels_like, ch);
    printf("Temperature min: %f %s | Temperature Max: %f %s\n", cW.temp_min, ch, cW.temp_max, ch);
    printf("Humidity: %f\n\n", cW.humidity);

    printf("Sunset: %ld | Sunrise: %ld\n", cW.sunset, cW.sunrise);
}

int main(int argc, char * argv[])
{   
    struct APIData_t data = {NULL};
    if (argc < 2) 
    {     
        checkCache(&data);
        struct string res = GETCurrentWeather(data);
        struct currentWeather_t cW = readCurrentWeather(res);
        printCurrentWeather(data, cW);

        deleteSTRING(&res);
        deleteAPIData_t(&data);
        deleteCurrentWeather_t(&cW);
    }
    else
    {   
        checkTerminalArgs(&data, argc, argv);
        struct string resGeocoding = GETGeocoding(data);

        data.geo = readGeocoding(resGeocoding);

        struct string resCurrentWeather = GETCurrentWeather(data);
        struct currentWeather_t cW = readCurrentWeather(resCurrentWeather);

        saveNewArgs(data);

        deleteSTRING(&resCurrentWeather);
        deleteSTRING(&resGeocoding);
        deleteAPIData_t(&data);
        deleteCurrentWeather_t(&cW);
    }

    return 0;
}