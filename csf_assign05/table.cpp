
#include "table.h"
#include "exceptions.h"
#include "guard.h"
#include <cassert>
#include <stdexcept>

Table::Table(const std::string &name) : m_name(name), is_locked(false) {
  if (pthread_mutex_init(&mutex, NULL) != 0) {
    throw std::runtime_error("Failed to initialize mutex");
  }
}

Table::~Table() { pthread_mutex_destroy(&mutex); }

std::string Table::get_name() const { return m_name; }

void Table::lock() {
  pthread_mutex_lock(&mutex);
  is_locked = true;
}

void Table::unlock() {
  is_locked = false;
  pthread_mutex_unlock(&mutex);
}

bool Table::trylock() {
  if (pthread_mutex_trylock(&mutex) == 0) {
    is_locked = true;
    return true;
  }
  return false;
}

void Table::set(const std::string &key, const std::string &value, bool stage) {
  if (!is_locked) {
    throw std::logic_error("Attempt to call set without lock being held");
  }
  if (stage) {
    staged_data[key] = value;
  } else {
    data[key] = value;
  }
}

std::string Table::get(const std::string &key, bool checkStaged) {
  if (!is_locked) {
    throw std::logic_error("Attempt to call get without lock being held");
  }
  if (checkStaged && staged_data.find(key) != staged_data.end()) {
    return staged_data[key];
  }
  auto it = data.find(key);
  if (it != data.end()) {
    return it->second;
  }
  throw std::out_of_range("Key not found: " + key);
}

bool Table::has_key(const std::string &key, bool checkStaged) {
  if (!is_locked) {
    throw std::logic_error("Attempt to call has_key without lock being held");
  }
  return (checkStaged && staged_data.find(key) != staged_data.end()) ||
         data.find(key) != data.end();
}

void Table::commit_changes() {
  if (!is_locked) {
    throw std::logic_error("Attempt to commit changes without lock being held");
  }
  for (const auto &kv : staged_data) {
    data[kv.first] = kv.second;
  }
  staged_data.clear();
}

void Table::rollback_changes() {
  if (!is_locked) {
    throw std::logic_error(
        "Attempt to rollback changes without lock being held");
  }
  staged_data.clear();
}