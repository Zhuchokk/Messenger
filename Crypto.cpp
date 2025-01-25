#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <numeric>
#include <cmath>
using namespace std;

//finding GCD of two numbers
long long gcd(long long a, long long b) {
	if (b == 0)
		return a;
	return gcd(b, a % b);
}

//checking a number for simplicity by iterating through the odd numbers up to sqrt(N)
bool IsPrime(long long &N) {
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
				for (long long i = 3; i <= n; i++) {
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
long long Euler(long long& N) {
	long long sum;
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
					for (long long i = 3; i < N / 2; i = i + 2) {
						if (gcd(N, i) == 1) {
							sum = sum + 2;
							//cout << i << " " << N - i << endl;
							//учитываем i и N - i
						}
					}
				}
				else {
					sum = 4;
					for (long long i = 3; i <= N / 2; i++) {
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
long long Euclid_algo(long long a, long long b) {
	if (a < 1 or b < 2) {
		return -1;
	}
	long long u1 = b;
	long long u2 = 0;
	long long v1 = a;
	long long v2 = 1;
	while (v1 != 0) {
		long long q = u1 / v1;
		long long t1 = u1 - q * v1;
		long long t2 = u2 - q * v2;
		u1 = v1;
		u2 = v2;
		v1 = t1;
		v2 = t2;
	}
	return u1 == 1 ? (u2 + b) % b : -1;
}

//checking the keys generation capability
bool IsGenPos(long long& p, long long& q) {
	if (p * q >= 256) {
		return 1;
	}
	else {
		return 0;
	}
}

//open and private keys generation
pair<pair<long long, long long>, long long> key_generation(long long &p, long long &q) {
	long long N = p * q;
	long long f = Euler(N);
	long long e = 691;
	long long d = Euclid_algo(e, f);
	pair <pair<long long, long long>, long long> a;
	a.first.first = e;
	a.first.second = d;
	a.second = N;
	//cout << "e: " << e << endl << "d: " << d << endl << "N: " << N << endl;
	return a;
}

//encrypting an incoming message
vector<long long> translation(string& message, pair <pair<long long, long long>, long long> &keys) {
	vector<long long> trans1(message.size()), trans2(message.size());
	for (long long i = 0; i < message.size(); i++) {
		trans1[i] = (int)message[i];
	}

	/*cout << "trans1: ";
	for (long long i = 0; i < trans1.size(); i++) {
		cout << trans1[i] << " ";
	}
	cout << endl;*/

	for (long long i = 0; i < message.size(); i++) {
		long long count = 0;
		long long res = 1;
		while (count < keys.first.first) {
			res = (res * trans1[i]) % keys.second;
			count++;
		}
		trans2[i] = res;
	}
	/*cout << "trans2: ";
	for (long long i = 0; i < trans2.size(); i++) {
		cout << trans2[i] << " ";
	}
	cout << endl;*/

	return trans2;
}

//decryption and assembly of the final message
string retranslation(vector<long long> &server_gift, pair <pair<long long, long long>, long long> &keys) {
	vector<long long> gift(server_gift.size());
	string answer;
	for (long long i = 0; i < server_gift.size(); i++) {
		long long count = 0;
		long long res = 1;
		while (count < keys.first.second) {
			res = (res * server_gift[i]) % keys.second;
			count++;
		}
		gift[i] = res;
		answer.push_back(char(res));
	}

	/*cout << "gift: ";
	for (long long i = 0; i < server_gift.size(); i++) {
		cout << gift[i] << " ";
	}
	cout << endl;*/

	return answer;
}

int main() {
	cout << "Введите два простых числа p и q:" << endl;
	long long p, q;
	cin >> p >> q;
	int a = IsGenPos(p, q);

	while (a == 0) {
		cout << "Невозможно сгенерировать ключи." << endl << "Введите два простых числа p и q:" << endl;
		cin >> p >> q;
		a = IsGenPos(p, q);
	}
	pair<pair<long long, long long>, long long> work = key_generation(p, q);
	string input;
	getline(cin, input);
	vector<long long> to_server = translation(input, work);
	string result = retranslation(to_server, work);
	cout << result;
	return 0;
}