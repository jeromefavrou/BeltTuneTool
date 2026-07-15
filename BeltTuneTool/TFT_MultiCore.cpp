#include "headers/TFT_MultiCore.hpp"

TFT_MultiCore::TFT_MultiCore(TFT_eSPI *p)
{
	tft = p;
	tftMutex = xSemaphoreCreateMutex();
}

bool TFT_MultiCore::lock()
{
	if(xSemaphoreTake(tftMutex, portMAX_DELAY) == pdTRUE)
	{
		return true;
	}

	return false;
}

void TFT_MultiCore::unlock()
{
	xSemaphoreGive(tftMutex);
}
