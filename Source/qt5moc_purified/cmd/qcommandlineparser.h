#ifndef QCOMMANDLINEPARSER_H
#define QCOMMANDLINEPARSER_H

//#include <QtCore/std::list<std::string>.h>

//#include <QtCore/qcoreapplication.h>
#include <qcommandlineoption.h>

//QT_REQUIRE_CONFIG(commandlineparser);

#define Q_NORETURN

namespace header_tool
{

	class QCommandLineParserPrivate;
	//class QCoreApplication;

	class Q_CORE_EXPORT QCommandLineParser
	{
		//Q_DECLARE_TR_FUNCTIONS(QCommandLineParser)
	public:
		QCommandLineParser();
		~QCommandLineParser();

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

		bool addOption(const QCommandLineOption &commandLineOption);
		bool addOptions(const std::list<QCommandLineOption> &options);

		QCommandLineOption addVersionOption();
		QCommandLineOption addHelpOption();
		void setApplicationDescription(const std::string &description);
		std::string applicationDescription() const;
		void addPositionalArgument(const std::string &name, const std::string &description, const std::string &syntax = std::string());
		void clearPositionalArguments();

		void process(const std::list<std::string> &arguments);
		//void process(const QCoreApplication &app);

		bool parse(const std::list<std::string> &arguments);
		std::string errorText() const;

		bool isSet(const std::string &name) const;
		std::string value(const std::string &name) const;
		std::list<std::string> values(const std::string &name) const;

		bool isSet(const QCommandLineOption &option) const;
		std::string value(const QCommandLineOption &option) const;
		std::list<std::string> values(const QCommandLineOption &option) const;

		std::list<std::string> positionalArguments() const;
		std::list<std::string> optionNames() const;
		std::list<std::string> unknownOptionNames() const;

		Q_NORETURN void showVersion();
		Q_NORETURN void showHelp(int exitCode = 0);
		std::string helpText() const;

	private:
		//Q_DISABLE_COPY(QCommandLineParser)
		QCommandLineParser(QCommandLineParser&) = delete;

		QCommandLineParserPrivate * const d;
	};

}

#endif // QCOMMANDLINEPARSER_H
