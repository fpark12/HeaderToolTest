#pragma once
#include "commandline_option.h"

namespace header_tool
{
	class commmandline_parser
	{
	private:
		std::string error_text;
		
		std::vector<commandline_option> options;
		std::unordered_map<std::string, commandline_option*> aliases;
		std::unordered_map<commandline_option*, std::vector<std::string>> values;

		//! Names of options found on the command line.
		std::vector<std::string> optionNames;

		//! Arguments which did not belong to any option.
		std::vector<std::string> positionalArgumentList;

		//! Names of options which were unknown.
		std::vector<std::string> unknownOptionNames;

		//! Application description
		std::string description;

		//! Documentation for positional arguments
		struct PositionalArgumentDefinition
		{
			std::string name;
			std::string description;
			std::string syntax;
		};
		std::vector<PositionalArgumentDefinition> positionalArgumentDefinitions;

		enum SingleDashWordOptionMode
		{
			ParseAsCompactedShortOptions,
			ParseAsLongOptions
		};
		SingleDashWordOptionMode singleDashWordOptionMode;

		enum OptionsAfterPositionalArgumentsMode
		{
			ParseAsOptions,
			ParseAsPositionalArguments
		};
		//! The parsing mode for "-abc"

		//! How to parse "arg -option"
		OptionsAfterPositionalArgumentsMode optionsAfterPositionalArgumentsMode;

		//! Whether addVersionOption was called
		bool builtinVersionOption;

		//! Whether addHelpOption was called
		bool builtinHelpOption;

		//! True if parse() needs to be called
		bool needsParsing;

	public:

		bool add_option(const commandline_option &option)
		{
			if (!option.keys.empty())
			{
				for (const std::string &name : optionNames)
				{
					if (aliases.find(name) != aliases.end())
					{
						//todo
						assert(false);
						return false;
					}
				}

				options.push_back(option);

				for (const std::string &alias : option.keys)
				{
					aliases[alias] = &options.back();
				}

				return true;
			}
			return false;
		}
	};
}