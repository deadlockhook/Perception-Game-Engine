#include "setup.h"
#include <Windows.h>
#include "entity_system/entity_system.h"

void engine_execute()
{
    p_set_timer_resolution(1);
    g_entity_mgr.execute_start();
    system("pause");
    g_entity_mgr.execute_stop();
    p_reset_timer_resolution(1);
}