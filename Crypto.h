#pragma once
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <numeric>
#include <cmath>
using namespace std;

int gcd(int a, int b);
bool IsPrime(int& N);
int Euler(int& N);
int Euclid_algo(int a, int b);
pair<pair<int, int>, int> key_generation(int& p, int& q);
vector<char> translation(vector<char>& message, pair <pair<int, int>, int>& keys);
vector<char> retranslation(vector<char>& server_gift, pair <pair<int, int>, int>& keys);
bool IsGenPos(int& p, int& q);
int exponent(int a, int n);
