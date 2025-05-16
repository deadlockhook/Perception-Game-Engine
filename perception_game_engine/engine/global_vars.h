#pragma once
#include "../platform/platform.h"
#include <cstdint>

constexpr uint32_t max_physics_ticks = 8;

enum thread_ids_t : uint16_t
{
    thread_id_frame=0,
    thread_id_physics,
    thread_id_count
};

struct frame_data_t
{
    double current_time = 0.0;
    double frame_time_ms = 0.0;
    uint64_t tick_count = 0;
    double time_accumulator = 0.0;
    double delta_time_ms = 0.0;
    double target_hz = 60.0;
    double current_hz = 0.0;
    double target_frame_time_ms = 1000.0 / 60.0;
	double tick_interval = 0.0; 

    __forceinline void update_refresh_rate(double new_target_hz) 
    {
        target_hz = new_target_hz;
        target_frame_time_ms = 1000.0 / new_target_hz;
		tick_interval = 1.0 / new_target_hz;
    }

    __forceinline void reset() 
    {
        frame_time_ms = 0.0;
        tick_count = 0;
        time_accumulator = 0.0;
        delta_time_ms = 0.0;
        current_hz = 0.0;
    }

    __forceinline double time_to_ticks(double time_100ns)  {  return (time_100ns / 10000000.0) / tick_interval;  }
    __forceinline double ticks_to_time(double ticks, double tick_interval_sec) { return (ticks * tick_interval) * 10000000.0;  }
};

struct global_vars_t
{
    atomic_uint64_t thread_update_end_tickcount[thread_id_count];

    __forceinline uint64_t get_update_end_tickcount(const thread_ids_t& id) { return p_get_atomic(thread_update_end_tickcount[id]); }
    __forceinline void set_update_end_tickcount(const thread_ids_t& id, uint64_t tickcount) { p_set_atomic(thread_update_end_tickcount[id], tickcount); }
  
    struct
    {
        struct {
            frame_data_t frame_data;
        } render_thread;

        struct {
            uint64_t physics_update_end_time[max_physics_ticks];
            frame_data_t frame_data;
        } physics_thread;

        struct {
            uint64_t last_physics_tick;
            frame_data_t frame_data;
        } frame_thread;

        struct
        {
            double delta_time_ms = 0.0;
            double total_time_ms = 0.0;
        } timing;

    } engine;

};

extern global_vars_t g_vars;