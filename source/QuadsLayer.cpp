#include "QuadsLayer.h"


QuadsLayer::QuadsLayer() {
  // TODO: switch to non-default shader
}

void QuadsLayer::Update(TimeDelta real_time_delta) {

}

void QuadsLayer::Render(TimeDelta real_time_delta) const {
  DrawSkeletonHands();
}

EventHandlerAction QuadsLayer::HandleKeyboardEvent(const SDL_KeyboardEvent &ev) {
  switch (ev.keysym.sym) {
  default:
    return EventHandlerAction::PASS_ON;
  }
}
