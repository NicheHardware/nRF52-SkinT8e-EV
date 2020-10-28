
#include "uevent.h"


__WEAK void user_event_dispatcher(uevt_t evt)
{
	LOG_RAW("[ERROR]event dispatcher NOT set!!!\r\n");
}

__WEAK void user_event_handler(uevt_t* evt, uint16_t _size_unused_)
{
	user_event_dispatcher(*evt);
}

void user_event_broadcast(uevt_t evt)
{
	platform_evt_put(&evt, sizeof(uevt_t), user_event_handler);
}
