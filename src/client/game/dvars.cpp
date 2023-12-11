#include <std_include.hpp>
#include <utils/string.hpp>

#include "dvars.hpp"
#include "game.hpp"

namespace dvars
{
	std::unordered_map<std::uint32_t, dvar_info> dvar_map;

	game::dvar_t* con_inputBoxColor = nullptr;
	game::dvar_t* con_inputHintBoxColor = nullptr;
	game::dvar_t* con_outputBarColor = nullptr;
	game::dvar_t* con_outputSliderColor = nullptr;
	game::dvar_t* con_outputWindowColor = nullptr;
	game::dvar_t* con_inputDvarMatchColor = nullptr;
	game::dvar_t* con_inputDvarValueColor = nullptr;
	game::dvar_t* con_inputDvarInactiveValueColor = nullptr;
	game::dvar_t* con_inputCmdMatchColor = nullptr;

	game::dvar_t* jump_enableFallDamage = nullptr;
	game::dvar_t* jump_ladderPushVel = nullptr;
	game::dvar_t* jump_spreadAdd = nullptr;

	game::dvar_t* pm_bouncing = nullptr;

	game::dvar_t* r_fullbright = nullptr;

	game::dvar_t* cg_legacyCrashHandling = nullptr;

	game::dvar_t* g_enableElevators = nullptr;

	std::string dvar_get_vector_domain(const int components, const game::dvar_limits& domain)
	{
		if (domain.vector.min == -FLT_MAX)
		{
			if (domain.vector.max == FLT_MAX)
			{
				return utils::string::va("Domain is any %iD vector", components);
			}
			else
			{
				return utils::string::va("Domain is any %iD vector with components %g or smaller", components,
				                         domain.vector.max);
			}
		}
		else if (domain.vector.max == FLT_MAX)
		{
			return utils::string::va("Domain is any %iD vector with components %g or bigger", components,
			                         domain.vector.min);
		}
		else
		{
			return utils::string::va("Domain is any %iD vector with components from %g to %g", components,
			                         domain.vector.min, domain.vector.max);
		}
	}

	std::string dvar_get_domain(const game::dvar_type type, const game::dvar_limits& domain)
	{
		std::string str;

		switch (type)
		{
		case game::dvar_type::boolean:
			return "Domain is 0 or 1"s;

		case game::dvar_type::value:
			if (domain.value.min == -FLT_MAX)
			{
				if (domain.value.max == FLT_MAX)
				{
					return "Domain is any number"s;
				}

				return utils::string::va("Domain is any number %g or smaller", domain.value.max);
			}

			if (domain.value.max == FLT_MAX)
			{
				return utils::string::va("Domain is any number %g or bigger", domain.value.min);
			}

			return utils::string::va("Domain is any number from %g to %g", domain.value.min, domain.value.max);

		case game::dvar_type::vec2:
			return dvar_get_vector_domain(2, domain);

		case game::dvar_type::rgb:
		case game::dvar_type::vec3:
			return dvar_get_vector_domain(3, domain);

		case game::dvar_type::vec4:
			return dvar_get_vector_domain(4, domain);

		case game::dvar_type::integer:
			if (domain.enumeration.stringCount == INT_MIN)
			{
				if (domain.integer.max == INT_MAX)
				{
					return "Domain is any integer"s;
				}

				return utils::string::va("Domain is any integer %i or smaller", domain.integer.max);
			}

			if (domain.integer.max == INT_MAX)
			{
				return utils::string::va("Domain is any integer %i or bigger", domain.integer.min);
			}

			return utils::string::va("Domain is any integer from %i to %i", domain.integer.min, domain.integer.max);

		case game::dvar_type::color:
			return "Domain is any 4-component color, in RGBA format"s;

		case game::dvar_type::enumeration:
			str = "Domain is one of the following:"s;

			for (auto string_index = 0; string_index < domain.enumeration.stringCount; ++string_index)
			{
				str += utils::string::va("\n  %2i: %s", string_index, domain.enumeration.strings[string_index]);
			}

			return str;

		case game::dvar_type::string:
			return "Domain is any text"s;

		default:
			return utils::string::va("unhandled dvar type '%i'", type);
		}
	}

