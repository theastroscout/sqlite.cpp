# SQLite C++ Wrapper


### Installation

```cpp
#include "sqlite.hpp"
surfy::SQLiteDB db;

#include "json.hpp" // https://github.com/nlohmann/json/blob/develop/single_include/nlohmann/json.hpp
using json = nlohmann::ordered_json;

surfy::SQLiteDB db;

int main() {
	db.connect("example.db");
	return 0;
}


```

### Compilation
```bash

g++ application.cpp -lsqlite3

```


### db.findOne(query[, params])

```cpp

json result;

// Simple Query
result = db.findOne("SELECT * FROM users WHERE name='John';");

// Using Params

std::vector<std::string> params;
params.push_back("John");

result = db.findOne("SELECT * FROM users WHERE name=?;", params);

/*

@result json {
	"id": 1,
	"name": "John",
	"username": "Doe"
}

*/
```


### db.findSync(query[, params])

```cpp
json result;

// Simple Query
result = db.findSync("SELECT * FROM users WHERE name='John';");

// Using Params
std::vector<std::string> params;
params.push_back("John");
result = db.findSync("SELECT * FROM users WHERE name=?;", params);

/*

@result json [
	{
		"id": 1,
		"name": "John",
		"username": "Doe",
		"assets": {}
	},
	{
		"id": 2,
		"name": "John",
		"username": "Doe Jr.",
		"assets": {}
	},
	{
		"id": 2,
		"name": "John",
		"username": "Doe Sr.",
		"assets": {
			"car": "nope",
			"house": "nope",
			"stableJob": false,
			"dept": true
		}
	}
]

*/

```


### db.find(query, callback[, params])
Callback will be invoked for each row.

```cpp

int main() {
	json result;

	// Simple Query
	result = db.find("SELECT * FROM users WHERE name='John';", callback);

	// Using Params
	std::vector<std::string> params;
	params.push_back("John");
	result = db.find("SELECT * FROM users WHERE name=?;", callback, params);

	/*

	@result json {
		"status": true // false
	}

	*/
}

void callback(const json& row) {

	/*

	@row json {
		"id": 1,
		"name": "John",
		"username": "Doe"
	}

	*/

	std::cout << "Name: " << row["name"] << std::endl;
}

```