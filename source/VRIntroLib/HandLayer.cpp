#include "stdafx.h"
#include "HandLayer.h"

#include "GLController.h"

HandLayer::HandLayer(const EigenTypes::Vector3f& initialEyePos) :
  InteractionLayer(initialEyePos) {
  // TODO: switch to non-default shader
}

void HandLayer::Update(TimeDelta real_time_delta) {

}

void HandLayer::Render(TimeDelta real_time_delta) const {
  DrawSkeletonHands();
}

EventHandlerAction HandLayer::HandleKeyboardEvent(const SDL_KeyboardEvent &ev) {
  //switch (ev.keysym.sym) {
  //default:
  return EventHandlerAction::PASS_ON;
  //}
}
