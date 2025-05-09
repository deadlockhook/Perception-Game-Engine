#include "entity_system.h"
#include "../threading/thread_storage.h"

void component_t::destroy()
{
	auto ts = get_current_thread_storage();
	ts->current_layer = owner->owner_layer;
	ts->current_entity = owner;
	ts->current_component = this;

	__try {

		if (on_destroy)
			on_destroy(_class);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		on_fail("Component destructor failed! %s ", name.c_str()); //add more proper logging
	}
}