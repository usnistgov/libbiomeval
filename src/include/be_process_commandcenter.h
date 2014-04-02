/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to Title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef __BE_PROCESS_COMMANDCENTER_H__
#define __BE_PROCESS_COMMANDCENTER_H__

#include <cstdio>
#include <string>
#include <vector>

#include <be_framework_enumeration.h>
#include <be_memory_autoarray.h>
#include <be_memory_autoarrayutility.h>
#include <be_process_messagecenter.h>
#include <be_text.h>

namespace BiometricEvaluation
{
	namespace Process
	{
		/** Receive enumerations as commands over the network. */
		template <typename T,
		    typename = typename std::enable_if<std::is_enum<T>::value>>
		class CommandCenter
		{
		public:
			/** Parsed command received from the network. */
			class Command
			{
			public:
				/** ID of the sender. */
				uint32_t clientID;
				/** Enumeration value of the command. */
				T command;
				/** Arguments passed to command (optional). */
				std::vector<std::string> arguments;
			};

			static_assert(std::is_enum<T>::value,
			    "Invalid templatization of CommandCenter.");

			/**
			 * @brief
			 * Constructor.
			 *
			 * @param port
			 * Port to listen on for commands.
			 */
			 CommandCenter(
			    uint16_t port = MessageCenter::DEFAULT_PORT) :
			    _messageCenter(port)
			{

			}

			/** Destructor (default). */
			~CommandCenter() = default;

			/**
			 * @brief
			 * Determine if there are commands waiting.
			 *
			 * @return
			 * true if there are commands waiting, false otherwise.
			 *
			 * @note
			 * Returns immediately.
			 *
			 * @see BiometricEvaluation::Process::CommandCenter::
			 * getNextCommand()
			 */
			inline bool
			hasPendingCommands()
			{
				return (this->_messageCenter.
				    hasUnseenMessages());
			}

			/**
			 * @brief
			 * Get the next command.
			 *
			 * @param command
			 * Reference to a Command that will be populated when
			 * this method returns true.
			 * @param numSeconds
			 * Number of seconds to wait for a command, or -1 to
			 * block indefinitely.
			 * @param invalidCommandResponse
			 * Optional string to send, such as usage, that will
			 * be sent when an unrecognized command is received.
			 *
			 * @return
			 * true if command has been populated, false otherwise.
			 */
			inline bool
			getNextCommand(
			    Command &command,
			    int numSeconds = -1,
			    std::string invalidCommandResponse = "")
			{
				Memory::uint8Array buffer;
				if (!this->_messageCenter.getNextMessage(
				    command.clientID, buffer, numSeconds))
					return (false);

				/* Arguments are space separated */
				command.arguments = Text::split(
				    to_string(buffer), ' ');
				if (command.arguments.size() == 0)
					return (false);

				/* Remove newline from last argument */
				std::for_each(command.arguments.begin(),
				    command.arguments.end(),
				    [](std::string &i) {
					Text::
					removeLeadingTrailingWhitespace(i);
				});

				/* Split actual command off of arguments */
				try {
					command.command = to_enum<T>(
					    command.arguments[0]);
				} catch (Error::ObjectDoesNotExist) {
					/* 
					 * Send implementation specific usage
					 * if set.
					 */
					if (invalidCommandResponse != "")
						this->sendResponse(
						    command.clientID,
						    invalidCommandResponse);
					else {
						static const std::string
						    INVALID = ": "
						    "command not recognized";
						this->sendResponse(
						    command.clientID,
						    command.arguments[0] +
						    INVALID);
					}
					return (false);
				}
				command.arguments.erase(
				    command.arguments.begin());

				return (true);
			}

			/**
			 * @brief
			 * Send a string response to a client.
			 *
			 * @param clientID
			 * ID of client to communicate with.
			 * @param response
			 * Printable string to send to client.
			 * @param prefix
			 * String to prefix to responses.
			 * @param suffix
			 * String to append to responses.
			 */
			inline void
			sendResponse(
			    uint32_t clientID,
			    const std::string &response,
			    const std::string prefix = ">> ",
			    const std::string suffix = "\n")
			{
				Memory::uint8Array message;
				Memory::AutoArrayUtility::setString(message,
				    prefix + response + suffix);
				this->_messageCenter.sendResponse(clientID,
				    message);
			}

			/**
			 * @brief
			 * Break the connection with a client.
			 *
			 * @param clientID
			 * ID of the client to disconect.
			 */
			inline void
			disconnectClient(
			    uint32_t clientID)
			{
				this->sendResponse(clientID, "Goodbye");
				this->_messageCenter.disconnectClient(clientID);
			}

		private:
			/** Send and receive commands over the network. */
			MessageCenter _messageCenter;
		};

		/** Abstraction to parse messages received via CommandCenter. */
		template <typename T>
		class CommandParser : public CommandCenter<T>
		{
		public:
			/**
			 * @brief
			 * Parse command.
			 * @details
			 * Implement this method as a switch statement of your
			 * command enumeration.
			 */
			virtual void
			parse(
			    const typename
			    CommandCenter<T>::Command &command) = 0;

			/**
			 * @brief
			 * Get the next command.
			 *
			 * @param command
			 * Reference to a Command that will be populated when
			 * this method returns true.
			 * @param numSeconds
			 * Number of seconds to wait for a command, or -1 to
			 * block indefinitely.
			 *
			 * @return
			 * true if command has been populated, false otherwise.
			 */
			inline bool
			getNextCommand(
			    typename CommandCenter<T>::Command &command,
			    int numSeconds = -1)
			{
				return (CommandCenter<T>::getNextCommand(
				    command, numSeconds, this->getUsage()));
			}

			/**
			 * @brief
			 * String sent when an invalid command is received.
			 *
			 * @param usage
			 * String to send when an invalid command is received.
			 *
			 * @note
			 * If not set, no additional usage is sent.
			 */
			inline void
			setUsage(
			    const std::string &usage)
			{
				this->_usage = usage;
			}

			/** @return Usage string. */
			inline std::string
			getUsage()
			    const
			{
				return (this->_usage);
			}

			/**
			 * @brief
			 * Constructor.
			 *
			 * @param port
			 * Port to listen on for commands.
			 */
			CommandParser(
			    uint16_t port = MessageCenter::DEFAULT_PORT) :
			    CommandCenter<T>(port),
			    _usage("")
			{

			}
			
			/** Virtual destructor (default). */
			virtual ~CommandParser() = default;

		private:
			/** Usage string. */
			std::string _usage;
		};
	}
}

#endif /* __BE_PROCESS_COMMANDCENTER_H__ */
