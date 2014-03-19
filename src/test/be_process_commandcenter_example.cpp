/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to Title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <iostream>
#include <map>
#include <string>

#include <be_framework_enumeration.h>
#include <be_process_commandcenter.h>

namespace BE = BiometricEvaluation;

/** Supported commands. */
enum class EvalCommand
{
	Stop,
	Status,
	Disconnect,

	Help
};

template<>
const std::map<EvalCommand, std::string>
BE::Framework::EnumerationFunctions<EvalCommand>::enumToStringMap {
    {EvalCommand::Stop, "stop"},
    {EvalCommand::Status, "status"},
    {EvalCommand::Disconnect, "disconnect"},

    {EvalCommand::Help, "help"}
};

/**
 * @brief
 * Parser for supported commands.
 * @details
 * Publically inherit a specialization of BE::Process::CommandParser, then
 * override parse() with your enumeration.
 *
 * @note
 * A CommandParser subclass is not strictly required, just a convenience.
 */
class EvalCommandParser : public BE::Process::CommandParser<EvalCommand>
{
public:
	/** Print the command and its arguments, for demonstration. */
	void
	print(
	    const BE::Process::CommandParser<EvalCommand>::Command &command)
	    const
	{
		std::cout << "Received Command:" << std::endl;
		std::cout << "\t" << to_string(command.command) << std::endl;
		if (command.arguments.size() > 0) {
			uint16_t counter = 0;
			for (auto &i : command.arguments) {
				std::cout << "\targ[" << counter++ << "] = " <<
				    i << std::endl;
			}
		} else
			std::cout << "\tNo arguments" << std::endl;
	}

	/** Override parse() by convention to parse commands. */
	void
	parse(
	    const BE::Process::CommandParser<EvalCommand>::Command &command)
	{
		/* Switch on the command. */
		switch (command.command) {
		case EvalCommand::Disconnect:
			this->disconnect(command);
			break;
		case EvalCommand::Stop:
			/* FALLTHROUGH */
		case EvalCommand::Status:
			this->notImplemented(command);
			break;
		case EvalCommand::Help:
			this->sendResponse(command.clientID, usage());
		}
	}

	/** Convenience to enumerate enumerations for usage string. */
	std::string
	usage()
	    const
	{
		auto lastCommand = EvalCommand::Help;
		std::string usage = "Allowed commands: ";

		for (auto i = static_cast<
		    std::underlying_type<EvalCommand>::type>(EvalCommand::Stop);
		    i <= static_cast<std::underlying_type<EvalCommand>::type>(
		    lastCommand); i++) {
			EvalCommand enumVal = to_enum<EvalCommand>(i);
			usage += to_string(enumVal);
			if (enumVal != lastCommand)
				usage += ", ";
		}

		return (usage);
	}

private:
	/*
	 * Implement command handlers as private methods.
	 */

	void
	disconnect(
	    const BE::Process::CommandCenter<EvalCommand>::Command &command)
	{
		this->disconnectClient(command.clientID);
	}

	void
	notImplemented(
	    const BE::Process::CommandCenter<EvalCommand>::Command &command)
	{
		/* Use sendResponse() to complain about syntax, etc. */
		if (command.arguments.size() != 1) {
			this->sendResponse(command.clientID, "Usage: " +
			    std::string(to_string(command.command)) +
			    " <process>");
			return;
		}

		/* 
		 * You'd parse arguments here and do work, sending back the
		 * appropriate response.
		 */

		this->sendResponse(command.clientID, "Not implemented");
	}
};

int
main()
{
	EvalCommandParser commandCenter;
	commandCenter.setUsage("Invalid command. Send \"help\" for allowed "
	    "commands.");
	EvalCommandParser::Command command;

	for (;;) {
		if (commandCenter.getNextCommand(command, 1)) {
			commandCenter.print(command);
			commandCenter.parse(command);
		}
	}

	return (0);
}
