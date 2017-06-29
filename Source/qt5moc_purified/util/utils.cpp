#include "utils.h"

namespace header_tool
{
	template<typename T>
	T sub(const T& vec, size_t pos, size_t len)
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
}