#ifndef __TEST_UI_H__
#define __TEST_UI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "WouoUI.h"
void TestUI_Init(void);
void TestUI_Proc(void);
extern WavePage wave_page;
extern TitlePage main_page;
extern TitlePage pid_page;
extern ListPage setting_page;
#ifdef __cplusplus
}
#endif

#endif
