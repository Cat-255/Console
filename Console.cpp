#include "Console.hpp"

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#ifndef NOMINMAX
	#define NOMINMAX
#endif
	#include <windows.h>
	#ifdef byte
	#undef byte
#endif
#endif

#include <filesystem>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <ctime>

using namespace std;

Console::Console()
{
    cd_list["help"] = {&Console::cd_help, "For dummies", "No more info"};
    cd_list["echo"] = {&Console::cd_echo, "Makes an echo", "No more info"};
    cd_list["clear"] = {&Console::cd_clear, "Clears the console", "No more info"};
    cd_list["time"] = {&Console::cd_time, "Shows current time", "No more info"};
    cd_list["create"] = {&Console::cd_create, "Makes something", "Takes the arguments folder/file and the path where the given object will be created. Then creates it. The full command should look like: /create folder/file \"name\" \"C:/Users/...\""};
    cd_list["debugger"] = {&Console::cd_debugger, "Shows/Hides debugger", "The full command should look like: \"/debugger hide/show\""};
    cd_list["info"] = {&Console::cd_info, "Shows additional info for one exact command", "No more info"};
    cd_list["exit"] = {&Console::cd_exit, "Terminates the program", "You can enter one numeric argument: 0 - Forced system shutdown, 1 - Safe shutdown (standard)"};
}

void Console::print(string user_text)
{
    for (char& c : user_text)
        if (c == '\r') c = '\n';

    size_t start = 0;
    while (start <= user_text.size())
    {
        size_t end = user_text.find('\n', start);
        string paragraph = (end == string::npos)
            ? user_text.substr(start)
            : user_text.substr(start, end - start);

        auto lines = text_splitter(paragraph, 62);
        if (lines.empty())
            lines.push_back("");

        for (const auto& line : lines)
        {
            history.push_back(line);
            if (history.size() > 1000)
                history.erase(history.begin());
        }

        if (end == string::npos) break;
        start = end + 1;
    }
}

void Console::process_input(const string& input)
{
    if (input.empty() || input[0] != '/') return;

    string clean_input = input.substr(1);
    vector<string> full_args = parse_args(clean_input);

    if (full_args.empty()) return;

    string cd_name = full_args[0];
    vector<string> args(full_args.begin() + 1, full_args.end());

    string arg;
    if (full_args.size() > 1)
    {
        size_t pos = clean_input.find(cd_name);
        if (pos != string::npos)
        {
            arg = clean_input.substr(pos + cd_name.length());
            while (!arg.empty() && (arg[0] == ' ' || arg[0] == '\t'))
                arg.erase(0, 1);
        }
    }

    if (cd_list.count(cd_name))
    {
        auto method = cd_list[cd_name].command;
        (this->*method)(args, arg);
    }
    else
    {
        print("ERROR: Unknown command. Type \"/help\" for help");
    }
}

// ====================== Commands ======================

void Console::cd_echo(const vector<string>& args, const string& arg)
{
    print(arg);
}

void Console::cd_time(const vector<string>& args, const string& arg) 
{
    time_t now = time(0);
    tm ltm;

    #ifdef _WIN32
        localtime_s(&ltm, &now);
    #else
        localtime_r(&now, &ltm);
    #endif

    char buffer[32];
    strftime(buffer, sizeof(buffer), "%a %b %d %H:%M:%S %Y\n", &ltm);

    print("Current time:");
    print("> " + string(buffer));
}


