#pragma once

#include "contrib/json11.hpp"
#include "EigenTypes.h"
#include <vector>

using json11::Json;

class Rock {
  int id;
  int active_player;
  EigenTypes::Vector3f position;
  float mass;
};

struct PlayerState {
  static const int NUM_TRAILS = 1000;

  PlayerState() : render_offset(0), trails((NUM_TRAILS + 1)*2*3) {}

  Json ToJSON() const;
  void FromJSON(const Json& message);
  void UpdateTrail(const EigenTypes::Vector3f& left, const EigenTypes::Vector3f& right);

  EigenTypes::Vector3f position;
  EigenTypes::Vector3f last_position;
  EigenTypes::Matrix3x3f orientation;

  Eigen::vector<EigenTypes::Vector3f> hand_positions;
  Eigen::vector<EigenTypes::Matrix3x3f> hand_orientations;

  float mass;

  std::vector<Rock> rocks_created;
  std::vector<int> rocks_activated;
  std::vector<int> rocks_deleted;

  std::vector<float> trails;
  int render_offset;
};
