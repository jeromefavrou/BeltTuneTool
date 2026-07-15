#pragma once

#include <TFT_eSPI.h>
#include <Arduino.h>
#include <memory>

class TFT_MultiCore
{
public:
    TFT_eSPI *tft;

    SemaphoreHandle_t tftMutex;

    TFT_MultiCore(TFT_eSPI *p);

    bool lock();

    void unlock();
};


