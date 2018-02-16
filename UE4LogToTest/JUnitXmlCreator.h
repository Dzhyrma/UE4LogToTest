#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <regex>

class JUnitXmlCreator {
public:
	JUnitXmlCreator(char* inputFileName, char* outputFileName);
	~JUnitXmlCreator();

	void parseTestcases();
private:
	void parseTime(const std::string& dateTimeString, std::tm& time);
	void calculateTimeDiff(std::tm& startTime, int startTimeMillis, std::tm& endTime, int endTimeMillis, int& diffInSeconds, int& diffPartMillis);

private:
	static const std::regex TEST_START_REGEX;
	static const std::regex TEST_END_REGEX;

	static const std::regex TEST_CASE_REGEX;

	char* infilePath;
	char* outfilePath;
};
