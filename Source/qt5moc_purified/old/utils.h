#pragma once
#ifndef UTILS_H
#define UTILS_H

//#include <QtCore/qglobal.h>
#include <string>

namespace header_tool
{
	inline std::string replace_all(std::string str, const std::string& from, const std::string& to)
	{
		size_t start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != std::string::npos)
		{
			str.replace(start_pos, from.length(), to);
			start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
		}
		return str;
	}

	inline bool is_whitespace(char s)
	{
		return (s == ' ' || s == '\t' || s == '\n');
	}

	inline bool is_space(char s)
	{
		return (s == ' ' || s == '\t');
	}

	inline bool is_ident_start(char s)
	{
		return ((s >= 'a' && s <= 'z')
			|| (s >= 'A' && s <= 'Z')
			|| s == '_' || s == '$'
			);
	}

	inline bool is_ident_char(char s)
	{
		return ((s >= 'a' && s <= 'z')
			|| (s >= 'A' && s <= 'Z')
			|| (s >= '0' && s <= '9')
			|| s == '_' || s == '$'
			);
	}

	inline bool is_identifier(const char *s, int len)
	{
		if (len < 1)
			return false;
		if (!is_ident_start(*s))
			return false;
		for (int i = 1; i < len; ++i)
			if (!is_ident_char(s[i]))
				return false;
		return true;
	}

	inline bool is_digit_char(char s)
	{
		return (s >= '0' && s <= '9');
	}

	inline bool is_octal_char(char s)
	{
		return (s >= '0' && s <= '7');
	}

	inline bool is_hex_char(char s)
	{
		return ((s >= 'a' && s <= 'f')
			|| (s >= 'A' && s <= 'F')
			|| (s >= '0' && s <= '9')
			);
	}

	inline const char *skipQuote(const char *data)
	{
		while (*data && (*data != '\"'))
		{
			if (*data == '\\')
			{
				++data;
				if (!*data) break;
			}
			++data;
		}

		if (*data)  //Skip last quote
			++data;
		return data;
	}

	using str = std::string;

	template<typename T, typename... Args, typename U = size_t>
	T sub(const T& vec, U pos, U len = -1)
	{
		if (pos >= vec.size())
		{
			return T();
		}

		if (len < 0)
		{
			len = vec.size() - pos;
		}

		if (pos < 0)
		{
			len += pos;
			pos = 0;
		}

		if (len + pos > vec.size())
		{
			len = vec.size() - pos;
		}
		if (pos == 0 && len == vec.size())
		{
			return vec;
		}
		return T(vec.data() + pos, vec.data() + pos + len);
	}
	/*
	template<typename T>
	T sub(const T& vec, size_t pos)
	{
		return sub(vec, pos, -1);
	}
	*/
}

#endif // UTILS_H
