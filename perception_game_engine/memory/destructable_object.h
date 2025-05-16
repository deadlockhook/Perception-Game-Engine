#pragma once
#include "../platform/atomic_datatype.h"
#include "../engine/global_vars.h"

struct destructable_object_t
{
	destructable_object_t()
	{
		p_set_atomic(destroyed, false);
		p_set_atomic(destroy_pending, false);
	}

	atomic_bool_t destroyed;
	atomic_bool_t destroy_pending;

	__forceinline bool is_destroyed() { return p_get_atomic(destroyed); }
	__forceinline bool is_destroy_pending() { return p_get_atomic(destroy_pending); }
	__forceinline bool is_destroyed_or_pending() { return is_destroyed() || is_destroy_pending(); }
};

struct destruction_queue_t : public destructable_object_t
{
	destruction_queue_t() = default;

	uint64_t request_tick[thread_ids_t::thread_id_count];

	void mark_for_destruction()
	{
		if (is_destroyed_or_pending())
			return;

		for (uint16_t i = 0; i < thread_ids_t::thread_id_count; ++i)
			request_tick[i] = g_vars.get_update_end_tickcount((thread_ids_t)i);

		p_set_atomic(destroy_pending, true);
	}

	bool can_destroy()
	{
		for (uint16_t i = 0; i < thread_ids_t::thread_id_count; ++i)
		{
			if (g_vars.get_update_end_tickcount((thread_ids_t)i) < request_tick[i] + 2)
				return false;
		}

		return true;
	}

	void post_destruction()
	{
		p_set_atomic(destroyed, true);
		p_set_atomic(destroy_pending, false);
	}
};