#include "stdafx.h"
#include "QuadsLayer.h"

#include "GLController.h"

QuadsLayer::QuadsLayer(const EigenTypes::Vector3f& initialEyePos) :
  InteractionLayer(initialEyePos) {
  // TODO: switch to non-default shader
}

void QuadsLayer::Update(TimeDelta real_time_delta) {

}

void QuadsLayer::Render(TimeDelta real_time_delta) const {
}

EventHandlerAction QuadsLayer::HandleKeyboardEvent(const SDL_KeyboardEvent &ev) {
  //switch (ev.keysym.sym) {
  //default:
  return EventHandlerAction::PASS_ON;
  //}
}
