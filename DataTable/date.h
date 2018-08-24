#pragma once
#include<iostream>
#include<windows.h>
#include<assert.h>
using namespace std;

class Date
{

public:
	Date(int year = 1900, int month = 1, int day = 1)
		:_year(year)
		, _month(month)
		, _day(day)
		//判断日期是否合法 在默认构造函数内加断言
	{
		assert(IsInvalid());
	}
	void Display()
	{
		cout << _year << "_" << _month << "_" << _day << endl;
	}

	Date& operator=(const Date& d)
	{
		if (this != &d)
		{
			_year = d._year;
			_month = d._month;
			_day = d._day;
		}
		return *this;
	}

	bool IsInvalid()
	{
		if (_year > 1900&& _month>0
			&& _month<13 && _day>0 
			&& _day <= Getmonthdays(_year, _month))
		{
			return true;
		}
		return false;
	}

	int Getmonthdays(int year, int month)
	{
		int monthDays[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
		if (month != 2)
		{
			return monthDays[month];
		}
		else
		{
			if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)
			{
				return 29;
			}
			return 28;
		}
	} 
	Date operator + (int day)
	{
		if (day < 0)
			return *this - (-day);
		_day += day;
		while (IsInvalid() == false)
		{
			if (_day >= 365)
			{
				if ((_year % 4 == 0 && _year % 100 != 0) || _year % 400 == 0)
				{
					_day -= 366;
					_year++;
				}
				_day -= 365;
				_year++;
			}
			else
			{
				if (_day >= Getmonthdays(_year, _month))
				{
					if (12 == _month)
					{
						_month = 1;
						_year++;	
					}
					else
					{
						_month++;	
					}
					_day = _day - Getmonthdays(_year, _month);
				}
			}
		}
		return *this;
	}
	Date operator - (int day)
	{
		if (day < 0)
			return *this + (-day);
		Date tmp(*this);
		tmp._day -= day;
		while (tmp.IsInvalid() == false)
		{
			if (tmp._month == 1)
			{
				tmp._year--;
				tmp._month = 12;
			}
			else
			{
				tmp._month--;
			}
			int day = Getmonthdays(tmp._year, tmp._month);
			tmp._day += day;
		}
		return tmp;
	}
	Date operator += (int day)
	{
		*this = *this + 100;
		return *this;
	}
	Date operator -= (int day)
	{
		return *this = *this - day;
	}
	inline Date& operator++()
	{
		*this += 1;
		return *this;
	}
	inline Date operator++(int) 
	{
		Date tmp(*this);
		*this = *this + 1;
		return tmp;
	}
	//int operator-(const Date& d)
	//{
	//	Date MinDate(*this);
	//	Date MaxDate(d);
	//	int days = 0;
	//	if (*this > d)
	//	{
	//		MinDate = d;
	//		MaxDate = *this;
	//	}if (*this < d)
	//	{
	//		MinDate = *this;
	//		MaxDate = d;
	//	}
	//		while (MinDate != MaxDate)//较小的日期每次增加一天，直到与较大日期相等
	//		{
	//			MinDate += 1;
	//			days++;//记录天数
	//		}
	//		return days;
	//	}
	//}
	int operator-(const Date& d)
	{
		Date MinDate(*this);
		Date MaxDate(d);
		int days = 0;
		if (_year > d._year)
		{
			MinDate = d;
			MaxDate = *this;
		}if (_year < d._year)
		{
			MinDate = *this;
			MaxDate = d;
		}
		while (MinDate._year != MaxDate._year)
		{
			if (((MaxDate._year % 4 == 0 && MaxDate._year % 100 != 0)|| MaxDate._year % 400 == 0)//闰年是大的年份
				&& (MaxDate._month>2))
			{
				days += 366;
				MinDate._year++;
			}
			else if ((((MinDate._year % 4 == 0 && MinDate._year % 100 != 0)|| MinDate._year % 400 == 0) //闰年是小的年份
				&& (MinDate._month<3 )))
			{
				days += 366;
				MinDate._year++;
			}
			else
			{
				days += 365;
				MinDate._year++;
			}
		}

		if (_month > d._month)
		{
			MinDate = d;
			MaxDate = *this;
		}if (_month < d._month)
		{
			MinDate = *this;
			MaxDate =d;
		}
		while (MinDate._month != MaxDate._month)
		{
			if (MinDate._year>MaxDate._year)
			{

				if (MinDate._month == 2)
				{
					days += -28;
					MinDate._month++;
				}
				else
				{

					days += (-Getmonthdays(MinDate._year, MinDate._month));
					MinDate._month++;
				}	
			}

			if (MinDate._year<MaxDate._year)
			{
				if (MinDate._month == 2)
				{
					days += 28;
					MinDate._month++;
				}
				else
				{
					days += Getmonthdays(MinDate._year, MinDate._month);
					MinDate._month++;
				}
			}
		}
		if (MinDate._day <MaxDate._day)
		{
			days += (MaxDate._day - MinDate._day);
		}
		if (MinDate._day>MaxDate._day)
		{
			days += (-(MinDate._day - MaxDate._day));
		}
		return days;
	}
	bool operator == (const Date& d)
	{
		return (_year == d._year&& _month == d._month&&_day == d._day);
	}
	bool operator != (const Date& d)
	{
		return !(*this == d);
	}
	bool operator >(const Date& d)
	{
		if (_year > d._year || 
			(_year == d._year&&_month > d._month) 
			|| (_year == d._year&&_month == d._month && _day > d._day))
		{
			return true;
		}
		return false;
	}
	bool operator < (const Date& d)
	{
		return !(*this > d);
	}
	bool operator >= (const Date& d)
	{
		return (*this == d) && (*this > d);
	}
	bool operator <= (const Date& d)
	{
		return (*this == d) && (*this < d);
	}

private:
	int _year;
	int _month;
	int _day;
};
