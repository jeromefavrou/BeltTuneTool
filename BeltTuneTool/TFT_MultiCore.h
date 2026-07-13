
#ifndef tftMultiCore_h
#define tftMultiCore_h

#include <TFT_eSPI.h>
#include <Arduino.h>
#include <memory>

class TFT_MultiCore
{
public:
    TFT_eSPI *tft;

    SemaphoreHandle_t tftMutex;

    TFT_MultiCore(TFT_eSPI *p)
    {
        tft = p;
        tftMutex = xSemaphoreCreateMutex();
    }

    bool lock()
  {

  
      if (xSemaphoreTake(tftMutex, portMAX_DELAY) == pdTRUE)
      {

          return true;
      }
  
      return false;
  }

    void unlock()
  {

      xSemaphoreGive(tftMutex);
  }
};


#endif
