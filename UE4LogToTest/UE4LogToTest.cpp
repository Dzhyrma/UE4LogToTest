// UE4LogToTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "JUnitXmlCreator.h"

int main(int argc, char** argv)
{
	if (argc != 3) return 1;

	JUnitXmlCreator reportCreator(argv[1], argv[2]);
	reportCreator.parseTestcases();

    return 0;
}

