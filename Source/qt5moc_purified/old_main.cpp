#include "preprocessor.h"
#include "moc.h"
#include "outputrevision.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <CommandLineOption.h>
#include <commandlineparser.h>

#if 0

#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>

//#include <qcoreapplication.h>
#endif
namespace header_tool
{


	/*
		This function looks at two file names and returns the name of the
		infile with a path relative to outfile.

		Examples:

			/tmp/abc, /tmp/bcd -> abc
			xyz/a/bc, xyz/b/ac -> ../a/bc
			/tmp/abc, xyz/klm -> /tmp/abc
	 */

	// TODO
	static std::string combinePath(const std::string &infile, const std::string &outfile)
	{
		std::filesystem::path cwd(std::filesystem::current_path());
		std::filesystem::path in_file_info(infile);
		std::filesystem::path out_file_info(outfile);
		std::filesystem::current_path(in_file_info);
		const std::string relative_path = out_file_info.relative_path().string();
		std::filesystem::current_path(cwd);

#ifdef PLATFORM_WINDOWS
		// It's a system limitation.
		// It depends on the Win API function which is used by the program to open files.
		// cl apparently uses the functions that have the MAX_PATH limitation.
		if (out_file_info.string().length() + relative_path.length() + 1 >= 260)
			return in_file_info.string();
#endif
		return relative_path;
	}

	void error(const char *msg = "Invalid argument")
	{
		if (msg)
			fprintf(stderr, "moc: %s\n", msg);
	}

	static inline bool hasNext(const std::vector<Symbol> &symbols, int i)
	{
		return (i < symbols.size());
	}

	static inline const Symbol &next(const std::vector<Symbol> &symbols, int &i)
	{
		return symbols.at(i++);
}

	std::string composePreprocessorOutput(const std::vector<Symbol> &symbols)
	{
		std::string output;
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
						output.erase(output.length() - 1);
					else if (secondlast == PP_STRING_LITERAL && last == PP_WHITESPACE)
						output.erase(output.length() - 2);
					else
						break;
					output += sub(sym.lexem(), 1);
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
				memset((void*)(output.data() + output.size() - padding), '\n', padding);
				lineNum = sym.lineNum;
			}

			output += sym.lexem();
		}

		return output;
	}

	static std::vector<std::string> argumentsFromCommandLineAndFile(const std::vector<std::string> &arguments)
	{
		std::vector<std::string> allArguments;
		allArguments.reserve(arguments.size());
		std::string line;
		for (const std::string &argument : arguments)
		{
			// "@file" doesn't start with a '-' so we can't use QCommandLineParser for it
			if (argument[0] == '@')
			{
				std::string optionsFile = argument;
				optionsFile.erase(0, 1);
				if (optionsFile.empty())
				{
					error("The @ option requires an input file");
					return std::vector<std::string>();
				}

				std::ifstream f(optionsFile, std::ios::in);
				if (!f.good())
				{
					error("Cannot open options file specified with @");
					return std::vector<std::string>();
				}
				while (std::getline(f, line))
				{
					if (!line.empty())
					{
						allArguments.push_back(line);
					}
				}
			}
			else
			{
				allArguments.push_back(argument);
			}
		}
		return allArguments;
	}

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
		FILE* in = 0;
		FILE* out = 0;

