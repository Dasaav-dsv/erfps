#pragma once

#include <cstdint>
#include <string>

class IndType
{
public:
	IndType(const char* str)
	{
		this->Set(str);
	}

	constexpr const uint32_t Get() const
	{
		return this->Type;
	}

	constexpr const char* GetStr() const
	{
		return this->TypeStr[this->Type];
	}

	void Set(const char* str)
	{
		for (int i = 0; i < this->TypeNum; i++)
		{
			if (_stricmp(str, this->TypeStr[i]) == 0)
			{
				this->Type = i;

				return;
			}
		}

		this->Type = this->TypeDefault;
	}

private:
	uint32_t Type;
	static const uint32_t TypeDefault = 0;
	static constexpr const char* const TypeStr[] =
	{
		"LARGE",
		"SMALL",
		"ITEMS"
	};
	static const uint32_t TypeNum = sizeof(TypeStr) / sizeof(char*);
};