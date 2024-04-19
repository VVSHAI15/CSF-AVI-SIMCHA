#ifndef TABLE_H
#define TABLE_H

#include <map>
#include <string>
#include <pthread.h>

class Table {
private:
  std::string m_name;
  std::map<std::string, std::string> data;
  pthread_mutex_t mutex;
  std::map<std::string, std::string> staged_data;  // Temporary storage for proposed changes.

  // copy constructor and assignment operator are prohibited
  Table( const Table & );
  Table &operator=( const Table & );

public:
  Table( const std::string &name );
  ~Table();

  std::string get_name() const { return m_name; }

  void lock();
  void unlock();
  bool trylock();

  // Note: these functions should only be called while the
  // table's lock is held!
  void set(const std::string& key, const std::string& value, bool stage = true);
  std::string get(const std::string& key, bool checkStaged = true);
  bool has_key(const std::string& key, bool checkStaged = true);
  void commit_changes();
  void rollback_changes();
};

#endif // TABLE_H



