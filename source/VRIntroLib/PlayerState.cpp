#include "stdafx.h"
#include "PlayerState.h"

using json11::Json;

std::string PlayerState::ToJSON() const {
  EigenTypes::Vector3 _position = position.cast<double>();
  EigenTypes::Matrix3x3 _orientation = orientation.cast<double>();

  Json state = Json::array {
    Json::array { std::vector<double>(_position.data(), _position.data() + 3) },
    Json::array { std::vector<double>(_orientation.data(), _orientation.data() + 9) }
  };
  return state.dump();
}

void PlayerState::FromJSON(const std::string& message) {
  Json state(message);

  Json _position = state[0];
  position << static_cast<float>(_position[0].number_value()),
           static_cast<float>(_position[1].number_value()),
           static_cast<float>(_position[2].number_value());

  Json _orientation = state[0];
  orientation << static_cast<float>(_orientation[0].number_value()),
              static_cast<float>(_orientation[1].number_value()),
              static_cast<float>(_orientation[2].number_value()),
              static_cast<float>(_orientation[3].number_value()),
              static_cast<float>(_orientation[4].number_value()),
              static_cast<float>(_orientation[5].number_value()),
              static_cast<float>(_orientation[6].number_value()),
              static_cast<float>(_orientation[7].number_value()),
              static_cast<float>(_orientation[8].number_value());
}
