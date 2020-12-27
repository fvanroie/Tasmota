#include "ArduinoLog.h"

#define HASP_NUM_PAGES 12

#define TAG_HASP 1
#define TAG_ATTR 2
#define TAG_MSGR 3

#define HASP_VERSION_MAJOR 0
#define HASP_VERSION_MINOR 0
#define HASP_VERSION_REVISION 1

#define HASP_USE_SPIFFS 0
#define HASP_USE_LITTLEFS 0

#define HASP_USE_APP 1
#define HASP_USE_DEBUG 0

#define halRestartMcu()
#define guiCalibrate()
#define guiSetDim(x)
#define guiGetDim() -1
#define guiSetBacklight(x)
#define guiGetBacklight() 1
#define guiStart()
#define guiStop()
#define gpio_set_group_state(x,y)