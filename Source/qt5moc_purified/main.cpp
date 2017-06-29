#include "preprocessor.h"
#include "moc.h"
#include "outputrevision.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <qcommandlineoption.h>
#include <qcommandlineparser.h>
#if 0

#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>

//#include <qcoreapplication.h>
#endif
namespace header_tool
{

#if 0

	/*
		This function looks at two file names and returns the name of the
		infile with a path relative to outfile.

		Examples:

			/tmp/abc, /tmp/bcd -> abc
			xyz/a/bc, xyz/b/ac -> ../a/bc
			/tmp/abc, xyz/klm -> /tmp/abc
	 */

	static std::vector<uint8> combinePath(const std::string &infile, const std::string &outfile)
	{
		QFileInfo inFileInfo(QDir::current(), infile);
		QFileInfo outFileInfo(QDir::current(), outfile);
		const std::vector<uint8> relativePath = outFileInfo.dir().relativeFilePath(inFileInfo.filePath()));
#ifdef Q_OS_WIN
		// It's a system limitation.
		// It depends on the Win API function which is used by the program to open files.
		// cl apparently uses the functions that have the MAX_PATH limitation.
		if (outFileInfo.dir().absolutePath().length() + relativePath.length() + 1 >= 260)
			return inFileInfo.absoluteFilePath());
#endif
		return relativePath;
	}


	void error(const char *msg = "Invalid argument")
	{
		if (msg)
			fprintf(stderr, "moc: %s\n", msg);
	}


	static inline bool hasNext(const Symbols &symbols, int i)
	{
		return (i < symbols.size());
	}

	static inline const Symbol &next(const Symbols &symbols, int &i)
	{
		return symbols.at(i++);
	}


	std::vector<uint8> composePreprocessorOutput(const Symbols &symbols)
	{
		std::vector<uint8> output;
		int lineNum = 1;
		Token last = PP_NOTOKEN;
		Token secondlast = last;
		int i = 0;
		while (hasNext(symbols, i))
		{
			Symbol sym = next(symbols, i);
			switch (sym.token)
			{
				case PP_NEWLINE:
				case PP_WHITESPACE:
					if (last != PP_WHITESPACE)
					{
						secondlast = last;
						last = PP_WHITESPACE;
						output += ' ';
					}
					continue;
				case PP_STRING_LITERAL:
					if (last == PP_STRING_LITERAL)
						output.chop(1);
					else if (secondlast == PP_STRING_LITERAL && last == PP_WHITESPACE)
						output.chop(2);
					else
						break;
					output += sym.lexem().mid(1);
					secondlast = last;
					last = PP_STRING_LITERAL;
					continue;
				case MOC_INCLUDE_BEGIN:
					lineNum = 0;
					continue;
				case MOC_INCLUDE_END:
					lineNum = sym.lineNum;
					continue;
				default:
					break;
			}
			secondlast = last;
			last = sym.token;

			const int padding = sym.lineNum - lineNum;
			if (padding > 0)
			{
				output.resize(output.size() + padding);
				memset(output.data() + output.size() - padding, '\n', padding);
				lineNum = sym.lineNum;
			}

			output += sym.lexem();
		}

		return output;
	}

	static std::list<std::string> argumentsFromCommandLineAndFile(const std::list<std::string> &arguments)
	{
		std::list<std::string> allArguments;
		allArguments.reserve(arguments.size());
		for (const std::string &argument : arguments)
		{
			// "@file" doesn't start with a '-' so we can't use QCommandLineParser for it
			if (argument.startsWith(QLatin1Char('@')))
			{
				std::string optionsFile = argument;
				optionsFile.remove(0, 1);
				if (optionsFile.empty())
				{
					error("The @ option requires an input file");
					return std::list<std::string>();
				}
				QFile f(optionsFile);
				if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
				{
					error("Cannot open options file specified with @");
					return std::list<std::string>();
				}
				while (!f.atEnd())
				{
					std::string line = std::string::fromLocal8Bit(f.readLine().trimmed());
					if (!line.empty())
						allArguments << line;
				}
			}
			else
			{
				allArguments << argument;
			}
		}
		return allArguments;
	}

