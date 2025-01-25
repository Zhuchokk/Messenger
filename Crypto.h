#pragma once
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <numeric>
#include <cmath>
using namespace std;

long long gcd(long long a, long long b);
bool IsPrime(long long& N);
long long Euler(long long& N);
long long Euclid_algo(long long a, long long b);
pair<pair<long long, long long>, long long> key_generation(long long& p, long long& q);
vector<long long> translation(string& message, pair <pair<long long, long long>, long long>& keys);
string retranslation(vector<long long>& server_gift, pair <pair<long long, long long>, long long>& keys);
bool IsGenPos(long long& p, long long& q);
