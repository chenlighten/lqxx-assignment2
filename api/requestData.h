#pragma once

#include <string>

/*
 * @brief This is a structure that specifies which object is what we try to
 * access ie: the object parsed from API url For example: /users/1/tasks/2
 */

struct RequestData {
  /* data */
  /*
   * @brief User id
   */
  std::string user_key;
  /*
   * @brief Task list id
   */
  std::string tasklist_key;
  /*
   * @brief Task id
   */
  std::string task_key;

  /* methods */
  /*
   * @brief RequestData default constructor
   */
  RequestData() {}
  /*
   * @brief RequestData decomposed constructor
   */
  virtual ~RequestData() {}
  /*
   * @brief RequestData copy constructor
   */
  RequestData(const RequestData &data) {
    user_key = data.user_key;
    tasklist_key = data.tasklist_key;
    task_key = data.task_key;
  }
  /*
   * @brief operator == overload
   * @param data RequestData object to compare
   */
  bool operator==(const RequestData &other) const {
    return this->user_key == other.user_key &&
           this->tasklist_key == other.tasklist_key &&
           this->task_key == other.task_key;
  }
  /*
   * @brief Check if the User id is empty, const method
   * @return true if the User id is empty
   */
  bool RequestUserIsEmpty() const { return user_key == ""; }

  /*
   * @brief For TasklistWorker in request
   * @return true if the Task list id is empty
   */
  bool RequestTaskListIsEmpty() const {
    return user_key == "" || tasklist_key == "";
  }

  /*
   * @brief For TaskWorker in request
   * @return true if the Task id is empty
   */
  bool RequestIsEmpty() const {
    return user_key == "" || tasklist_key == "" || task_key == "";
  }
};