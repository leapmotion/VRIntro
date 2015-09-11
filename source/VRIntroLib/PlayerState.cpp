#include "stdafx.h"
#include "PlayerState.h"

#include <iostream>

float Cubic(float a, float b, float c, float d, float x) {
  float x2 = x*x;
  return 0.5f*(a*(x - 1)*(1 - x)*x +
               b*((3*x - 5)*x2 + 2) +
               c*((4 - 3*x)*x2 + x) +
               d*(x - 1)*x2);
}

Json PlayerState::ToJSON() const {
  EigenTypes::Vector3 _position = position.cast<double>();
  EigenTypes::Matrix3x3 _orientation = orientation.cast<double>();

  return Json::array {
    std::vector<double>(_position.data(), _position.data() + 3),
    std::vector<double>(_orientation.data(), _orientation.data() + 9)
  };
}

void PlayerState::FromJSON(const Json& message) {
  last_position = position;

  Json _position = message[0];
  position << static_cast<float>(_position[0].number_value()),
           static_cast<float>(_position[1].number_value()),
           static_cast<float>(_position[2].number_value());

  Json _orientation = message[1];
  orientation << static_cast<float>(_orientation[0].number_value()),
              static_cast<float>(_orientation[1].number_value()),
              static_cast<float>(_orientation[2].number_value()),
              static_cast<float>(_orientation[3].number_value()),
              static_cast<float>(_orientation[4].number_value()),
              static_cast<float>(_orientation[5].number_value()),
              static_cast<float>(_orientation[6].number_value()),
              static_cast<float>(_orientation[7].number_value()),
              static_cast<float>(_orientation[8].number_value());

  EigenTypes::Vector3f dx = position - last_position;

  //const EigenTypes::Vector3f side = (std::abs(dx.x()) > std::abs(dx.z()) ?
  //                                   EigenTypes::Vector3f(-dx.y(), dx.x(), 0.0) :
  //                                   EigenTypes::Vector3f(0.0, -dx.z(), dx.y())
  //                                  ).normalized() * 0.4f;
  const EigenTypes::Vector3f side = dx.cross(orientation.col(2)).normalized() * 0.4f;

  EigenTypes::Vector3f left_trail = position - side;
  EigenTypes::Vector3f right_trail = position + side;

  // render_offset = (renderoffset + NUM_TRAILS - 1) % NUM_TRAILS;
  // updatetrail with PREVIOUS
  // for loop, with some predefined number of interplated points.
  UpdateTrail(left_trail, right_trail);

  render_offset++;
  render_offset %= NUM_TRAILS;

  UpdateTrail(left_trail, right_trail);
}

void PlayerState::UpdateTrail(const EigenTypes::Vector3f& left, const EigenTypes::Vector3f& right) {
  // set
  trails[6*render_offset + 0] = left.x();
  trails[6*render_offset + 1] = left.y();
  trails[6*render_offset + 2] = left.z();
  trails[6*render_offset + 3] = right.x();
  trails[6*render_offset + 4] = right.y();
  trails[6*render_offset + 5] = right.z();

  // duplicate first and last set of points, to ensure continuity
  if (render_offset == 0) {
    trails[6*NUM_TRAILS + 0] = left.x();
    trails[6*NUM_TRAILS + 1] = left.y();
    trails[6*NUM_TRAILS + 2] = left.z();
    trails[6*NUM_TRAILS + 3] = right.x();
    trails[6*NUM_TRAILS + 4] = right.y();
    trails[6*NUM_TRAILS + 5] = right.z();
  }
}