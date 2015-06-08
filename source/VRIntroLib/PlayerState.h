#pragma once

#include "contrib/json11.hpp"
#include "EigenTypes.h"
#include <vector>

class Rock {
  int id;
  int active_player;
  EigenTypes::Vector3f position;
  float mass;
};

struct PlayerState {
  std::string ToJSON() const;
  void FromJSON(const std::string& message);

  EigenTypes::Vector3f position;
  EigenTypes::Matrix3x3f orientation;

  Eigen::vector<EigenTypes::Vector3f> hand_positions;
  Eigen::vector<EigenTypes::Matrix3x3f> hand_orientations;

  float mass;

  std::vector<Rock> rocks_created;
  std::vector<int> rocks_activated;
  std::vector<int> rocks_deleted;
};
