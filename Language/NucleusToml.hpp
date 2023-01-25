#ifndef NUCLEUSTOML_HPP
#define NUCLEUSTOML_HPP

#include "../TOML++/toml.h"
#include <iostream>
#include "../String/StringAPI.hpp"
#include <filesystem>

#ifdef _WIN32
#include <Windows.h>
#endif

struct NucleusTOML {
	static std::vector<std::string> folders;
	static std::vector<std::string> CPPIncludes;

	static std::string GetExecutableDirectory() {
		std::string getPathToExe;

		#ifdef _WIN32
			WCHAR path[MAX_PATH];
			GetModuleFileNameW(NULL, path, MAX_PATH);

			std::wstring ws(path);
			std::string str(ws.begin(), ws.end());
			getPathToExe = str;
		#endif

		std::filesystem::path p = getPathToExe.c_str();

		return p.parent_path().u8string();
	}

	static int Read(std::string path) {
		toml::parse_result result = toml::parse_file(path.c_str());
		if (!result) {
			std::cout << "Parsing failed:\n" << result.error() << "\n";
			return 1;
		}

		toml::table tbl = std::move(result).table();

		auto imports = tbl["imports"]["folders"];

		if (toml::array* arr = imports.as_array()) {
			// visitation with for_each() helps deal with heterogeneous data
			arr->for_each([](auto&& el) {
				if constexpr (toml::is_string<decltype(el)>) {
					std::string s = el.get();

					s = StringAPI::RemoveAll(s, "'");
					s = StringAPI::ReplaceAll(s, ".", "/");

					//std::cout << "New Imported Folder: " << s << "\n";

					folders.push_back(s);
				}
			});
		}

		auto cppincludes = tbl["imports.cpp"]["include"];

		if (toml::array* arr = cppincludes.as_array()) {
			// visitation with for_each() helps deal with heterogeneous data
			arr->for_each([](auto&& el) {
				if constexpr (toml::is_string<decltype(el)>) {
					std::string s = el.get();

					CPPIncludes.push_back(s);
				}
			});
		}

		return 0;
	}
};

#endif