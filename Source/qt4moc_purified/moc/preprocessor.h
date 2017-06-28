#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include "parser.h"
#include <vector>
#include <list>
#include <set>
#include <unordered_map>
#include <stdio.h>

namespace header_tool
{

	struct Macro
	{
		Symbols symbols;
	};

#ifdef USE_LEXEM_STORE
	typedef std::vector<unsigned char> MacroName;
#else
	typedef SubArray MacroName;
#endif
	typedef std::unordered_map<MacroName, Macro> Macros;
	typedef std::vector<MacroName> MacroSafeSet;


	class Preprocessor : public Parser
	{
	public:
		Preprocessor()
		{}
		static bool preprocessOnly;
		struct IncludePath
		{
			inline explicit IncludePath( const std::vector<unsigned char> &_path )
				: path( _path ), isFrameworkPath( false )
			{}
			std::vector<unsigned char> path;
			bool isFrameworkPath;
		};
		std::list<IncludePath> includes;
		std::list<std::vector<unsigned char>> frameworks;
		std::set<std::vector<unsigned char>> preprocessedIncludes;
		Macros macros;
		Symbols preprocessed( const std::vector<unsigned char> &filename, FILE *file );


		void skipUntilEndif();
		bool skipBranch();

		void substituteMacro( const MacroName &macro, Symbols &substituted, MacroSafeSet safeset = MacroSafeSet() );
		void substituteUntilNewline( Symbols &substituted, MacroSafeSet safeset = MacroSafeSet() );

		int evaluateCondition();


	private:
		void until( Token );

		void preprocess( const std::vector<unsigned char> &filename, Symbols &preprocessed );
	};

}

#endif // PREPROCESSOR_H
