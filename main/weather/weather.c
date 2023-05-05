#include "weather.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include <time.h>


static weather_data_t weather_data = {.code = 38, .location="xian", .text="hot", .temp = 25};

weather_data_t *weather_data_upate()
{
    weather_data.code = rand() % 40;
    weather_data.temp = rand() % 40;
	return &weather_data;
}