#endif

	int runMoc(int argc, char **argv)
	{
		// QCoreApplication app(argc, argv);
		// QCoreApplication::setApplicationVersion(std::string::fromLatin1(QT_VERSION_STR));

		bool autoInclude = true;
		bool defaultInclude = true;
		Preprocessor pp;
		Moc moc;
		pp.macros["Q_MOC_RUN"];
		pp.macros["__cplusplus"];

		// Don't stumble over GCC extensions
		Macro dummyVariadicFunctionMacro;
		dummyVariadicFunctionMacro.isFunction = true;
		dummyVariadicFunctionMacro.isVariadic = true;
		dummyVariadicFunctionMacro.arguments.push_back(Symbol(0, PP_IDENTIFIER, "__VA_ARGS__"));
		pp.macros["__attribute__"] = dummyVariadicFunctionMacro;
		pp.macros["__declspec"] = dummyVariadicFunctionMacro;

		std::string filename;
		std::string output;
		//QFile in;
		FILE *out = 0;

#pragma region cmdline
		// Note that moc isn't translated.
		// If you use this code as an example for a translated app, make sure to translate the strings.
		QCommandLineParser parser;
		parser.setApplicationDescription(string_format("Qt Meta Object Compiler version %1 (Qt %2)",
			mocOutputRevision, "4.8.7"));
		parser.addHelpOption();
		parser.addVersionOption();
		parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);

		QCommandLineOption outputOption("o");
		outputOption.setDescription("Write output to file rather than stdout.");
		outputOption.setValueName("file");
		outputOption.setFlags(QCommandLineOption::ShortOptionStyle);
		parser.addOption(outputOption);

		QCommandLineOption includePathOption("I");
		includePathOption.setDescription("Add dir to the include path for header files.");
		includePathOption.setValueName("dir");
		includePathOption.setFlags(QCommandLineOption::ShortOptionStyle);
		parser.addOption(includePathOption);

		QCommandLineOption macFrameworkOption("F");
		macFrameworkOption.setDescription("Add Mac framework to the include path for header files.");
		macFrameworkOption.setValueName("framework");
		macFrameworkOption.setFlags(QCommandLineOption::ShortOptionStyle);
		parser.addOption(macFrameworkOption);

		QCommandLineOption preprocessOption("E");
		preprocessOption.setDescription("Preprocess only; do not generate meta object code.");
		parser.addOption(preprocessOption);

		QCommandLineOption defineOption("D");
		defineOption.setDescription("Define macro, with optional definition.");
		defineOption.setValueName("macro[=def]");
		defineOption.setFlags(QCommandLineOption::ShortOptionStyle);
		parser.addOption(defineOption);

		QCommandLineOption undefineOption("U");
		undefineOption.setDescription("Undefine macro.");
		undefineOption.setValueName("macro");
		undefineOption.setFlags(QCommandLineOption::ShortOptionStyle);
		parser.addOption(undefineOption);

		QCommandLineOption metadataOption("M");
		metadataOption.setDescription("Add key/value pair to plugin meta data");
		metadataOption.setValueName("key=value");
		metadataOption.setFlags(QCommandLineOption::ShortOptionStyle);
		parser.addOption(metadataOption);

		QCommandLineOption compilerFlavorOption("compiler-flavor");
		compilerFlavorOption.setDescription("Set the compiler flavor: either \"msvc\" or \"unix\".");
		compilerFlavorOption.setValueName("flavor");
		parser.addOption(compilerFlavorOption);

		QCommandLineOption noIncludeOption("i");
		noIncludeOption.setDescription("Do not generate an #include statement.");
		parser.addOption(noIncludeOption);

		QCommandLineOption pathPrefixOption("p");
		pathPrefixOption.setDescription("Path prefix for included file.");
		pathPrefixOption.setValueName("path");
		pathPrefixOption.setFlags(QCommandLineOption::ShortOptionStyle);
		parser.addOption(pathPrefixOption);

		QCommandLineOption forceIncludeOption("f");
		forceIncludeOption.setDescription("Force #include <file> (overwrite default).");
		forceIncludeOption.setValueName("file");
		forceIncludeOption.setFlags(QCommandLineOption::ShortOptionStyle);
		parser.addOption(forceIncludeOption);

		QCommandLineOption prependIncludeOption("b");
		prependIncludeOption.setDescription("Prepend #include <file> (preserve default include).");
		prependIncludeOption.setValueName("file");
		parser.addOption(prependIncludeOption);

		QCommandLineOption includeOption("include");
		includeOption.setDescription("Parse <file> as an #include before the main source(s).");
		includeOption.setValueName("file");
		parser.addOption(includeOption);

		QCommandLineOption noNotesWarningsCompatOption("n");
		noNotesWarningsCompatOption.setDescription("Do not display notes (-nn) or warnings (-nw). Compatibility option.");
		noNotesWarningsCompatOption.setValueName("which");
		noNotesWarningsCompatOption.setFlags(QCommandLineOption::ShortOptionStyle);
		parser.addOption(noNotesWarningsCompatOption);

		QCommandLineOption noNotesOption("no-notes");
		noNotesOption.setDescription("Do not display notes.");
		parser.addOption(noNotesOption);

		QCommandLineOption noWarningsOption("no-warnings");
		noWarningsOption.setDescription("Do not display warnings (implies --no-notes).");
		parser.addOption(noWarningsOption);

		QCommandLineOption ignoreConflictsOption("ignore-option-clashes");
		ignoreConflictsOption.setDescription("Ignore all options that conflict with compilers, like -pthread conflicting with moc's -p option.");
		parser.addOption(ignoreConflictsOption);

		parser.addPositionalArgument("[header-file]", "Header file to read from, otherwise stdin.");
		parser.addPositionalArgument("[@option-file]", "Read additional options from option-file.");
