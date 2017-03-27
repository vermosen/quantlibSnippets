//============================================================================
// Name        : helloWorld.cpp
// Author      : jean-mathieu vermosen
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <ql/time/date.hpp>

int main()
{
	std::cout << "!!!Hello World!!!" << std::endl; // prints !!!Hello World!!!
	std::cout << "Today's date is: " << QuantLib::Date::todaysDate() << std::endl;
	return 0;
}
