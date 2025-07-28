//
// Created by a2057 on 25-7-27.
//

#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "Task.h"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>

class TaskManager {
public:
    TaskManager();

    ~TaskManager();

    void addTask(const std::string &description, int priority, const std::string &date);

    void removeTask(int id);

    void updateTask(int id, const std::string &description, int priority, const std::string &date);

    void listTasks(int sortOption) const; //0: 按ID，1：按优先级，2：按日期
    void saveTasksToFile() const;

    void loadTasksFromFile();

private:
    std::vector<Task> tasks;
    int nextId;

    static bool compareByDate(const Task &first, const Task &second);

    static bool compareByPriority(const Task &first, const Task &second);
};


#endif //TASKMANAGER_H
