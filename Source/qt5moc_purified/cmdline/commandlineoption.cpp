#include "commandlineoption.h"

#include <set>

#define Q_NEVER_INLINE
#define Q_DECL_NOEXCEPT

namespace header_tool
{

	class CommandLineOptionPrivate // : public QSharedData
	{
	public:
		Q_NEVER_INLINE
			explicit CommandLineOptionPrivate(const std::string &name)
		{
			names.push_back(name);
			removeInvalidNames(names);
		}

		Q_NEVER_INLINE
			explicit CommandLineOptionPrivate(const std::vector<std::string> &names)
		{
			//a.splice(a.end(), b);
			this->names.insert(this->names.begin(), names.begin(), names.end());
			removeInvalidNames(this->names);
		}

		static void removeInvalidNames(std::vector<std::string>& nameList);

		//! The list of names used for this option.
		std::vector<std::string> names;

		//! The documentation name for the value, if one is expected
		//! Example: "-o <file>" means valueName == "file"
		std::string valueName;

		//! The description used for this option.
		std::string description;

		//! The list of default values used for this option.
		std::vector<std::string> defaultValues;

		CommandLineOption::Flags flags;
	};

	/*!
		\since 5.2
		\class CommandLineOption
		\brief The CommandLineOption class defines a possible command-line option.
		\inmodule QtCore
		\ingroup shared
		\ingroup tools

		This class is used to describe an option on the command line. It allows
		different ways of defining the same option with multiple aliases possible.
		It is also used to describe how the option is used - it may be a flag (e.g. \c{-v})
		or take a value (e.g. \c{-o file}).

		Examples:
		\snippet code/src_corelib_tools_CommandLineOption.cpp 0

		\sa QCommandLineParser
	*/

	/*!
		\fn CommandLineOption &CommandLineOption::operator=(CommandLineOption &&other)

		Move-assigns \a other to this CommandLineOption instance.

		\since 5.2
	*/

	/*!
		Constructs a command line option object with the name \a name.

		The name can be either short or long. If the name is one character in
		length, it is considered a short name. Option names must not be empty,
		must not start with a dash or a slash character, must not contain a \c{=}
		and cannot be repeated.

		\sa setDescription(), setValueName(), setDefaultValues()
	*/
	CommandLineOption::CommandLineOption(const std::string &name)
		: d(new CommandLineOptionPrivate(name))
	{}

	/*!
		Constructs a command line option object with the names \a names.

		This overload allows to set multiple names for the option, for instance
		\c{o} and \c{output}.

		The names can be either short or long. Any name in the list that is one
		character in length is a short name. Option names must not be empty,
		must not start with a dash or a slash character, must not contain a \c{=}
		and cannot be repeated.

		\sa setDescription(), setValueName(), setDefaultValues()
	*/
	CommandLineOption::CommandLineOption(const std::vector<std::string> &names)
		: d(new CommandLineOptionPrivate(names))
	{}

	/*!
		Constructs a command line option object with the given arguments.

		The name of the option is set to \a name.
		The name can be either short or long. If the name is one character in
		length, it is considered a short name. Option names must not be empty,
		must not start with a dash or a slash character, must not contain a \c{=}
		and cannot be repeated.

		The description is set to \a description. It is customary to add a "."
		at the end of the description.

		In addition, the \a valueName needs to be set if the option expects a value.
		The default value for the option is set to \a defaultValue.

		In Qt versions before 5.4, this constructor was \c explicit. In Qt 5.4
		and later, it no longer is and can be used for C++11-style uniform
		initialization:

		\snippet code/src_corelib_tools_CommandLineOption.cpp cxx11-init

		\sa setDescription(), setValueName(), setDefaultValues()
	*/
	CommandLineOption::CommandLineOption(const std::string &name, const std::string &description,
		const std::string &valueName,
		const std::string &defaultValue)
		: d(new CommandLineOptionPrivate(name))
	{
		setValueName(valueName);
		setDescription(description);
		setDefaultValue(defaultValue);
	}

