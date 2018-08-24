# define _CRT_SECURE_NO_WARNINGS 1
#include<iostream>
#include<windows.h>
using namespace std;

#include"date.h"

int main()
{
	Date d1(2017, 3, 1);
	Date d2(2016, 2, 1);
	cout << d1 - d2 << endl;
	system("pause");
	return 0;
}
