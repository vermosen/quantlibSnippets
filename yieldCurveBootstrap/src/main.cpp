//============================================================================
// Name        : yieldCurve.cpp
// Author      : jean-mathieu vermosen
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>

#include <ql/time/all.hpp>
#include <ql/currencies/america.hpp>
#include <ql/settings.hpp>
#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>

namespace ql = QuantLib;

struct Datum
{
    ql::Integer 	n		;
    ql::TimeUnit 	units	;
    ql::Rate 		rate	;
};

int main()
{
	// static data
	const ql::Calendar calendar = ql::UnitedStates();
	const ql::Natural settlementDays = 2;
	const ql::Date today = ql::Date::todaysDate();

	ql::Settings::instance().evaluationDate() = today;
	ql::Date settlement = calendar.advance(today, settlementDays, ql::Days);

	std::vector<Datum> depositData =
	{
		{ 1, ql::Months, 0.04581 },
		{ 2, ql::Months, 0.04573 },
		{ 3, ql::Months, 0.04557 },
		{ 6, ql::Months, 0.04496 },
		{ 9, ql::Months, 0.04490 }
	};

	std::vector<Datum> swapData =
	{
		{  1, ql::Years, 0.0454 },
		{  5, ql::Years, 0.0499 },
		{ 10, ql::Years, 0.0547 },
		{ 20, ql::Years, 0.0589 },
		{ 30, ql::Years, 0.0596 }
	};

	// create the rate helpers
	std::vector<boost::shared_ptr<ql::RateHelper>> instruments;

	for(auto & it : depositData)
	{
		instruments.push_back(
			boost::shared_ptr<ql::RateHelper>(
				new ql::DepositRateHelper(
					it.rate,
					it.n * it.units,
					settlementDays, calendar,
					ql::ModifiedFollowing, true,
					ql::Actual360())));
	}

	boost::shared_ptr<ql::IborIndex> index(
		new ql::IborIndex("US libor 3M",
			 3 * ql::Months,
			 settlementDays,
			 ql::USDCurrency(),
			 calendar,
			 ql::ModifiedFollowing,
			 false,
			 ql::Actual360()));

	index->addFixing(settlement, depositData[2].rate);

	for(auto & it : swapData)
	{
		instruments.push_back(
			boost::shared_ptr<ql::RateHelper>(
				new ql::SwapRateHelper(
					it.rate,
					it.n * it.units,
					calendar, ql::Semiannual,
					ql::Unadjusted, ql::Thirty360(),
					index)));
	}

	auto ts = boost::shared_ptr<ql::YieldTermStructure>(
		new ql::PiecewiseYieldCurve<ql::ForwardRate, ql::Cubic>(
			settlement, instruments, ql::Actual360()));

	for(int i = 0; i < 10; i++)
	{
		ql::Date t = settlement + ql::Period(i, ql::Years);

		std::cout
			<< "On " << t
			<< ", zero rate is "
			<< ts->zeroRate(t, ql::Actual365Fixed(), ql::Continuous)
			<<  std::endl;
	}

	return 0;
}
