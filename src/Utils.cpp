#include "Utils.h"

#include <iostream>

std::vector<std::string> Utils::SplitString(std::string s, std::string delimeter)
{
	std::vector<std::string> split;

	size_t position = 0;
	std::string token;
	while ((position = s.find(delimeter)) != std::string::npos) {
		token = s.substr(0, position);
		split.push_back(token);
		s.erase(0, position + delimeter.length());
	}
	split.push_back(s);
	return split;
}