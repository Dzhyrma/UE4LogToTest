#include "stdafx.h"

#include <time.h>
#include <unordered_set>
#include <vector>
#include <iostream>
#include <iomanip>

#include "JUnitXmlCreator.h"

const std::regex JUnitXmlCreator::TEST_START_REGEX = std::regex("^\\[(\\d+\\.\\d+\\.\\d+\\-\\d+\\.\\d+\\.\\d+):(\\d+)\\].*Running Automation: '(.*)' \\(Class Name: '(.*)'\\)$");
const std::regex JUnitXmlCreator::TEST_END_REGEX = std::regex("^\\[(\\d+\\.\\d+\\.\\d+\\-\\d+\\.\\d+\\.\\d+):(\\d+)\\].*Automation Test (Succeeded|Failed) \\((.*)\\)$");

const std::regex JUnitXmlCreator::TEST_CASE_REGEX = std::regex("^\\[(\\d+\\.\\d+\\.\\d+\\-\\d+\\.\\d+\\.\\d+):(\\d+)\\]\\[\\d+\\](Error)?:?\\s*(.*):\\s*(Expected .*)$");

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
	std::string testname = "";
	std::string className = "";
	std::vector<std::string> testcases;
	std::vector<std::string> testcaseStacktraces;
	std::vector<std::tm> testcaseTimestamps;
	std::vector<int> testcaseTimestampsMillis;
	std::unordered_set<int> failedIndices;
	std::tm testStartTime = {};
	int testStartTimeMillis = 0;
	while (std::getline(infile, line)) {
		std::smatch sm;
		if (testname != "")
		{
			if (std::regex_match(line, sm, TEST_END_REGEX))
			{
				std::tm testEndTime = {};
				parseTime(sm[1], testEndTime);
				int testEndTimeMillis = std::stoi(sm[2]);
				int diffSeconds, diffMillis;
				calculateTimeDiff(testStartTime, testStartTimeMillis, testEndTime, testEndTimeMillis, diffSeconds, diffMillis);
				outfile << "\t<testsuite name=\"" << testname << "\""
					<< " errors=\"0\" tests=\"" << testcases.size() << "\""
					<< " skipped=\"0\" failures=\"" << failedIndices.size() << "\""
					<< " time=\"" << diffSeconds << "." << std::setfill('0') << std::setw(3) << diffMillis << "\""
					<< " timestamp=\"" << std::put_time(&testStartTime, "%Y-%m-%dT%H:%M:%S") << "\">" << std::endl;

				std::tm lastTestStartTime = testStartTime;
				int lastTestStartTimeMillis = testStartTimeMillis;
				for (int i = 0; i < testcases.size(); i++)
				{
					calculateTimeDiff(lastTestStartTime, lastTestStartTimeMillis, testcaseTimestamps[i], testcaseTimestampsMillis[i], diffSeconds, diffMillis);
					lastTestStartTime = testcaseTimestamps[i];
					lastTestStartTimeMillis = testcaseTimestampsMillis[i];

					outfile << "\t\t<testcase classname=\"" << className << "\""
						<< " name=\"" << testcases[i] << "\""
						<< " time=\"" << diffSeconds << "." << std::setfill('0') << std::setw(3) << diffMillis << "\"";
					if (failedIndices.count(i) == 0)
					{
						outfile << "/>" << std::endl;
					}
					else
					{
						outfile << ">" << std::endl;
						outfile << "\t\t\t<failure message=\"" << testcases[i] << "\">"
							<< testcaseStacktraces[i] << ": [" << className << "] assertion failed</failure>" << std::endl;
						outfile << "\t\t</testcase>" << std::endl;
					}
				}

				outfile << "\t</testsuite>" << std::endl;
				testname = "";
				className = "";
				testcases.clear();
				testcaseStacktraces.clear();
				testcaseTimestamps.clear();
				testcaseTimestampsMillis.clear();
				failedIndices.clear();
			}
			else if (std::regex_match(line, sm, TEST_CASE_REGEX))
			{
				if (sm[3] == "Error") {
					failedIndices.emplace(testcases.size());
				}
				std::tm testcaseTime = {};
				parseTime(sm[1], testcaseTime);
				testcaseTimestamps.push_back(testcaseTime);
				testcaseTimestampsMillis.push_back(std::stoi(sm[2]));
				testcaseStacktraces.push_back(sm[4]);
				testcases.push_back(sm[5]);
			}
		}

		if (std::regex_match(line, sm, TEST_START_REGEX))
		{
			testname = sm[3];
			className = sm[4];

			parseTime(sm[1], testStartTime);
			testStartTimeMillis = std::stoi(sm[2]);
		}
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