#pragma endregion cmdline

		std::list<std::string> arguments;
		for (int i = 0; i < argc; i++)
		{
			arguments.push_back(argv[i]);
		}
		if (arguments.empty())
			return 1;

		parser.process(arguments);
		const std::list<std::string> files = parser.positionalArguments();
		if (files.size() > 1)
		{
			std::cout << (std::string("Too many input files specified: '") + join(files, "' '") + '\'');
			parser.showHelp(1);
		}
		else if (!files.empty())
		{
			filename = files.front();
		}
		const bool ignoreConflictingOptions = parser.isSet(ignoreConflictsOption);
		output = parser.value(outputOption);
		pp.preprocessOnly = parser.isSet(preprocessOption);
		if (parser.isSet(noIncludeOption))
		{
			moc.noInclude = true;
			autoInclude = false;
		}

		if (!ignoreConflictingOptions)
		{
			if (parser.isSet(forceIncludeOption))
			{
				moc.noInclude = false;
				autoInclude = false;
				const auto forceIncludes = parser.values(forceIncludeOption);
				for (const std::string &include : forceIncludes)
				{
					moc.includeFiles.push_back(include);
					defaultInclude = false;
				}
			}
			const auto prependIncludes = parser.values(prependIncludeOption);
			for (const std::string &include : prependIncludes)
				moc.includeFiles.push_front(include);
			if (parser.isSet(pathPrefixOption))
				moc.includePath = parser.value(pathPrefixOption);
		}

		const auto includePaths = parser.values(includePathOption);
		for (const std::string &path : includePaths)
			pp.includes.emplace_back(path);
		std::string compilerFlavor = parser.value(compilerFlavorOption);
		if (compilerFlavor.empty() || compilerFlavor == "unix")
		{
			/*
			// traditional Unix compilers use both CPATH and CPLUS_INCLUDE_PATH
			// $CPATH feeds to #include <...> and #include "...", whereas
			// CPLUS_INCLUDE_PATH is equivalent to GCC's -isystem, so we parse later
			const auto cpath = qgetenv("CPATH").split(QDir::listSeparator().toLatin1());
			for (const std::vector<uint8> &p : cpath)
				pp.includes.emplace_back(p);
			const auto cplus_include_path = qgetenv("CPLUS_INCLUDE_PATH").split(QDir::listSeparator().toLatin1());
			for (const std::vector<uint8> &p : cplus_include_path)
				pp.includes.emplace_back(p);
				*/
		}
		else if (compilerFlavor == "msvc")
		{
			/*
			// MSVC uses one environment variable: INCLUDE
			const auto include = qgetenv("INCLUDE").split(QDir::listSeparator().toLatin1());
			for (const std::vector<uint8> &p : include)
				pp.includes.emplace_back(p);
				*/
		}
		else
		{
			printf((std::string("Unknown compiler flavor '") + compilerFlavor + "'; valid values are: msvc, unix.").c_str());
			parser.showHelp(1);
		}

		const auto macFrameworks = parser.values(macFrameworkOption);
		for (const std::string &path : macFrameworks)
		{
			// minimalistic framework support for the mac
			Preprocessor::IncludePath p(path);
			p.isFrameworkPath = true;
			pp.includes.emplace_back(p);
		}
		const auto defines = parser.values(defineOption);
		for (const std::string &arg : defines)
		{
			std::string name = arg;// .toLocal8Bit();
			std::string value("1");
			int eq = name.find('=');
			if (eq >= 0)
			{
				value = sub(name, eq + 1);
				name = sub(name,0, eq);
			}
			if (name.empty())
			{
				printf("Missing macro name");
				parser.showHelp(1);
			}
			Macro macro;
			macro.symbols = Preprocessor::tokenize(value, 1, Preprocessor::TokenizeDefine);
			macro.symbols.pop_back(); // remove the EOF symbol
			pp.macros[name] = macro;
		}
		const auto undefines = parser.values(undefineOption);
		for (const std::string &arg : undefines)
		{
			std::string macro = arg;// .toLocal8Bit();
			if (macro.empty())
			{
				printf("Missing macro name");
				parser.showHelp(1);
			}
			pp.macros.erase(macro);
		}

		const std::list<std::string> noNotesCompatValues = parser.values(noNotesWarningsCompatOption);
		if (parser.isSet(noNotesOption) || std::find(noNotesCompatValues.begin(), noNotesCompatValues.end(),"n") != noNotesCompatValues.end())
		moc.displayNotes = false;
		if (parser.isSet(noWarningsOption) || std::find(noNotesCompatValues.begin(), noNotesCompatValues.end(),"w") != noNotesCompatValues.end())
		moc.displayWarnings = moc.displayNotes = false;


