#include <iostream>
#include <string>

#include <deque>
#include <memory>
#include <utility>
#include <tuple>

class TestLib
{
public:

    void exec(long num)
    {
        std::cout << "long value '" << num << "'\n";
    }

    void exec(std::string str)
    {
        std::cout << "std::string value  '" << str << "'\n";
    }

    void exec(const char* str)
    {
        std::cout << "string literal '" << str << "'\n";
    }

    void exec(long num, std::string str, std::unique_ptr<std::string> pStr, const char* strL)
    {
        std::cout << "Command accepting multiple arguments:\n";
        std::cout << "\tlong value '" << num << "'\n";
        std::cout << "\tstd::string value  '" << str << "'\n";
        std::cout << "\tstd::unique_ptr<std::string> value  '" << (*pStr) << "'\n";
        std::cout << "\tstring literal '" << strL << "'\n";
    }

};

template <class L>
class CommandQueue
{
    class AbstractCommand
    {
    public:
        virtual ~AbstractCommand() { };

        virtual void execute(L*) = 0;
    };

    template <class ...Args>
    class Command : public AbstractCommand
    {
    public:
        Command(void(L::*meth)(Args...), Args... args) : method(meth), arguments(std::forward<Args>(args)...) { }

        virtual ~Command() { }

        void execute(L* obj) override { (obj->*method)(std::forward<Args>(std::get<Args>(arguments))...); }

    private:
        Command() { }

        void(L::*method)(Args...);
        std::tuple<Args...> arguments;
    };

    class NullCommand : public AbstractCommand
    {
    public:
        virtual ~NullCommand() { }

        void execute(L* obj) override { }
    };

    typedef std::unique_ptr<AbstractCommand> ContainerEntryType;
    typedef std::deque<ContainerEntryType> ContainerType;

public:

    class CommandHandle
    {
    public:
        CommandHandle() : command(new NullCommand()) { }
        CommandHandle(ContainerEntryType&& cmd) : command(std::forward<ContainerEntryType>(cmd)) { }
        
        void execute(L* obj) { command->execute(obj); }

        operator bool() { return !dynamic_cast<NullCommand*>(command.get()); }

    private:
        ContainerEntryType command;
    };

    template <class ...Args>
    void pushCommand(void(L::*meth)(Args...), Args... args)
    {
        queue.push_back(ContainerEntryType(new Command<Args...>(meth, std::forward<Args>(args)...)));
    }

    CommandHandle popCommand()
    {
        if (!queue.empty())
        {
            CommandHandle cmdhdl(std::move(queue.front()));
            queue.pop_front();
            return cmdhdl;
        }
        return CommandHandle();
    }

private:

    ContainerType queue;

};

int main(int argc, char** argv)
{
    TestLib lib;
    CommandQueue<TestLib> queue;

    queue.pushCommand(&TestLib::exec, std::string("main_function_pointers"));
    queue.pushCommand(&TestLib::exec, "main_function_pointers");
    queue.pushCommand(&TestLib::exec, long(39));
    queue.pushCommand(&TestLib::exec, long(39)
                                    , std::string("main_function_pointers_string")
                                    , std::unique_ptr<std::string>(new std::string("main_function_pointers_uptr"))
                                    , "main_function_pointers_literal");

    while (auto cmdhdl = queue.popCommand())
    {
        cmdhdl.execute(&lib);
    }

    return 0;
}