#pragma region cmdline
		// Note that moc isn't translated.
		// If you use this code as an example for a translated app, make sure to translate the strings.
		CommandLineParser clp;
		clp.setApplicationDescription(string_format("Qt Meta Object Compiler version %1 (Qt %2)",
			mocOutputRevision, "4.8.7"));
		clp.addHelpOption();
		clp.addVersionOption();
		clp.setSingleDashWordOptionMode(CommandLineParser::ParseAsLongOptions);

		CommandLineOption outputOption("o");
		outputOption.setDescription("Write output to file rather than stdout.");
		outputOption.setValueName("file");
		outputOption.setFlags(CommandLineOption::ShortOptionStyle);
		clp.addOption(outputOption);

		CommandLineOption includePathOption("I");
		includePathOption.setDescription("Add dir to the include path for header files.");
		includePathOption.setValueName("dir");
		includePathOption.setFlags(CommandLineOption::ShortOptionStyle);
		clp.addOption(includePathOption);

		CommandLineOption macFrameworkOption("F");
		macFrameworkOption.setDescription("Add Mac framework to the include path for header files.");
		macFrameworkOption.setValueName("framework");
		macFrameworkOption.setFlags(CommandLineOption::ShortOptionStyle);
		clp.addOption(macFrameworkOption);

		CommandLineOption preprocessOption("E");
		preprocessOption.setDescription("Preprocess only; do not generate meta object code.");
		clp.addOption(preprocessOption);

		CommandLineOption defineOption("D");
		defineOption.setDescription("Define macro, with optional definition.");
		defineOption.setValueName("macro[=def]");
		defineOption.setFlags(CommandLineOption::ShortOptionStyle);
		clp.addOption(defineOption);

		CommandLineOption undefineOption("U");
		undefineOption.setDescription("Undefine macro.");
		undefineOption.setValueName("macro");
		undefineOption.setFlags(CommandLineOption::ShortOptionStyle);
		clp.addOption(undefineOption);

		CommandLineOption metadataOption("M");
		metadataOption.setDescription("Add key/value pair to plugin meta data");
		metadataOption.setValueName("key=value");
		metadataOption.setFlags(CommandLineOption::ShortOptionStyle);
		clp.addOption(metadataOption);

		CommandLineOption compilerFlavorOption("compiler-flavor");
		compilerFlavorOption.setDescription("Set the compiler flavor: either \"msvc\" or \"unix\".");
		compilerFlavorOption.setValueName("flavor");
		clp.addOption(compilerFlavorOption);

		CommandLineOption noIncludeOption("i");
		noIncludeOption.setDescription("Do not generate an #include statement.");
		clp.addOption(noIncludeOption);

		CommandLineOption pathPrefixOption("p");
		pathPrefixOption.setDescription("Path prefix for included file.");
		pathPrefixOption.setValueName("path");
		pathPrefixOption.setFlags(CommandLineOption::ShortOptionStyle);
		clp.addOption(pathPrefixOption);

		CommandLineOption forceIncludeOption("f");
		forceIncludeOption.setDescription("Force #include <file> (overwrite default).");
		forceIncludeOption.setValueName("file");
		forceIncludeOption.setFlags(CommandLineOption::ShortOptionStyle);
		clp.addOption(forceIncludeOption);

		CommandLineOption prependIncludeOption("b");
		prependIncludeOption.setDescription("Prepend #include <file> (preserve default include).");
		prependIncludeOption.setValueName("file");
		clp.addOption(prependIncludeOption);

		CommandLineOption includeOption("include");
		includeOption.setDescription("Parse <file> as an #include before the main source(s).");
		includeOption.setValueName("file");
		clp.addOption(includeOption);

		CommandLineOption noNotesWarningsCompatOption("n");
		noNotesWarningsCompatOption.setDescription("Do not display notes (-nn) or warnings (-nw). Compatibility option.");
		noNotesWarningsCompatOption.setValueName("which");
		noNotesWarningsCompatOption.setFlags(CommandLineOption::ShortOptionStyle);
		clp.addOption(noNotesWarningsCompatOption);

		CommandLineOption noNotesOption("no-notes");
		noNotesOption.setDescription("Do not display notes.");
		clp.addOption(noNotesOption);

		CommandLineOption noWarningsOption("no-warnings");
		noWarningsOption.setDescription("Do not display warnings (implies --no-notes).");
		clp.addOption(noWarningsOption);

		CommandLineOption ignoreConflictsOption("ignore-option-clashes");
		ignoreConflictsOption.setDescription("Ignore all options that conflict with compilers, like -pthread conflicting with moc's -p option.");
		clp.addOption(ignoreConflictsOption);

		clp.addPositionalArgument("[header-file]", "Header file to read from, otherwise stdin.");
		clp.addPositionalArgument("[@option-file]", "Read additional options from option-file.");
