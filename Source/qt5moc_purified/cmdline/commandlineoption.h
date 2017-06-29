#ifndef CommandLineOption_H
#define CommandLineOption_H

//#include <QtCore/std::vector<std::string>.h>
//#include <QtCore/qshareddata.h>
#include <memory>
#include <string>
#include <cstdio>
#include <sstream>

template<typename ... Args>
std::string string_format(const std::string& format, Args ... args)
{
	size_t size = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
	std::unique_ptr<char[]> buf(new char[size]);
	std::snprintf(buf.get(), size, format.c_str(), args ...);
	return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

/*! note: separator cannot contain null characters
*/
template<template<typename, typename> typename Container, typename Arg>
std::string join(const Container<std::string, Arg>& elements, const char* const separator)
{
	switch (elements.size())
	{
		case 0:
			return "";
		case 1:
			return elements.front();
		default:
			std::ostringstream os;
			std::copy(elements.begin(), elements.end(), std::ostream_iterator<std::string>(os, separator));
			os << *elements.rbegin();
			return os.str();
	}
}

template<template<typename, typename> typename Container, typename Arg>
void split(const char pChr, Container<std::string, Arg> &pRet, const std::string &pPath)
{
	std::string::const_iterator cur = pPath.begin();
	std::string::const_iterator beg = pPath.begin();
	bool added = false;
	while (cur < pPath.end())
	{
		if (*cur == pChr)
		{
			pRet.insert(pRet.end(), std::string(beg, cur));
			beg = ++cur;
			added = true;
		}
		else
		{
			cur++;
		}
	}

	pRet.insert(pRet.end(), std::string(beg, cur));
}

//QT_REQUIRE_CONFIG(commandlineparser);
#define Q_CORE_EXPORT
#define Q_DECL_NOTHROW

namespace header_tool
{

	class CommandLineOptionPrivate;

	class Q_CORE_EXPORT CommandLineOption
	{
	public:
		enum Flags
		{
			HiddenFromHelp = 0x1,
			ShortOptionStyle = 0x2
		};
		//Q_DECLARE_FLAGS(Flags, Flag)
		Flags flags;

		explicit CommandLineOption(const std::string &name);
		explicit CommandLineOption(const std::vector<std::string> &names);
		/*implicit*/ CommandLineOption(const std::string &name, const std::string &description,
			const std::string &valueName = std::string(),
			const std::string &defaultValue = std::string());
		/*implicit*/ CommandLineOption(const std::vector<std::string> &names, const std::string &description,
			const std::string &valueName = std::string(),
			const std::string &defaultValue = std::string());
		CommandLineOption(const CommandLineOption &other);

		~CommandLineOption();

		CommandLineOption &operator=(const CommandLineOption &other);
#ifdef Q_COMPILER_RVALUE_REFS
		CommandLineOption &operator=(CommandLineOption &&other) Q_DECL_NOTHROW
		{
			swap(other); return *this;
		}
#endif

		void swap(CommandLineOption &other) Q_DECL_NOTHROW
		{
			std::swap(d, other.d);
		}

		std::vector<std::string> names() const;

		void setValueName(const std::string &name);
		std::string valueName() const;

		void setDescription(const std::string &description);
		std::string description() const;

		void setDefaultValue(const std::string &defaultValue);
		void setDefaultValues(const std::vector<std::string> &defaultValues);
		std::vector<std::string> defaultValues() const;

		//Flags flags() const;
		void setFlags(Flags aflags);

		/*
		#if QT_DEPRECATED_SINCE(5, 8)
				QT_DEPRECATED_X("Use setFlags() with HiddenFromHelp")
					void setHidden(bool hidden);
				QT_DEPRECATED_X("Use flags() and HiddenFromHelp")
					bool isHidden() const;
		#endif
		*/

	private:
		std::shared_ptr<CommandLineOptionPrivate> d;
	};

	//Q_DECLARE_SHARED(CommandLineOption)
	//Q_DECLARE_OPERATORS_FOR_FLAGS(CommandLineOption::Flags)

}
#endif // CommandLineOption_H
