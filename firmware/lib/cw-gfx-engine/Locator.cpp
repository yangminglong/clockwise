#include "Locator.h"

VirtualMatrixPanel* Locator::_display;
EventBus* Locator::_eventBus;

void Locator::provide(VirtualMatrixPanel* display)
{
  _display = display;
}

void Locator::provide(EventBus* eventBus)
{
  _eventBus = eventBus;
}

VirtualMatrixPanel* Locator::getDisplay() 
{ 
  return _display; 
}

EventBus* Locator::getEventBus() 
{ 
  return _eventBus; 
}
