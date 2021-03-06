#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <time.h>
#include <regex>

class JUnitXmlCreator {
public:
	struct TestCase
	{
		std::string name;

		std::tm startTime;
		int startTimeMillis;

		std::tm endTime;
		int endTimeMillis;

		std::string result;

		std::vector<std::pair<std::string, std::string>> errors;
	};

	struct TestSuite 
	{
		TestSuite() :
			name("")
		{
		}

		TestSuite(std::string name) :
			name(name)
		{
		}

		int errors = 0;

		std::string name;

		std::vector<TestCase> testCases;
	};

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
	static const std::regex TEST_ERROR_REGEX;

	char* infilePath;
	char* outfilePath;
};