	/*!
		Constructs a command line option object with the given arguments.

		This overload allows to set multiple names for the option, for instance
		\c{o} and \c{output}.

		The names of the option are set to \a names.
		The names can be either short or long. Any name in the list that is one
		character in length is a short name. Option names must not be empty,
		must not start with a dash or a slash character, must not contain a \c{=}
		and cannot be repeated.

		The description is set to \a description. It is customary to add a "."
		at the end of the description.

		In addition, the \a valueName needs to be set if the option expects a value.
		The default value for the option is set to \a defaultValue.

		In Qt versions before 5.4, this constructor was \c explicit. In Qt 5.4
		and later, it no longer is and can be used for C++11-style uniform
		initialization:

		\snippet code/src_corelib_tools_CommandLineOption.cpp cxx11-init-list

		\sa setDescription(), setValueName(), setDefaultValues()
	*/
	CommandLineOption::CommandLineOption(const std::vector<std::string> &names, const std::string &description,
		const std::string &valueName,
		const std::string &defaultValue)
		: d(new CommandLineOptionPrivate(names))
	{
		setValueName(valueName);
		setDescription(description);
		setDefaultValue(defaultValue);
	}

	/*!
		Constructs a CommandLineOption object that is a copy of the CommandLineOption
		object \a other.

		\sa operator=()
	*/
	CommandLineOption::CommandLineOption(const CommandLineOption &other)
		: d(other.d)
	{}

	/*!
		Destroys the command line option object.
	*/
	CommandLineOption::~CommandLineOption()
	{}

	/*!
		Makes a copy of the \a other object and assigns it to this CommandLineOption
		object.
	*/
	CommandLineOption &CommandLineOption::operator=(const CommandLineOption &other)
	{
		d = other.d;
		return *this;
	}

	/*!
		\fn void CommandLineOption::swap(CommandLineOption &other)

		Swaps option \a other with this option. This operation is very
		fast and never fails.
	*/

	/*!
		Returns the names set for this option.
	 */
	std::vector<std::string> CommandLineOption::names() const
	{
		return d->names;
	}

	namespace
	{
		struct IsInvalidName
		{
			typedef bool result_type;
			typedef std::string argument_type;

			Q_NEVER_INLINE
				result_type operator()(const std::string &name) const Q_DECL_NOEXCEPT
			{
				if (/* Q_UNLIKELY */ (name.empty()))
					return warn("be empty");

				const char c = name.at(0);
				if (/* Q_UNLIKELY */ (c == '-'))
					return warn("start with a '-'");
				if (/* Q_UNLIKELY */ (c == '/'))
					return warn("start with a '/'");
				if (/* Q_UNLIKELY */ (name.find('=') != std::string::npos))
					return warn("contain a '='");

				return false;
			}

			Q_NEVER_INLINE
				static bool warn(const char *what) Q_DECL_NOEXCEPT
			{
				/* qWarning */ printf("CommandLineOption: Option names cannot %s", what);
				return true;
			}
		};
	} // unnamed namespace

	// static
	void CommandLineOptionPrivate::removeInvalidNames(std::vector<std::string>& nameList)
	{
		if (/* Q_UNLIKELY */ (nameList.empty()))
			/* qWarning */ printf("CommandLineOption: Options must have at least one name");
		else
		{
			nameList.erase(std::remove_if(nameList.begin(), nameList.end(), IsInvalidName()), nameList.end());
		}
	}

	/*!
		Sets the name of the expected value, for the documentation, to \a valueName.

		Options without a value assigned have a boolean-like behavior:
		either the user specifies --option or they don't.

		Options with a value assigned need to set a name for the expected value,
		for the documentation of the option in the help output. An option with names \c{o} and \c{output},
		and a value name of \c{file} will appear as \c{-o, --output <file>}.

		Call QCommandLineParser::value() if you expect the option to be present
		only once, and QCommandLineParser::values() if you expect that option
		to be present multiple times.

		\sa valueName()
	 */
	void CommandLineOption::setValueName(const std::string &valueName)
	{
		d->valueName = valueName;
	}