#pragma endregion cmdline

		std::vector<std::string> arguments;
		for (int i = 0; i < argc; i++)
		{
			arguments.push_back(argv[i]);
		}
		if (arguments.empty())
			return 1;

		clp.process(arguments);
		const std::vector<std::string> files = clp.positionalArguments();
		if (files.size() > 1)
		{
			std::cout << (std::string("Too many input files specified: '") + join(files, "' '") + '\'');
			clp.showHelp(1);
		}
		else if (!files.empty())
		{
			filename = files.front();
		}
		const bool ignoreConflictingOptions = clp.isSet(ignoreConflictsOption);
		output = clp.value(outputOption);
		pp.preprocessOnly = clp.isSet(preprocessOption);
		if (clp.isSet(noIncludeOption))
		{
			moc.noInclude = true;
			autoInclude = false;
		}

		if (!ignoreConflictingOptions)
		{
			if (clp.isSet(forceIncludeOption))
			{
				moc.noInclude = false;
				autoInclude = false;
				const auto forceIncludes = clp.values(forceIncludeOption);
				for (const std::string &include : forceIncludes)
				{
					moc.includeFiles.push_back(include);
					defaultInclude = false;
				}
			}
			const auto prependIncludes = clp.values(prependIncludeOption);
			for (const std::string &include : prependIncludes)
				moc.includeFiles.insert(moc.includeFiles.begin(), include);
			if (clp.isSet(pathPrefixOption))
				moc.includePath = clp.value(pathPrefixOption);
		}

		const auto includePaths = clp.values(includePathOption);
		for (const std::string &path : includePaths)
			pp.includes.emplace_back(path);
		std::string compilerFlavor = clp.value(compilerFlavorOption);
		if (compilerFlavor.empty() || compilerFlavor == "unix")
		{
			/*
			// traditional Unix compilers use both CPATH and CPLUS_INCLUDE_PATH
			// $CPATH feeds to #include <...> and #include "...", whereas
			// CPLUS_INCLUDE_PATH is equivalent to GCC's -isystem, so we parse later
			const auto cpath = qgetenv("CPATH").split(QDir::listSeparator().toLatin1());
			for (const std::string &p : cpath)
				pp.includes.emplace_back(p);
			const auto cplus_include_path = qgetenv("CPLUS_INCLUDE_PATH").split(QDir::listSeparator().toLatin1());
			for (const std::string &p : cplus_include_path)
				pp.includes.emplace_back(p);
				*/
		}
		else if (compilerFlavor == "msvc")
		{
			/*
			// MSVC uses one environment variable: INCLUDE
			const auto include = qgetenv("INCLUDE").split(QDir::listSeparator().toLatin1());
			for (const std::string &p : include)
				pp.includes.emplace_back(p);
				*/
		}
		else
		{
			printf((std::string("Unknown compiler flavor '") + compilerFlavor + "'; valid values are: msvc, unix.").c_str());
			clp.showHelp(1);
		}

		const auto macFrameworks = clp.values(macFrameworkOption);
		for (const std::string &path : macFrameworks)
		{
			// minimalistic framework support for the mac
			Preprocessor::IncludePath p(path);
			p.isFrameworkPath = true;
			pp.includes.emplace_back(p);
		}
		const auto defines = clp.values(defineOption);
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
				clp.showHelp(1);
			}
			Macro macro;
			macro.symbols = Preprocessor::tokenize(value, 1, Preprocessor::TokenizeDefine);
			macro.symbols.pop_back(); // remove the EOF symbol
			pp.macros[name] = macro;
		}
		const auto undefines = clp.values(undefineOption);
		for (const std::string &arg : undefines)
		{
			std::string macro = arg;// .toLocal8Bit();
			if (macro.empty())
			{
				printf("Missing macro name");
				clp.showHelp(1);
			}
			pp.macros.erase(macro);
		}

		const std::vector<std::string> noNotesCompatValues = clp.values(noNotesWarningsCompatOption);
		if (clp.isSet(noNotesOption) || std::find(noNotesCompatValues.begin(), noNotesCompatValues.end(),"n") != noNotesCompatValues.end())
		moc.displayNotes = false;
		if (clp.isSet(noWarningsOption) || std::find(noNotesCompatValues.begin(), noNotesCompatValues.end(),"w") != noNotesCompatValues.end())
		moc.displayWarnings = moc.displayNotes = false;

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
						//moc.includeFiles.push_back(combinePath(filename, output));
					}
					else
					{
						moc.includeFiles.push_back(filename);
					}
				}
			}
			else
			{
				//moc.includeFiles.push_back(combinePath(filename, filename));
			}
		}
		if (filename.empty())
		{
			filename = "standard input";
			//in.open(stdin, QIODevice::ReadOnly);
		}
		else
		{
			in = fopen(filename.c_str(), "r");
			if (!in)
			{
				fprintf(stderr, "moc: %s: No such file\n", filename.c_str());
				return 1;
			}
			moc.filename = filename;
		}

		const auto metadata = clp.values(metadataOption);
		for (const std::string &md : metadata)
		{
			int split = md.find('=');
			std::string key = sub(md, 0, split);
			std::string value = sub(md, split + 1);

			if (split == std::string::npos || key.empty() || value.empty())
			{
				std::cerr << "missing key or value for option '-M'";
			}
			else if (key.find('.') != std::string::npos)
			{
				// Don't allow keys with '.' for now, since we might need this
				// format later for more advanced meta data API
				std::cerr << "A key cannot contain the letter '.' for option '-M'";
			}
			else
			{
				//QJsonArray array = moc.metaArgs.value(key);
				//array.append(value);
				//moc.metaArgs.insert(key, array);
			}
		}

		moc.currentFilenames.push(filename);
		moc.includes = pp.includes;

		// 1. preprocess
		const auto includeFiles = clp.values(includeOption);
		for (const std::string &includeName : includeFiles)
		{
			std::string rawName = pp.resolveInclude(includeName, moc.filename);
			if (rawName.empty())
			{
				fprintf(stderr, "Warning: Failed to resolve include \"%s\" for moc file %s\n",
					includeName.data(),
					moc.filename.empty() ? "<standard input>" : moc.filename.data());
			}
			else
			{
				FILE* f = fopen(rawName.c_str(), "w");
				if (f)
				{
					moc.symbols.emplace_back(0, MOC_INCLUDE_BEGIN, rawName);
					auto temp = pp.preprocessed(rawName, f);
					moc.symbols.insert(moc.symbols.end(), temp.begin(), temp.end());
					moc.symbols.emplace_back(0, MOC_INCLUDE_END, rawName);
				}
				else
				{
					fprintf(stderr, "Warning: Cannot open %s included by moc file %s: %s\n",
						rawName.data(),
						moc.filename.empty() ? "<standard input>" : moc.filename.data(), "error"
						/*f.errorString().toLocal8Bit().constData()*/);
				}
			}
		}

		auto temp = pp.preprocessed(moc.filename, in);
		moc.symbols.insert(moc.symbols.end(), temp.begin(), temp.end());

		if (!pp.preprocessOnly)
		{
			// 2. parse
			moc.parse();
		}

		// 3. and output meta object code

		if (output.size())
		{ // output file specified
#if defined(_MSC_VER) && _MSC_VER >= 1400
			if (fopen_s(&out, output.c_str(), "w"))
#else
			out = fopen(output).constData(), "w"); // create output file
			if (!out)
#endif
			{
				fprintf(stderr, "moc: Cannot create %s\n", output.c_str());
				return 1;
			}
		}
		else
		{ // use stdout
			out = stdout;
		}

		if (pp.preprocessOnly)
		{
			fprintf(out, "%s\n", composePreprocessorOutput(moc.symbols).data());
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

		return 0;
	}

}

int main(int _argc, char **_argv)
{
	return header_tool::runMoc(_argc, _argv);
}