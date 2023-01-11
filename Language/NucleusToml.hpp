#ifndef NUCLEUSTOML_HPP
#define NUCLEUSTOML_HPP

#include "../TOML++/toml.h"
#include <iostream>
#include "../String/StringAPI.hpp"

struct NucleusTOML
{
	static std::vector<std::string> folders;

	static int Read(std::string path)
	{
		folders.clear();

		toml::parse_result result = toml::parse_file(path.c_str());
    	if (!result)
    	{
    	    std::cout << "Parsing failed:\n" << result.error() << "\n";
    	    return 1;
    	}

    	toml::table tbl = std::move(result).table();

		auto imports = tbl["imports"]["folders"];

		if (toml::array* arr = imports.as_array())
    	{
    	    // visitation with for_each() helps deal with heterogeneous data
    	    arr->for_each([](auto&& el)
    	    {
    	        if constexpr (toml::is_string<decltype(el)>)
    	        {
    	        	std::string s = el.get();

    	        	s = StringAPI::RemoveAll(s, "'");
    	        	s = StringAPI::ReplaceAll(s, ".", "/");

    	        	//std::cout << "New Imported Folder: " << s << "\n";

    	            folders.push_back(s);
    	        }
    	    });
    	}

    	return 0;
	}
};

#endif