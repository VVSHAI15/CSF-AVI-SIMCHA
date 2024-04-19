#include <cassert>
#include "table.h"
#include "exceptions.h"
#include "guard.h"
#include <stdexcept>

Table::Table(const std::string& name) : m_name(name) {
  // Initialize the pthread mutex
  if (pthread_mutex_init(&mutex, NULL) != 0) {
    throw std::runtime_error("Failed to initialize mutex");
  }
}

Table::~Table()
{
  pthread_mutex_destroy(&mutex);
}


void Table::lock()
{
  pthread_mutex_lock(&mutex);
}

void Table::unlock()
{
  pthread_mutex_unlock(&mutex);
}

bool Table::trylock()
{
  return pthread_mutex_trylock(&mutex) == 0;   // Returns 0 on success
}

void Table::set(const std::string& key, const std::string& value, bool stage) {
  // Locking is assumed to be managed by the caller 
  if (stage) {
    staged_data[key] = value;  // Place changes in the staging area.
  } else {
    data[key] = value;  // Directly modify the main data map.
  }
}


std::string Table::get(const std::string& key, bool checkStaged) {
  if (checkStaged && staged_data.find(key) != staged_data.end()) {
    return staged_data[key];  // Return staged value if it exists.
  }
  if (data.find(key) != data.end()) {
    return data[key];  // Return the committed value.
  }
  throw std::out_of_range("Key not found: " + key);
}

bool Table::has_key(const std::string& key, bool checkStaged) {
  return (checkStaged && staged_data.find(key) != staged_data.end()) ||
  data.find(key) != data.end();
}

void Table::commit_changes() {
  for (const auto& kv : staged_data) {
    data[kv.first] = kv.second;  // Commit each staged change to the main data.
  }
  staged_data.clear();  // Clear the staging area after committing.
}

void Table::rollback_changes() {
  staged_data.clear();  // Discard all staged changes.
}


