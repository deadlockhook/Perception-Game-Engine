#include "setup.h"
#include <Windows.h>
#include "entity_system/entity_system.h"
#include "hwnd/window_manager.h"

void engine_execute()
{
    v_heap_mem::init();

 
#ifdef EDITOR_MODE
    auto window_instance = g_window_manager.create_window("PerceptionEditor", 0, 0, window_flag_centered | window_flag_resizable);
#else
    auto window_instance = g_window_manager.create_window("PerceptionWindow", 0, 0, window_flag_centered | window_flag_resizable);
#endif

    if (!window_instance) {
        MessageBoxA(nullptr, "Failed to create window", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    window_instance->set_resolution(500, 500);
  
    p_set_thread_priority_high();

    p_set_timer_resolution(1);
    g_entity_mgr.execute_start();

    uint64_t frequency = p_query_frequency();
    uint64_t start_time = p_query_counter();
    uint64_t current_time = 0;

    while (window_instance->process_messages())
    {
        g_vars.engine.render_thread.frame_data.current_time = p_query_counter();


        //exec here


        current_time = p_query_counter();
        double elapsed_ms = (current_time - start_time) * 1000.0 / static_cast<double>(frequency);

        g_vars.engine.render_thread.frame_data.frame_time_ms = elapsed_ms;
        g_vars.engine.render_thread.frame_data.delta_time_ms = elapsed_ms;
        g_vars.engine.render_thread.frame_data.time_accumulator += elapsed_ms;
        g_vars.engine.render_thread.frame_data.tick_count++;

        g_vars.engine.render_thread.frame_data.current_hz = 1000.0 / elapsed_ms;

        g_vars.set_update_end_tickcount(thread_ids_t::thread_id_frame, g_vars.engine.render_thread.frame_data.tick_count);
 
        start_time = p_query_counter();
    }

    g_entity_mgr.execute_stop();
    p_reset_timer_resolution(1); 

    return;
}