#include "WebInterface.h"
#include<vector>

class Client {
private:
	pair<pair<int, int>, int> key;
	vector<vector<char>> available_users = { {} };
	SOCKET ClientSock;
	int namelong;
	int crypt_len;
	vector<char> myname;

	void RecieveData(SOCKET self);
public:
	Client() = default;
	int Start();
	int Work();
};

class Administrator : protected Client {
	int Work();
};