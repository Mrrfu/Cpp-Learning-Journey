//
// Created by a2057 on 25-7-27.
//

#include "TaskManager.h"
#include "Logger.h"
#include <iostream>


TaskManager::TaskManager(): nextId(1) {
    loadTasksFromFile();
}

TaskManager::~TaskManager() {
    saveTasksToFile();
}


void TaskManager::loadTasksFromFile() {
    std::ifstream infile;
    infile.open("tasks.txt");
    if (!infile.is_open()) {
        Logger::getInstance().log("Failed to open tasks file");
        return;
    }
    std::string line;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        Task task;
        char delimiter;
        iss >> task.id >> delimiter;
        std::getline(iss, task.description, ',');
        iss >> task.priority >> delimiter;
        iss >> task.dueDate;
        tasks.emplace_back(task);
        if (task.id >= nextId) {
            nextId = task.id + 1;
        }
    }
    infile.close();
    Logger::getInstance().log("Loaded tasks from file");
}

void TaskManager::addTask(const std::string &description, int priority, const std::string &date) {
    Task task;
    task.id = nextId++;
    task.description = description;
    task.priority = priority;
    task.dueDate = date;
    tasks.emplace_back(task);
    Logger::getInstance().log("Task added: " + task.toString());
    saveTasksToFile();
}

void TaskManager::removeTask(int id) {
    auto it = find_if(tasks.begin(), tasks.end(), [id](const Task &task) {
        return task.id == id;
    });
    if (it != tasks.end()) {
        tasks.erase(it);
        Logger::getInstance().log("Task removed: " + it->toString());
        saveTasksToFile();
    } else {
        std::cout << "Task not found " << std::endl;
    }
}

void TaskManager::updateTask(int id, const std::string &description, int priority, const std::string &date) {
    auto it = find_if(tasks.begin(), tasks.end(), [id](const Task &task) {
        return task.id == id;
    });
    if (it != tasks.end()) {
        it->priority = priority;
        it->description = description;
        it->dueDate = date;
    } else {
        std::cout << "Task not found " << std::endl;
    }
}

void TaskManager::saveTasksToFile() const {
    std::ofstream outfile;
    outfile.open("tasks.txt");
    if (!outfile.is_open()) {
        Logger::getInstance().log("Failed open tasks file for writing");
        return;
    }
    for (const auto &task: tasks) {
        outfile << task.id << "," << task.description << "," << task.priority << "," << task.dueDate << std::endl;
    }

    outfile.close();
    Logger::getInstance().log("Saved tasks to file");
}

void TaskManager::listTasks(int sortOption) const {
    std::vector<Task> sortedTasks = tasks;
    switch (sortOption) {
        case 1:
            std::sort(sortedTasks.begin(), sortedTasks.end(), compareByPriority);
            break;
        case 2:
            std::sort(sortedTasks.begin(), sortedTasks.end(), compareByDate);
            break;
        default:
            break;
    }
    for (const auto &task: sortedTasks) {
        std::cout << task.toString() << std::endl;
    }
}


bool TaskManager::compareByPriority(const Task &task1, const Task &task2) {
    return task1.priority < task2.priority;
}

bool TaskManager::compareByDate(const Task &task1, const Task &task2) {
    return task1.dueDate < task2.dueDate;
}
