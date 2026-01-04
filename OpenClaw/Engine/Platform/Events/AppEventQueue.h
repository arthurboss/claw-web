#pragma once

#include "AppEvent.h"
#include <deque>
#include <mutex>

class AppEventQueue {
public:
  void Push(const AppEvent &e);
  bool Poll(AppEvent &out);
  void Clear();

private:
  std::mutex m_mutex;
  std::deque<AppEvent> m_queue;
};
