#include "entity_system.h"
#include "../global_vars.h"

DWORD WINAPI execute_on_physics_update(LPVOID param)
{
    p_set_thread_priority_time_critical();

	auto& frame_data = g_vars.engine.physics_thread.frame_data;

    frame_data.update_refresh_rate(60.0);

    uint64_t frequency = p_query_frequency();
    uint64_t start_time = p_query_counter();
    uint64_t current_time = 0;

    while (true)
    {
        frame_data.current_time = p_query_counter();
       // g_entity_mgr.on_physics_start();
        g_entity_mgr.on_physics_update();
      //  g_entity_mgr.on_physics_end();

        g_vars.engine.physics_thread.physics_update_end_time[g_vars.engine.physics_thread.frame_data.tick_count % max_physics_ticks] = p_query_counter();
        g_vars.set_update_end_tickcount(thread_ids_t::thread_id_physics,frame_data.tick_count);

        while (true)
        {
            current_time = p_query_counter();
            double elapsed_ms = (current_time - start_time) * 1000.0 / frequency;
            if (elapsed_ms >=frame_data.target_frame_time_ms)
                break;

            double sleep_ms =frame_data.target_frame_time_ms - elapsed_ms;
            if (sleep_ms > 2.0)
                Sleep(1); 
        }
        current_time = p_query_counter();
        double actual_frame_time_ms = (current_time - start_time) * 1000.0 / frequency;

       frame_data.frame_time_ms = actual_frame_time_ms;

        double capped_dt = std::min(actual_frame_time_ms, frame_data.target_frame_time_ms * 2.0);
       frame_data.delta_time_ms = capped_dt;

       frame_data.time_accumulator += actual_frame_time_ms;
       frame_data.tick_count++;

       frame_data.current_hz = 1000.0 / actual_frame_time_ms;

        start_time = p_query_counter();
    }

    return 0;
}

DWORD WINAPI execute_on_frame(LPVOID param)
{
    p_set_thread_priority_high();
    auto& frame_data = g_vars.engine.frame_thread.frame_data;

    uint64_t frequency = p_query_frequency();
    uint64_t start_time = p_query_counter();
    uint64_t current_time = 0;

    while (true)
    {
        frame_data.current_time = p_query_counter();
		g_vars.engine.frame_thread.last_physics_tick = g_vars.get_update_end_tickcount(thread_ids_t::thread_id_physics);

       // g_entity_mgr.on_frame_start();
        g_entity_mgr.on_frame_update();
       // g_entity_mgr.on_frame_end();

        g_vars.set_update_end_tickcount(thread_ids_t::thread_id_frame, frame_data.tick_count);

        current_time = p_query_counter();
        double elapsed_ms = (current_time - start_time) * 1000.0 / static_cast<double>(frequency);

        frame_data.frame_time_ms = elapsed_ms;
        frame_data.delta_time_ms = elapsed_ms;
        frame_data.time_accumulator += elapsed_ms;
        frame_data.tick_count++;

        frame_data.current_hz = 1000.0 / elapsed_ms;
     
		std::cout << "Frame Time: " << frame_data.current_hz << " ms" << std::endl;

        start_time = p_query_counter();
    }

    return 0;
}

DWORD WINAPI execute_gc(LPVOID param)
{
    while (true)
    {
        g_entity_mgr.on_gc();
        p_sleep(200);
    }
}
