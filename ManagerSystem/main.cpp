#include <iostream>

#include "Logger.h"
#include "TaskManager.h"
#include "Command.h"
#include <unordered_map>
#include <memory>
#include <functional>
#include <variant>

int main() {
    system("chcp 65001 > null");
    TaskManager taskManager;
    //方式一：虚基类，使用了虚函数，破环了CRTP的结构
    // std::unordered_map<std::string,std::unique_ptr<CommandBase>> commands;
    // commands["add"] = std::make_unique<AddCommand>(taskManager);
    // commands["delete"] = std::make_unique<RemoveCommand>(taskManager);
    // commands["list"] = std::make_unique<ListCommand>(taskManager);
    // commands["update"] = std::make_unique<UpdateCommand>(taskManager);
    //方式二 类型擦除
    // std::unordered_map<std::string, std::function<void(const std::string &args)> > commands;
    // //使用共享智能指针延长声明周期
    // auto add_command = std::make_shared<AddCommand>(taskManager);
    // auto list_command = std::make_shared<ListCommand>(taskManager);
    // auto update_command = std::make_shared<UpdateCommand>(taskManager);
    // auto remove_command = std::make_shared<RemoveCommand>(taskManager);
    // commands["add"] = [add_command](const std::string &args) {
    //     add_command->execute(args);
    // };
    // commands["list"] = [list_command](const std::string &args) {
    //     list_command->execute(args);
    // };
    // commands["update"] = [update_command](const std::string &args) {
    //     update_command->execute(args);
    // };
    // commands["delete"] = [remove_command](const std::string &args) {
    //     remove_command->execute(args);
    // };

    //方式三 std::variant
    using CommandVariant = std::variant<
        std::unique_ptr<AddCommand>,
        std::unique_ptr<RemoveCommand>,
        std::unique_ptr<ListCommand>,
        std::unique_ptr<UpdateCommand>
    >;
    std::unordered_map<std::string, CommandVariant> commands;
    commands["add"] = std::make_unique<AddCommand>(taskManager);
    commands["delete"] = std::make_unique<RemoveCommand>(taskManager);
    commands["list"] = std::make_unique<ListCommand>(taskManager);
    commands["update"] = std::make_unique<UpdateCommand>(taskManager);

    std::cout << "欢迎使用任务管理系统！" << std::endl;
    std::cout << "可用命令：add, delete, list, update" << std::endl;
    std::string cmd;
    while (true) {
        std::cout << "\n";
        std::getline(std::cin, cmd);
        if (cmd.empty()) {
            continue;
        }
        size_t pos = cmd.find(' ');
        std::string command = cmd.substr(0, pos);
        std::string args;
        if (pos != std::string::npos) {
            args = cmd.substr(pos + 1);
        }
        if (command == "exit") {
            std::cout << "退出程序" << std::endl;
            break;
        }
        auto it = commands.find(command);
        if (it != commands.end()) {
            // it->second(args);
            std::visit([&args](auto&& cmdPtr) {
                cmdPtr->execute(args);
            },it->second);
        } else {
            std::cout << "未知命令" << std::endl;
        }
    }
    return 0;
}
