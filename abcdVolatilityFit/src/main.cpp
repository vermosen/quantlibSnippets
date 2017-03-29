//============================================================================
// Name        : lmmCovariance.cpp
// Author      : jean-mathieu vermosen
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <array>
#include <iostream>
#include <algorithm>

#include <ql/time/date.hpp>
#include <ql/time/imm.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/time/calendars/unitedkingdom.hpp>
#include <ql/time/calendars/jointcalendar.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/currencies/america.hpp>
#include <ql/settings.hpp>
#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/models/marketmodels/all.hpp>

#include <ql/termstructures/volatility/abcd.hpp>
#include <ql/termstructures/volatility/abcdcalibration.hpp>

namespace ql = QuantLib;

struct datum
{
    ql::Integer 	n		;
    ql::TimeUnit 	units	;
    ql::Rate 		rate	;
};

struct futureDatum
{
	std::string	code 	;
	ql::Real 	price	;
    ql::Real 	convAdj ;
};

constexpr std::array<datum, 4> depositData
{{
	datum{ 1, ql::Months, 0.0098278 },
	datum{ 3, ql::Months, 0.0115128 },
	datum{ 6, ql::Months, 0.0142711 },
	datum{ 1, ql::Years , 0.0180289 }
}};

const std::array<futureDatum, 16> futureData
{{
	futureDatum{ "J7", 98.83 , 0.00000	},
	futureDatum{ "K7", 98.83 , 0.00000	},
	futureDatum{ "M7", 98.715, 0.00080	},
	futureDatum{ "N7", 98.675, 0.00000	},
	futureDatum{ "Q7", 98.635, 0.00000	},
	futureDatum{ "U7", 98.58 , 0.00129	},
	futureDatum{ "Z7", 98.45 , 0.00695	},
	futureDatum{ "H8", 98.34 , 0.01108	},
	futureDatum{ "M8", 98.22 , 0.01578	},
	futureDatum{ "U8", 98.11 , 0.02209	},
	futureDatum{ "Z8", 97.99 , 0.03182	},
	futureDatum{ "H9", 97.915, 0.04502	},
	futureDatum{ "M9", 97.845, 0.05276	},
	futureDatum{ "U9", 97.775, 0.06018	},
	futureDatum{ "Z9", 97.69 , 0.07022	},
	futureDatum{ "H0", 97.64 , 0.07855	}
}};

constexpr std::array<datum, 2> swapData
{{
	datum{ 4, ql::Years, 0.01939880 },
	datum{ 5, ql::Years, 0.02049567 }
}};

// all 3M maturity contracts
constexpr std::array<datum, 11> mktVols
{{
	datum{ 3 , ql::Months, 0.251521478090478 },
	datum{ 6 , ql::Months, 0.297208210878435 },
	datum{ 9 , ql::Months, 0.366644109706502 },
	datum{ 1 , ql::Years , 0.433800427414978 },
	datum{ 15, ql::Months, 0.485821043603255 },
	datum{ 18, ql::Months, 0.534148940744967 },
	datum{ 21, ql::Months, 0.595165151639790 },
	datum{ 2 , ql::Years , 0.655092846188407 },
	datum{ 3 , ql::Years , 0.795388077730595 },
	datum{ 4 , ql::Years , 0.851994683383768 },
	datum{ 5 , ql::Years , 0.869720024181853 }
}};

