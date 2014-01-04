#pragma once

#include "pebble.h"

#define SIZE 5
#define HR_LEN 27
#define MIN_LEN 45

static const GPathInfo HOUR_HAND_POINTS = {
  4, (GPoint []){
    {-SIZE, 0},
    {0, SIZE},
    {SIZE, 0},
    {0, -HR_LEN},
  }
};

static const GPathInfo MINUTE_HAND_POINTS = {
  4, (GPoint []) {
    { -SIZE, 0 },
    { 0, SIZE },
    { SIZE, 0 },
    { 0, -MIN_LEN },
  }
};

static const GPathInfo DOW_POINTS = {
  3, (GPoint []){
    {0, 0},
    {10, 11},
    {19, 0},
  }
};
