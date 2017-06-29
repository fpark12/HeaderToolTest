#ifndef QCOMMANDLINEPARSER_H
#define QCOMMANDLINEPARSER_H

//#include <QtCore/std::vector<std::string>.h>

//#include <QtCore/qcoreapplication.h>
#include <commandlineoption.h>

//QT_REQUIRE_CONFIG(commandlineparser);

#define Q_NORETURN

namespace header_tool
{

	class CommandLineParserPrivate;
	//class QCoreApplication;

	class Q_CORE_EXPORT CommandLineParser
	{
		//Q_DECLARE_TR_FUNCTIONS(QCommandLineParser)
	public:
		CommandLineParser();
		~CommandLineParser();

		enum SingleDashWordOptionMode
		{
			ParseAsCompactedShortOptions,
			ParseAsLongOptions
		};
		void setSingleDashWordOptionMode(SingleDashWordOptionMode parsingMode);

		enum OptionsAfterPositionalArgumentsMode
		{
			ParseAsOptions,
			ParseAsPositionalArguments
		};
		void setOptionsAfterPositionalArgumentsMode(OptionsAfterPositionalArgumentsMode mode);

		bool addOption(const CommandLineOption &commandLineOption);
		bool addOptions(const std::vector<CommandLineOption> &options);

		CommandLineOption addVersionOption();
		CommandLineOption addHelpOption();
		void setApplicationDescription(const std::string &description);
		std::string applicationDescription() const;
		void addPositionalArgument(const std::string &name, const std::string &description, const std::string &syntax = std::string());
		void clearPositionalArguments();

		void process(const std::vector<std::string> &arguments);
		//void process(const QCoreApplication &app);

		bool parse(const std::vector<std::string> &arguments);
		std::string errorText() const;

		bool isSet(const std::string &name) const;
		std::string value(const std::string &name) const;
		std::vector<std::string> values(const std::string &name) const;

		bool isSet(const CommandLineOption &option) const;
		std::string value(const CommandLineOption &option) const;
		std::vector<std::string> values(const CommandLineOption &option) const;

		std::vector<std::string> positionalArguments() const;
		std::vector<std::string> optionNames() const;
		std::vector<std::string> unknownOptionNames() const;

		Q_NORETURN void showVersion();
		Q_NORETURN void showHelp(int exitCode = 0);
		std::string helpText() const;

	private:
		//Q_DISABLE_COPY(QCommandLineParser)
		CommandLineParser(CommandLineParser&) = delete;

		CommandLineParserPrivate * const d;
	};

}

#endif // QCOMMANDLINEPARSER_H