	/*!
		Returns the name of the expected value.

		If empty, the option doesn't take a value.

		\sa setValueName()
	 */
	std::string CommandLineOption::valueName() const
	{
		return d->valueName;
	}

	/*!
		Sets the description used for this option to \a description.

		It is customary to add a "." at the end of the description.

		The description is used by QCommandLineParser::showHelp().

		\sa description()
	 */
	void CommandLineOption::setDescription(const std::string &description)
	{
		d->description = description;
	}

	/*!
		Returns the description set for this option.

		\sa setDescription()
	 */
	std::string CommandLineOption::description() const
	{
		return d->description;
	}

	/*!
		Sets the default value used for this option to \a defaultValue.

		The default value is used if the user of the application does not specify
		the option on the command line.

		If \a defaultValue is empty, the option has no default values.

		\sa defaultValues() setDefaultValues()
	 */
	void CommandLineOption::setDefaultValue(const std::string &defaultValue)
	{
		std::vector<std::string> newDefaultValues;
		if (!defaultValue.empty())
		{
			newDefaultValues.resize(1);
			newDefaultValues.push_back(defaultValue);
		}
		// commit:
		d->defaultValues.swap(newDefaultValues);
	}

	/*!
		Sets the list of default values used for this option to \a defaultValues.

		The default values are used if the user of the application does not specify
		the option on the command line.

		\sa defaultValues() setDefaultValue()
	 */
	void CommandLineOption::setDefaultValues(const std::vector<std::string> &defaultValues)
	{
		d->defaultValues = defaultValues;
	}

	/*!
		Returns the default values set for this option.

		\sa setDefaultValues()
	 */
	std::vector<std::string> CommandLineOption::defaultValues() const
	{
		return d->defaultValues;
	}

#if 0
#if QT_DEPRECATED_SINCE(5, 8)
	/*!
		Sets whether to hide this option in the user-visible help output.

		All options are visible by default. Setting \a hide to true for
		a particular option makes it internal, i.e. not listed in the help output.

		\since 5.6
		\obsolete Use setFlags(CommandLineOption::HiddenFromHelp), CommandLineOption::HiddenFromHelp
		\sa isHidden
	 */
	void CommandLineOption::setHidden(bool hide)
	{
		d->flags.setFlag(HiddenFromHelp, hide);
	}

	/*!
		Returns true if this option is omitted from the help output,
		false if the option is listed.

		\since 5.6
		\obsolete Use flags() & CommandLineOption::HiddenFromHelp
		\sa setHidden(), CommandLineOption::HiddenFromHelp
	 */
	bool CommandLineOption::isHidden() const
	{
		return d->flags & HiddenFromHelp;
	}
#endif
#endif

	/*!
		Returns a set of flags that affect this command-line option.

		\since 5.8
		\sa setFlags(), CommandLineOption::Flags
	 */
	/*
	CommandLineOption::Flags CommandLineOption::flags() const
	{
		return d->flags;
	}
	*/
	/*!
		Set the set of flags that affect this command-line option to \a flags.

		\since 5.8
		\sa flags(), CommandLineOption::Flags
	 */

	void CommandLineOption::setFlags(Flags flags)
	{
		d->flags = flags;
	}
	
	/*!
		\enum CommandLineOption::Flag

		\value HiddenFromHelp Hide this option in the user-visible help output. All
		options are visible by default. Setting this flag for a particular
		option makes it internal, i.e. not listed in the help output.

		\value ShortOptionStyle The option will always be understood as a short
		option, regardless of what was set by
		QCommandLineParser::setSingleDashWordOptionMode.
		This allows flags such as \c{-DDEFINE=VALUE} or \c{-I/include/path} to be
		interpreted as short flags even when the parser is in
		QCommandLineParser::ParseAsLongOptions mode.

		\sa CommandLineOption::setFlags(), CommandLineOption::flags()
	*/

}
