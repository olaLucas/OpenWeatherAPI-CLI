#if !defined(INCLUDES)
    #define INCLUDES
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <cjson/cJSON.h>

    #define STRING_SIZE 100
    #define BUFFER_SIZE 1024

#endif // INCLUDES

#include <time.h>
#include "../libs/request.h"

struct APIData_t
{
    char * lon;
    char * lat;
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

void printAPIData_t(struct APIData_t * gtD)
{
    printf("lon: %s\n", gtD->lon);
    printf("lat: %s\n", gtD->lat);
    printf("appid: %s\n", gtD->appid);
    printf("units: %s\n", gtD->units);
}

void printCurrentWeather_t(const struct currentWeather_t cW)
{
    printf("date: %s", cW.date);
    printf("name: %s\n", cW.name);
    printf("weather_main: %s\n", cW.weather_main);
    printf("weather_description: %s\n", cW.weather_description);
    printf("temp: %f\n", cW.temp);
    printf("feels_like: %f\n", cW.feels_like);
    printf("temp_min: %f\n", cW.temp_min);
    printf("temp_max: %f\n", cW.temp_max);
    printf("humidity: %f\n", cW.humidity);
    printf("sunset: %ld\n", cW.sunset);
    printf("sunrise: %ld\n", cW.sunrise);
    printf("dt: %ld\n", cW.dt);
}


void deleteAPIData_t(struct APIData_t * gtD)
{
    if (gtD->appid != NULL)
        free(gtD->appid);

    if (gtD->units != NULL)
        free(gtD->units);
    
    if (gtD->lat != NULL)
        free(gtD->lat);

    if (gtD->lon != NULL)
        free(gtD->lon);
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

void getCurrentWeather()
{
    cJSON * json;
    struct APIData_t gtD = {NULL};
    // struct getRequest gtR = initGetRequest();

    char * URL = (char *)calloc(BUFFER_SIZE + 1, sizeof(char));
    struct fileStruct fl = initFileStruct("../response.json", "w");

    FILE * arq = fopen("info.json", "r");
    if (arq == NULL)
    {
        fprintf(stderr, "Error while opening file.\n");
        // deleteGetRequest(&gtR);
        free(URL);
        deleteAPIData_t(&gtD);

        exit(-1);
    }

    size_t buffer_size = file_size(arq);
    char * buffer = (char *)calloc(buffer_size + 1, sizeof(char));

    fread(buffer, 1, buffer_size, arq);
    json = cJSON_Parse(buffer);

    gtD.appid = getDynamicJSONString("appid", buffer);
    gtD.units = getDynamicJSONString("units", buffer); 
    gtD.lat = getDynamicJSONString("lat", buffer);
    gtD.lon = getDynamicJSONString("lon", buffer);

    URL = makeURL(
        "https://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s&units=%s",
        gtD.lat, gtD.lon, gtD.appid, gtD.units
    );

    GETRequest(URL, (void *)&fl, JSON_FLAG);

    fclose(arq);
    free(buffer);
    free(URL);
    deleteFileStruct(&fl);
    // deleteGetRequest(&gtR);
    deleteAPIData_t(&gtD);
    cJSON_Delete(json);
}

struct currentWeather_t readCurrentWeather()
{
    cJSON * json;
    cJSON * main;
    cJSON * weather;
    cJSON * sys;

    struct currentWeather_t cW;
    
    char * buffer = NULL;
    size_t buffer_size = 0;
    
    FILE * arq = fopen("/media/dio/code/git repositories/OpenWeatherAPI-CLI/response.json", "r");
    if (arq == NULL) {
        fprintf(stderr, "readCurrentWeather(): arq == NULL\n\n");
        return cW;
    }

    buffer_size = file_size(arq);
    if (buffer_size <= 0) {
        fprintf(stderr, "readCurrentWeather(): buffer_size <= 0\n\n");
        exit(-1);
    }

    buffer = (char *)calloc(buffer_size + 1, sizeof(char));

    fread(buffer, 1, buffer_size, arq);
    json = cJSON_Parse(buffer);

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

    cW.temp = atof(cJSON_Print(cJSON_GetObjectItem(main, "temp")));
    cW.feels_like = atof(cJSON_Print(cJSON_GetObjectItem(main, "feels_like")));
    cW.temp_min = atof(cJSON_Print(cJSON_GetObjectItem(main, "temp_min")));
    cW.temp_max = atof(cJSON_Print(cJSON_GetObjectItem(main, "temp_max")));
    cW.dt = atoll(cJSON_Print(cJSON_GetObjectItem(json, "dt")));
    cW.sunset = atoll(cJSON_Print(cJSON_GetObjectItem(sys, "sunset")));
    cW.sunrise = atoll(cJSON_Print(cJSON_GetObjectItem(sys, "sunrise")));
    cW.date = ctime(&cW.dt);

    free(buffer);
    cJSON_Delete(json);
    fclose(arq);
    return cW;
}

int main(void)
{
    struct currentWeather_t cW;

    // getCurrentWeather();
    cW = readCurrentWeather();
    printCurrentWeather_t(cW);

    deleteCurrentWeather_t(&cW);
    return 0;
}