//
// Created by a2057 on 25-7-27.
//

#ifndef COMMAND_H
#define COMMAND_H
#include <string>
#include <iostream>
#include <memory>


class CommandBase {
public:
    virtual ~CommandBase() = default;

    void virtual execute(const std::string &args) =0;
};


template<typename Derived>
class Command : public CommandBase {
public:
    void execute(const std::string &args) override {
        (static_cast<Derived *>(this))->executeImpl(args);
    }
};

#include "TaskManager.h"
#include "Logger.h"

class AddCommand : public Command<AddCommand> {
public:
    AddCommand(TaskManager &taskManager) : taskManager(taskManager) {
    }

    void executeImpl(const std::string &args) {
        //解析参数
        size_t pos1 = args.find(',');
        size_t pos2 = args.find(',', pos1 + 1);
        if (pos1 == std::string::npos || pos2 == std::string::npos) {
            std::cout << "参数格式错误，请使用：add <描述>,<优先级>,<截止日期>" << std::endl;
            return;
        }
        std::string description = args.substr(0, pos1);
        int priority = std::stoi(args.substr(pos1 + 1, pos2 - pos1 - 1));
        std::string dueDate = args.substr(pos2 + 1);
        taskManager.addTask(description, priority, dueDate);
        std::cout << "任务添加成功." << std::endl;
    }

private:
    TaskManager &taskManager;
};

class RemoveCommand : public Command<RemoveCommand> {
public:
    RemoveCommand(TaskManager &taskManager): taskManager(taskManager) {
    }

    void executeImpl(const std::string &args) {
        try {
            size_t pos;
            int id = std::stoi(args, &pos);

            if (pos != args.length()) {
                //如果字符串不能完全转换为一个数字，就是错误，例如11s
                std::cout << "参数格式错误，请使用delete <id>" << std::endl;
                return;
            }

            taskManager.removeTask(id);

            std::cout << "任务删除成功." << std::endl;
        } catch (const std::invalid_argument &e) {
            Logger::getInstance().log("参数格式错误");
        }catch (const std::out_of_range &e) {
            Logger::getInstance().log("参数格式错误");
        }
    }

private:
    TaskManager &taskManager;
};

class ListCommand : public Command<ListCommand> {
public:
    ListCommand(TaskManager &taskManager) : taskManager(taskManager) {
    }

    void executeImpl(const std::string &args) {
        //将参数转为对应的数字
        int sortOption = 0;
        try {
            if (!args.empty()) {
                size_t pos;
                sortOption = std::stoi(args, &pos);
                if (pos != args.length()) {
                    //如果字符串不能完全转换为一个数字，就是错误，例如11s
                    std::cout << "参数格式错误，使用list <排序选项> --0：默认排序, --1：按优先级，--2：按日期" << std::endl;
                    return;
                }
            }

            taskManager.listTasks(sortOption);
        } catch (const std::invalid_argument &e) {
            Logger::getInstance().log("参数格式错误");
        }catch (const std::out_of_range &e) {
            Logger::getInstance().log("参数格式错误");
        }
    }

private:
    TaskManager &taskManager;
};

class UpdateCommand : public Command<UpdateCommand> {
public:
    UpdateCommand(TaskManager &taskManager): taskManager(taskManager) {
    }

    void executeImpl(const std::string &args) {
        try {
            //参数格式  1,明早九点起床,1,2025-07-28
            size_t pos1 = args.find(',');
            size_t pos2 = args.find(',', pos1 + 1);
            size_t pos3 = args.find(',', pos2 + 1);

            if (pos1 == std::string::npos || pos2 == std::string::npos || pos3 == std::string::npos) {
                std::cout << "参数格式错误，请使用：update  <id>,<描述>,<优先级>,<截止日期>" << std::endl;
                return;
            }

            int id = std::stoi(args.substr(0, pos1));
            std::string description = args.substr(pos1 + 1, pos2 - pos1 - 1);
            int priority = std::stoi(args.substr(pos2 + 1, pos3 - pos2 - 1));
            std::string dueDate = args.substr(pos3 + 1);
            taskManager.updateTask(id, description, priority, dueDate);
            std::cout << "任务更新成功！" << std::endl;
        } catch (const std::invalid_argument &e) {
            Logger::getInstance().log("参数格式错误");
        }catch (const std::out_of_range &e) {
            Logger::getInstance().log("参数格式错误");
        }
    }

private:
    TaskManager &taskManager;
};

#endif //COMMAND_H
