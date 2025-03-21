#pragma once
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <numeric>
#include <cmath>
using namespace std;
class Crypto {
public:
	static int gcd(int a, int b);
	static bool IsPrime(int& N);
	static int Euler(int& N);
	static int Euclid_algo(int a, int b);
	static pair<pair<int, int>, int> key_generation(int& p, int& q);
	static vector<char> translation(vector<char>& message, pair <pair<int, int>, int>& keys);
	static vector<char> retranslation(vector<char>& server_gift, pair <pair<int, int>, int>& keys);
	static bool IsGenPos(int& p, int& q);
	static int exponent(int a, int n);
};
