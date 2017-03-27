//============================================================================
// Name        : yieldCurve.cpp
// Author      : jean-mathieu vermosen
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <ql/time/date.hpp>
#include <ql/time/calendars/unitedStates.hpp>

namespace ql = QuantLib;

int main()
{
	// static data
	const ql::Calendar calendar = ql::UnitedStates();
	const ql::Natural settlementDays = 2;
	const ql::Date today = ql::Date::todaysDate();

	ql::Date settlement = calendar.advance(today,settlementDays,ql::Days);

	std::cout << settlement << std::endl;
	return 0;
}