void Console::cd_create(const vector<string>& args, const string& arg)
{
    try
    {
        if (args.size() < 3)
        {
            print("ERROR: Command is missing some arguments. Type \"/help\" for help");
            return;
        }

        string type = args[0];
        string object = args[1];
        string path = args[2];
        
        filesystem::path full_path = filesystem::path(path) / object;

        if (type == "folder")
        {
        	if (!filesystem::exists(full_path))
			{
				filesystem::create_directories(full_path);
				if (filesystem::exists(full_path))
                	print("The object \"" + object + "\" was successfully created at the path: " + path);
            	else
                	print("ERROR: Object \"" + object + "\" creation failed on path: " + path);
			}
			else
			{
				print("ERROR: This object already exists at the path: " + path);
                return;
			}
        }
        else if (type == "file")
        {
        	if (!filesystem::exists(full_path))
			{
				filesystem::path Path(full_path);
				if (!Path.parent_path().empty() && !filesystem::exists(Path.parent_path()))
	            {
	                filesystem::create_directories(Path.parent_path());
	            }
	            ofstream file(Path);
	            
	            if (file.is_open())
	            {
	                print("The object \"" + object + "\" was successfully created at the path: " + path);
	                file.close();
	            }
	            else
	            {
	                print("ERROR: Object \"" + object + "\" creation failed on path: " + path);
	            }
			}
			else
			{
				print("ERROR: This object already exists at the path: " + path);
                return;
			}            
        }
        else
        {
            print("ERROR: Unknown object type \"" + type + "\". Use /info create, this might help...");
        }
    }
    catch (const exception& error)
    {
        print("ERROR: " + string(error.what()) + " Type \"/help\" for help");
    }
}

void Console::cd_help(const vector<string>& args, const string& arg)
{
    vector<string> order =
    {
        "help",
		"echo",
		"clear",
		"time",
        "create",
		"debugger",
		"info",
		"exit"
    };

    print("Available commands list:");
    for (const string& name : order)
    {
        if (cd_list.count(name))
            print("> /" + name + " - " + cd_list[name].description);
    }
    print("");
    print("Use /info (command) for more info about one exact command");
}

void Console::cd_clear(const vector<string>& args, const string& arg)
{
    history.clear();
}

void Console::cd_debugger(const vector<string>& args, const string& arg)
{
#ifdef _WIN32
    HWND hwnd = GetConsoleWindow();
    if (arg == "show" || arg == "1")
    {
        if (hwnd)
		{
			ShowWindow(hwnd, SW_SHOW);
			print("debugger shown");
		} 
    }
    else if (arg == "hide" || arg == "0")
    {
        if (hwnd)
		{
			ShowWindow(hwnd, SW_HIDE);
			print("debugger is hidden");
		}
    }
    else if(arg == "")
    {
        print("ERROR: Empty argument");
    }
    else
    {
        print("ERROR: Incorrect argument \"" + arg + "\"");
    }
#else
    print("ERROR: This OS is not supported.");
#endif
}

void Console::cd_info(const vector<string>& args, const string& arg)
{
    if (cd_list.count(arg))
        print("> Additional info: " + cd_list[arg].add_desc);
    else
        print("ERROR: Command \"" + arg + "\" not found");
}

void Console::cd_exit(const vector<string>& args, const string& arg)
{
    print("System shutdown initiated...");
    if (arg == "1" || arg.empty())
    {
        loop = 0;
    }
    else if (arg == "0")
    {
        exit(0);
    }
    else
    {
        print("SHUTDOWN ERROR: Incorrect argument \"" + arg + "\"");
    }
}

// ====================== Additional ======================

vector<string> Console::parse_args(const string& input)
{
    vector<string> args;
    string current_arg;
    bool in_quotes = false;

    for (size_t i = 0; i < input.size(); ++i)
    {
        char c = input[i];

        if (c == '"')
        {
            in_quotes = !in_quotes;
        }
        else if (c == ' ' && !in_quotes)
        {
            if (!current_arg.empty())
            {
                args.push_back(current_arg);
                current_arg.clear();
            }
        }
        else
        {
            current_arg += c;
        }
    }

    if (!current_arg.empty())
        args.push_back(current_arg);

    return args;
}

vector<string> Console::text_splitter(const string& text, size_t max_chars)
{
    vector<string> result;
    if (text.empty())
    {
        result.push_back("");
        return result;
    }

    size_t start = 0;
    while (start < text.size())
    {
        size_t end = min(start + max_chars, text.size());

        if (end < text.size())
        {
            size_t space = text.rfind(' ', end);
            if (space != string::npos && space > start)
                end = space;
        }

        result.push_back(text.substr(start, end - start));
        start = end;

        if (start < text.size() && text[start] == ' ')
            ++start;
    }
    return result;
}
