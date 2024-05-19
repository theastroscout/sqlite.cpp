# SQLite C++ Wrapper


### Installation

```cpp
#include "include/surfy/sqlite/sqlite.hpp"
surfy::SQLiteDB db;

#include "include/json.hpp" // https://github.com/nlohmann/json/blob/develop/single_include/nlohmann/json.hpp
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


### db.findOne(query[, params]);

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
	"username": "Doe",
	"_status": true
}

@error_result json {
	"_status": false,
	"_msg": "Some error message"
}

*/
```

### db.find(query);
```cpp

json result = db.find("SELECT * FROM users WHERE name='John';");
if (result._status) {
	for ( int i = 0, l = result.rows.size(); i < l; ++i) {
		json row = result.rows[i];
	}
} else {
	std::cout << result._msg << std::endl;
}

```


### db.find(query[, params[, callback]);
Callback will be invoked for each row.

```cpp

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

int main() {
	json result;

	// Simple Query
	result = db.find("SELECT * FROM users WHERE name='John';", {}, callback);

	// Using Params
	std::vector<std::string> params;
	params.push_back("John");
	result = db.find("SELECT * FROM users WHERE name=?;", params, callback);

	/*

	@result json {
		"_status": true, // false
		"_msg": std::string, // If error was occured
		"rows": std::vector<json> // If callback wasn't set
	}

	*/
}

```