int main()
{
	// US libor calendar
	ql::Calendar calendar = ql::JointCalendar(
		ql::UnitedStates(),
		ql::UnitedKingdom(),
		ql::JointCalendarRule::JoinHolidays);

	ql::Natural settlementDays = 2;
	ql::DayCounter curveDc = ql::Actual365Fixed();
	ql::Date todaysDate = ql::Date(24, ql::Mar, 2017);
	ql::Date settlementDate = calendar.advance(
			todaysDate, settlementDays * ql::Days, ql::Following);
	ql::Settings::instance().evaluationDate() = todaysDate;

	// create the libor index
	boost::shared_ptr<ql::IborIndex> index(
		new ql::IborIndex("US libor 3M",
			 3 * ql::Months, settlementDays,
			 ql::USDCurrency(), calendar,
			 ql::ModifiedFollowing,
			 false, ql::Actual360()));

	index->addFixing(settlementDate, (double)depositData[1].rate, true);

	// create the libor curve
	std::vector<boost::shared_ptr<ql::RateHelper>> instruments;

	for(auto & it : depositData)
		instruments.push_back(
			boost::shared_ptr<ql::RateHelper>(
				new ql::DepositRateHelper(
					it.rate,
					it.n * it.units,
					settlementDays, calendar,
					ql::ModifiedFollowing, true,
					ql::Actual360())));

	for(auto & it : futureData)
		instruments.push_back(
			boost::shared_ptr<ql::RateHelper>(
				new ql::FuturesRateHelper(
					ql::Handle<ql::Quote>(
						boost::shared_ptr<ql::Quote>(
							new ql::SimpleQuote(it.price))),
					ql::IMM::date(it.code, todaysDate),
					index,
					ql::Handle<ql::Quote>(
						boost::shared_ptr<ql::Quote>(
							new ql::SimpleQuote(it.convAdj))))));

	for(auto & it : swapData)
		instruments.push_back(
			boost::shared_ptr<ql::RateHelper>(
				new ql::SwapRateHelper(
					it.rate,
					it.n * it.units,
					calendar, ql::Semiannual,
					ql::Unadjusted, ql::Thirty360(),
					index)));

	auto ts = boost::shared_ptr<ql::YieldTermStructure>(
		new ql::PiecewiseYieldCurve<ql::ForwardRate, ql::Cubic>(
			settlementDate, instruments, curveDc));

	// 3m libors, different expiries
	std::vector<ql::Date> startDates, endDates;

	startDates.push_back(settlementDate);

	for (auto & it : mktVols)
		startDates.push_back(calendar.advance(
			settlementDate, it.n * it.units, ql::Following));

	std::for_each(startDates.begin(), startDates.end(),
	[&calendar, &endDates](ql::Date dt)
	{
		endDates.push_back(calendar.advance(dt, 3 * ql::Months, ql::Following));
	});

	std::vector<ql::Time> rateTimes, paymentTimes, accruals;

	std::for_each(startDates.begin(), startDates.end(),
	[&curveDc, &settlementDate, &rateTimes](ql::Date i)
	{
		rateTimes.push_back(curveDc.yearFraction(settlementDate, i));
	});

	std::for_each(endDates.begin(), endDates.end(),
	[&curveDc, &settlementDate, &paymentTimes](ql::Date i)
	{
		paymentTimes.push_back(curveDc.yearFraction(settlementDate, i));
	});

	int i = 0; std::for_each(rateTimes.begin(), rateTimes.end(),
	[&i, &paymentTimes, &accruals](ql::Time t)
	{
		accruals.push_back(paymentTimes[i++] - t);
	});

	auto blackVols = std::vector<ql::Volatility>();

	for (auto & it : mktVols)
		blackVols.push_back(it.rate);

	ql::AbcdCalibration instVol(std::vector<ql::Time>(
			rateTimes.begin(), rateTimes.end() - 1), blackVols);

	instVol.compute();

	std::cout
		<< "calibration completed"
		<< ": a=" << instVol.a()
		<< ", b=" << instVol.b()
		<< ", c=" << instVol.c()
		<< ", d=" << instVol.d()
		<< std::endl;

	// create the abcd vol curve
	ql::AbcdFunction f(instVol.a(), instVol.b(), instVol.c(), instVol.d());

//	std::for_each(paymentTimes.begin(), paymentTimes.end(),
//	[&instVol](ql::Time & t)
//	{
//		std::cout << t << ": " << instVol.value(t) << ", " <<  << std::endl;
//	});

	return 0;

}