	constexpr int generate_hash(const char* string)
	{
		const char* v1;
		char v2, v6;
		int v4, v5, v7;
		char* end_ptr;

		v1 = string;
		v2 = *string;

		if (v2 == 48 && v1[1] == 120)
		{
			return strtoul(v1 + 2, &end_ptr, 16);
		}

		v4 = v2;

		if ((v2 - 65) <= 0x19u)
		{
			v4 = v2 + 32;
		}

		v5 = 0xB3CB2E29 * static_cast<unsigned int>(v4 ^ 0x319712C3);

		if (v2)
		{
			do
			{
				v6 = *++v1;
				v7 = v6;
				if ((v6 - 65) <= 0x19u)
				{
					v7 = v6 + 32;
				}

				v5 = 0xB3CB2E29 * static_cast<unsigned int>(v5 ^ v7);
			} while (v6);
		}

		return v5;
	}

	void insert_dvar_info(const std::uint32_t hash, const std::string& name, const std::string& description)
	{
		dvar_info info{};
		info.hash = hash;
		info.name = name;
		info.description = description;
		dvar_map.insert(std::make_pair(hash, info));
	}

	void insert_dvar_info(const std::string& name, const std::string& description)
	{
		insert_dvar_info(game::generateHashValue(name.data()), name, description);
	}

	std::optional<dvar_info> get_dvar_info_from_hash(const std::uint32_t hash)
	{
		const auto iter = dvar_map.find(hash);
		if (iter != dvar_map.end())
		{
			return iter->second;
		}

		return {};
	}

	std::string dvar_get_description(const std::string& name)
	{
		const auto hash = game::generateHashValue(name.data());
		const auto info = get_dvar_info_from_hash(hash);

		if (info.has_value())
		{
			return info->description;
		}

		return {};
	}

	std::string hash_to_string(const int hash)
	{
		return utils::string::va("0x%lX", hash);
	}

	game::dvar_t* register_int(const std::string& name, int value, int min, int max,
		unsigned int flags, const std::string& description)
	{
		const auto hash = game::generateHashValue(name.data());
		insert_dvar_info(hash, name, description);
		return game::Dvar_RegisterInt(hash, "", value, min, max, flags);
	}

	game::dvar_t* register_bool(const std::string& name, bool value,
		unsigned int flags, const std::string& description)
	{
		const auto hash = game::generateHashValue(name.data());
		insert_dvar_info(hash, name, description);
		return game::Dvar_RegisterBool(hash, "", value, flags);
	}

	game::dvar_t* register_string(const std::string& name, const char* value,
		unsigned int flags, const std::string& description)
	{
		const auto hash = game::generateHashValue(name.data());
		insert_dvar_info(hash, name, description);
		return game::Dvar_RegisterString(hash, "", value, flags);
	}

	game::dvar_t* register_float(const std::string& name, float value, float min,
		float max, unsigned int flags, const std::string& description)
	{
		const auto hash = game::generateHashValue(name.data());
		insert_dvar_info(hash, name, description);
		return game::Dvar_RegisterFloat(hash, "", value, min, max, flags);
	}

	game::dvar_t* register_vec4(const std::string& name, float x, float y, float z,
		float w, float min, float max, unsigned int flags, const std::string& description)
	{
		const auto hash = game::generateHashValue(name.data());
		insert_dvar_info(hash, name, description);
		return game::Dvar_RegisterVec4(hash, "", x, y, z, w, min, max, flags);
	}

	game::dvar_t* register_enum(const std::string& name, const char** value_list, int default_index, 
		unsigned int flags, const std::string& description)
	{
		const auto hash = game::generateHashValue(name.data());
		insert_dvar_info(hash, name, description);
		return game::Dvar_RegisterEnum(hash, "", value_list, default_index, flags);
	}
}
