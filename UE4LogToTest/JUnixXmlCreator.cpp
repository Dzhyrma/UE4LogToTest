#include "stdafx.h"

#include <unordered_set>
#include <vector>
#include <map>
#include <iostream>
#include <iomanip>
#include <string>

#include "JUnitXmlCreator.h"

const std::regex JUnitXmlCreator::TEST_START_REGEX = std::regex("^\\[(\\d+\\.\\d+\\.\\d+\\-\\d+\\.\\d+\\.\\d+):(\\d+)\\].*LogAutomationController: Display: Test Started. Name=\\{(.*)\\}$");
const std::regex JUnitXmlCreator::TEST_END_REGEX = std::regex("^\\[(\\d+\\.\\d+\\.\\d+\\-\\d+\\.\\d+\\.\\d+):(\\d+)\\].*LogAutomationController: .*: Test Completed. Result=\\{(.*)\\} Name=\\{(.*)\\} Path=\\{(.*)\\}$");
const std::regex JUnitXmlCreator::TEST_ERROR_REGEX = std::regex("^\\[(\\d+\\.\\d+\\.\\d+\\-\\d+\\.\\d+\\.\\d+):(\\d+)\\].*LogAutomationController: Error: (.*) \\[(.*)\\]$");

JUnitXmlCreator::JUnitXmlCreator(char* inputFileName, char* outputFileName) :
	outfilePath(outputFileName),
	infilePath(inputFileName)
{
}

JUnitXmlCreator::~JUnitXmlCreator()
{
}

void JUnitXmlCreator::parseTestcases()
{
	std::ifstream infile(infilePath);
	std::ofstream outfile(outfilePath);

	outfile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
	outfile << "<testsuites>" << std::endl;

	std::string line;
	std::string testname = "Unknown";
	std::tm testStartTime = {};
	int testStartTimeMillis = 0;

	std::map<std::string, TestSuite> testSuites;
	TestCase* lastTestCase = nullptr;

	while (std::getline(infile, line)) {
		std::smatch sm;
		if (std::regex_match(line, sm, TEST_END_REGEX))
		{
			std::string suiteName = sm[5];
			std::size_t pos = suiteName.find(sm[4]);
			if (pos > 0)
			{
				suiteName = suiteName.substr(0, pos - 1);
			}

			if (testSuites.find(suiteName) == testSuites.end())
			{
				testSuites.emplace(suiteName, TestSuite(suiteName));
			}

			TestCase testCase = TestCase();
			testCase.name = sm[4];
			testCase.startTime = testStartTime;
			testCase.startTimeMillis = testStartTimeMillis;
			parseTime(sm[1], testCase.endTime);
			testCase.endTimeMillis = std::stoi(sm[2]);
			testCase.result = sm[3];

			TestSuite& testSuite = testSuites[suiteName];

			if (testCase.result == "Failed")
			{
				testSuite.errors++;
			}

			testSuite.testCases.push_back(testCase);

			lastTestCase = &testSuite.testCases[testSuite.testCases.size() - 1];
		}
		if (std::regex_match(line, sm, TEST_START_REGEX))
		{
			testname = sm[3];

			parseTime(sm[1], testStartTime);
			testStartTimeMillis = std::stoi(sm[2]);
		}
		if (std::regex_match(line, sm, TEST_ERROR_REGEX))
		{
			if (lastTestCase != nullptr)
			{
				lastTestCase->errors.push_back(std::pair<std::string, std::string>(sm[3], sm[4]));
			}
		}

	}

	for (std::map<std::string, TestSuite>::iterator it = testSuites.begin(); it != testSuites.end(); ++it)
	{
		TestSuite& testSuite = it->second;
		if (testSuite.testCases.size() == 0) continue;

		TestCase& firstTestCase = testSuite.testCases[0];
		TestCase& lastTestCase = testSuite.testCases[testSuite.testCases.size() - 1];

		int diffSeconds, diffMillis;
		calculateTimeDiff(firstTestCase.startTime, firstTestCase.startTimeMillis, lastTestCase.endTime, lastTestCase.endTimeMillis, diffSeconds, diffMillis);

		outfile << "\t<testsuite name=\"" << testSuite.name << "\""
			<< " errors=\"0\" tests=\"" << testSuite.testCases.size() << "\""
			<< " skipped=\"0\" failures=\"" << testSuite.errors << "\""
			<< " time=\"" << diffSeconds << "." << std::setfill('0') << std::setw(3) << diffMillis << "\""
			//<< " timestamp=\"" << std::put_time(&testStartTime, "%Y-%m-%dT%H:%M:%S") << "\">" << std::endl;
			<< ">" << std::endl;

		for (int i = 0; i < testSuite.testCases.size(); i++)
		{
			TestCase& testCase = testSuite.testCases[i];
			calculateTimeDiff(testCase.startTime, testCase.startTimeMillis, testCase.endTime, testCase.endTimeMillis, diffSeconds, diffMillis);

			outfile << "\t\t<testcase classname=\"" << testSuite.name << "\""
				<< " name=\"" << testCase.name << "\""
				<< " time=\"" << diffSeconds << "." << std::setfill('0') << std::setw(3) << diffMillis << "\"";
			if (testCase.result == "Passed")
			{
				outfile << "/>" << std::endl;
			}
			else
			{
				outfile << ">" << std::endl;
				outfile << "\t\t\t<failure message=\"Assertion(s) Failed\">" << std::endl;
				
				for (int j = 0; j < testCase.errors.size(); j++)
				{
					outfile << "\t\t\t\t" << testCase.errors[j].first << " [" << testCase.errors[j].second << "]" << std::endl;
				}

				outfile << "\t\t\t</failure>" << std::endl;
				outfile << "\t\t</testcase>" << std::endl;
			}
		}
		outfile << "\t</testsuite>" << std::endl;
	}

	outfile << "</testsuites>" << std::endl;
	outfile.close();
}

void JUnitXmlCreator::parseTime(const std::string& dateTimeString, std::tm& time)
{
	std::istringstream ss(dateTimeString);
	ss >> std::get_time(&time, "%Y.%m.%d-%H.%M.%S");
}

void JUnitXmlCreator::calculateTimeDiff(std::tm& startTime, int startTimeMillis, std::tm& endTime, int endTimeMillis, int& diffInSeconds, int& diffPartMillis)
{
	time_t startTime_t = mktime(&startTime);
	time_t endTime_t = mktime(&endTime);
	diffInSeconds = difftime(endTime_t, startTime_t);
	diffPartMillis = endTimeMillis - startTimeMillis;
	if (diffPartMillis < 0)
	{
		diffPartMillis += 1000;
		diffInSeconds--;
	}
}