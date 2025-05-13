#include "entity_system.h"
#include "../global_vars.h"

void handle_physics_update() {  g_entity_mgr.on_physics_update(); }

DWORD WINAPI execute_on_physics_update(LPVOID param)
{
    p_set_thread_priority_time_critical();

    constexpr double target_hz = 60.0;
    constexpr double target_frame_time_ms = 1000.0 / target_hz;

    uint64_t frequency = p_query_frequency();
    uint64_t start_time = p_query_counter();
    uint64_t current_time = 0;

    while (true)
    {

        g_entity_mgr.on_physics_update();

        while (true)
        {
            current_time = p_query_counter();
            double elapsed_ms = (current_time - start_time) * 1000.0 / frequency;
            if (elapsed_ms >= target_frame_time_ms)
                break;

            double sleep_ms = target_frame_time_ms - elapsed_ms;
            if (sleep_ms > 2.0)
                Sleep(1); 
        }
        current_time = p_query_counter();
        double actual_frame_time_ms = (current_time - start_time) * 1000.0 / frequency;

        g_vars.engine.physics_update.frame_time_ms = actual_frame_time_ms;
        g_vars.engine.physics_update.delta_time_ms = actual_frame_time_ms;
        g_vars.engine.physics_update.time_accumulator += actual_frame_time_ms;
        g_vars.engine.physics_update.frame_count++;

        g_vars.engine.physics_update.current_hz = 1000.0 / actual_frame_time_ms;

        start_time = p_query_counter();
    }

    return 0;
}

DWORD WINAPI execute_on_frame(LPVOID param)
{
    p_set_thread_priority_high();

    uint64_t frequency = p_query_frequency();
    uint64_t start_time = p_query_counter();
    uint64_t current_time = 0;

    while (true)
    {
        g_entity_mgr.on_frame();

        current_time = p_query_counter();
        double elapsed_ms = (current_time - start_time) * 1000.0 / static_cast<double>(frequency);

        g_vars.engine.frame_update.frame_time_ms = elapsed_ms;
        g_vars.engine.frame_update.delta_time_ms = elapsed_ms;
        g_vars.engine.frame_update.time_accumulator += elapsed_ms;
        g_vars.engine.frame_update.frame_count++;

        g_vars.engine.frame_update.current_hz = 1000.0 / elapsed_ms;

        std::cout << "fps " << g_vars.engine.frame_update.current_hz << std::endl;

        start_time = p_query_counter();
    }

    return 0;
}
