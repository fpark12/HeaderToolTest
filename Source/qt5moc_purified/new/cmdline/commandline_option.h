#pragma once

namespace header_tool
{
	class commandline_option
	{
	public:
		enum Flags
		{
			HiddenFromHelp = 0x1,
			ShortOptionStyle = 0x2
		};
		Flags flags;

		std::vector<std::string> keys;
		std::string valueName;
		std::string description;
		std::vector<std::string> defaultValues;
	};
}