#if 0
		if (autoInclude)
		{
			int spos = std::distance(filename.begin(), std::find(filename.end(), filename.begin(), '/'));
			int ppos = std::distance(filename.begin(), std::find(filename.end(), filename.begin(), '.'));
			// spos >= -1 && ppos > spos => ppos >= 0
			moc.noInclude = (ppos > spos && std::tolower(filename[ppos + 1]) != 'h');
		}
		if (defaultInclude)
		{
			if (moc.includePath.empty())
			{
				if (filename.size())
				{
					if (output.size())
					{
						moc.includeFiles.push_back(combinePath(filename, output));
					}
					else
					{
						moc.includeFiles.push_back(filename);
					}
				}
			}
			else
			{
				moc.includeFiles.append(combinePath(filename, filename));
			}
		}

		if (filename.empty())
		{
			filename = "standard input");
			in.open(stdin, QIODevice::ReadOnly);
		}
		else
		{
			in.setFileName(filename);
			if (!in.open(QIODevice::ReadOnly))
			{
				fprintf(stderr, "moc: %s: No such file\n", filename.c_str());
				return 1;
			}
			moc.filename = filename;
		}










	

		const auto metadata = parser.values(metadataOption);
		for (const std::string &md : metadata)
		{
			int split = md.indexOf(QLatin1Char('='));
			std::string key = md.left(split);
			std::string value = md.mid(split + 1);

			if (split == -1 || key.empty() || value.empty())
			{
				error("missing key or value for option '-M'");
			}
			else if (key.indexOf(QLatin1Char('.')) != -1)
			{
				// Don't allow keys with '.' for now, since we might need this
				// format later for more advanced meta data API
				error("A key cannot contain the letter '.' for option '-M'");
			}
			else
			{
				QJsonArray array = moc.metaArgs.value(key);
				array.append(value);
				moc.metaArgs.insert(key, array);
			}
		}

		moc.currentFilenames.push(filename.toLocal8Bit());
		moc.includes = pp.includes;

		// 1. preprocess
		const auto includeFiles = parser.values(includeOption);
		for (const std::string &includeName : includeFiles)
		{
			std::vector<uint8> rawName = pp.resolveInclude(includeName), moc.filename);
			if (rawName.empty())
			{
				fprintf(stderr, "Warning: Failed to resolve include \"%s\" for moc file %s\n",
					includeName.toLocal8Bit().constData(),
					moc.filename.empty() ? "<standard input>" : moc.filename.constData());
			}
			else
			{
				QFile f(QFile::decodeName(rawName));
				if (f.open(QIODevice::ReadOnly))
				{
					moc.symbols += Symbol(0, MOC_INCLUDE_BEGIN, rawName);
					moc.symbols += pp.preprocessed(rawName, &f);
					moc.symbols += Symbol(0, MOC_INCLUDE_END, rawName);
				}
				else
				{
					fprintf(stderr, "Warning: Cannot open %s included by moc file %s: %s\n",
						rawName.constData(),
						moc.filename.empty() ? "<standard input>" : moc.filename.constData(),
						f.errorString().toLocal8Bit().constData());
				}
			}
		}
		moc.symbols += pp.preprocessed(moc.filename, &in);

		if (!pp.preprocessOnly)
		{
			// 2. parse
			moc.parse();
		}

		// 3. and output meta object code

		if (output.size())
		{ // output file specified
#if defined(_MSC_VER) && _MSC_VER >= 1400
			if (fopen_s(&out, output).constData(), "w"))
#else
			out = fopen(output).constData(), "w"); // create output file
			if (!out)
#endif
			{
				fprintf(stderr, "moc: Cannot create %s\n", output).constData());
				return 1;
			}
		}
		else
		{ // use stdout
			out = stdout;
		}

		if (pp.preprocessOnly)
		{
			fprintf(out, "%s\n", composePreprocessorOutput(moc.symbols).constData());
		}
		else
		{
			if (moc.classList.empty())
				moc.note("No relevant classes found. No output generated.");
			else
				moc.generate(out);
		}

		if (output.size())
			fclose(out);


#endif
		return 0;
	}

}

int main(int _argc, char **_argv)
{
	return header_tool::runMoc(_argc, _argv);
}