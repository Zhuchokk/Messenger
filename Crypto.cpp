#pragma once

#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <numeric>
#include <cmath>

#include "Crypto.h"
using namespace std;

//finding GCD of two numbers
int Crypto::gcd(int a, int b) {
	if (b == 0)
		return a;
	return gcd(b, a % b);
}

//checking a number for simplicity by iterating through the odd numbers up to sqrt(N)
bool Crypto::IsPrime(int &N) {
	if (N == 1) {
		return 0;
	}
	else {
		if (N == 2) {
			return 1;
		}
		else {
			if (N % 2 == 0) {
				return 0;
			}
			else {
				double n = sqrt(N);
				int flag = 0;
				for (int i = 3; i <= n; i++) {
					if (N % i == 0) {
						return 0;
						flag = 1;
						break;
					}
				}
				if (flag == 0) {
					return 1;
				}
			}
		}
	}
}

//Euler function (counting the number of numbers less than N and mutually prime with it)
int Crypto::Euler(int& N) {
	int sum;
	if (N == 1) {
		sum = 1;
	}
	else {
		if ((IsPrime(N) == 1) || (N == 2)) {
			sum = N - 1;
		}
		else {
			if ((N == 4) || (N == 6)) {
				sum = 2;
			}
			else {
				sum = 2;
				if (N % 2 == 0) {
					for (int i = 3; i < N / 2; i = i + 2) {
						if (gcd(N, i) == 1) {
							sum = sum + 2;
							//cout << i << " " << N - i << endl;
							//учитываем i и N - i
						}
					}
				}
				else {
					sum = 4;
					for (int i = 3; i <= N / 2; i++) {
						if (gcd(N, i) == 1) {
							sum = sum + 2;
							//cout << i << " " << N - i << endl;
						}
					}
				}
			}
		}
	}
	return sum;
}

//Euclid's advanced algorithm
int Crypto::Euclid_algo(int a, int b) {
	if (a < 1 or b < 2) {
		return -1;
	}
	int u1 = b;
	int u2 = 0;
	int v1 = a;
	int v2 = 1;
	while (v1 != 0) {
		int q = u1 / v1;
		int t1 = u1 - q * v1;
		int t2 = u2 - q * v2;
		u1 = v1;
		u2 = v2;
		v1 = t1;
		v2 = t2;
	}
	return u1 == 1 ? (u2 + b) % b : -1;
}

//checking the keys generation capability
bool Crypto::IsGenPos(int& p, int& q) {
	if ((IsPrime(p) == 0) || (IsPrime(q) == 0)) {
		return 0;
	}
	else {
		if (p * q >= 256) {
			return 1;
		}
		else {
			return 0;
		}
	}
}

//open and private keys generation
pair<pair<int, int>, int> Crypto::key_generation(int &p, int &q) {
	int N = p * q;
	int f = Euler(N);
	int e = f - 1;
	int d = Euclid_algo(e, f);
	pair <pair<int, int>, int> a;
	a.first.first = e;
	a.first.second = d;
	a.second = N;
	//cout << "e: " << e << endl << "d: " << d << endl << "N: " << N << endl;
	return a;
}

//encrypting an incoming message
vector<char> Crypto::translation(vector<char>& message, pair <pair<int, int>, int> &keys) {
	vector<int> trans1(message.size()), trans2(message.size());
	vector<char> mutation;
	for (int i = 0; i < message.size(); i++) {
		trans1[i] = (int)message[i];
		if (trans1[i] < 0) {
			trans1[i] = trans1[i] + 256;
		}
	}

	/*cout << "trans1: ";
	for (int i = 0; i < trans1.size(); i++) {
		cout << trans1[i] << " ";
	}
	cout << endl;*/

	for (int i = 0; i < message.size(); i++) {
		int count = 0;
		int res = 1;
		while (count < keys.first.first) {
			res = (res * trans1[i]) % keys.second;
			count++;
		}
		trans2[i] = res;
	}
	/*cout << "trans2: ";
	for (int i = 0; i < trans2.size(); i++) {
		cout << trans2[i] << " ";
	}
	cout << endl;*/

	for (int i = 0; i < message.size(); i++) {
		while (trans2[i] != 0) {
			mutation.push_back(char(trans2[i] % 10 + 48));
			trans2[i] = trans2[i] / 10;
		}
		mutation.push_back(char(45));
	}
	
	/*cout << "mutation: ";
	for (int i = 0; i < mutation.size(); i++) {
		cout << mutation[i] << " ";
	}
	cout << endl;*/

	return mutation;
}

//exponentiation
int Crypto::exponent(int a, int n) {
	if (n == 0) {
		return 1;
	}
	else {
		return a * exponent(a, n - 1);
	}
}

//decryption and assembly of the final message
vector<char> Crypto::retranslation(vector<char> &server_gift, pair <pair<int, int>, int> &keys) {
	vector<int> sg;
	int a = 0, count = 0;
	for (int i = 0; i < server_gift.size(); i++) {
		if ((int)server_gift[i] != 45) {
			a = a + ((int)server_gift[i] - 48) * exponent(10, count);
			count++;
		}
		else {
			sg.push_back(a);
			a = 0;
			count = 0;
		}
	}
	vector<int> gift(sg.size());
	vector<char> answer;
	for (int i = 0; i < sg.size(); i++) {
		int count = 0;
		int res = 1;
		while (count < keys.first.second) {
			res = (res * sg[i]) % keys.second;
			count++;
		}
		gift[i] = res;
		answer.push_back(char(res));
	}

	/*cout << "gift: ";
	for (int i = 0; i < sg.size(); i++) {
		cout << gift[i] << " ";
	}
	cout << endl;*/

	return answer;
}

/*int main() {
	cout << "Введите два простых числа p и q:" << endl;
	int p, q;
	cin >> p >> q;
	int a = Crypto::IsGenPos(p, q);

	while (a == 0) {
		cout << "Невозможно сгенерировать ключи." << endl << "Введите два простых числа p и q:" << endl;
		cin >> p >> q;
		a = Crypto::IsGenPos(p, q);
	}
	pair<pair<int, int>, int> work = Crypto::key_generation(p, q);
	string input;
	getline(cin, input);
	vector<char> symbols;
	for (int i = 0; i < input.size(); i++) {
		symbols.push_back(input[i]);
	}
	
	vector<char> to_server = Crypto::translation(symbols, work);
	vector<char> result = Crypto::retranslation(to_server, work);
	for (int i = 0; i < result.size(); i++) {
		cout << result[i];
	}
	return 0;
}*/