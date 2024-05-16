/*

For multithreading is better to run new instance for each thread

*/

#ifndef SURFY_SQLITE_HPP
#define SURFY_SQLITE_HPP

#include <sqlite3.h>
#include <iostream>

#ifndef SURFY_HPP
#include "../json.h"
using json = nlohmann::ordered_json;
#endif

namespace surfy {

	class SQLite {
		sqlite3* db;
		// sqlite3_stmt* stmt;

	public:
		SQLite() {
			
		}

		~SQLite() {
			sqlite3_close(db);
		}

		bool connect(const char* dbName, bool extensions = false) {
			int rc = sqlite3_open(dbName, &db);
			if (rc) {
				std::cerr << "@surfy::SQLite:: Can't open database: " << sqlite3_errmsg(db) << std::endl;
				sqlite3_close(db);
				return false;
			} else {
				std::cout << "@surfy::SQLite:: Connected to " << dbName << std::endl;
			}

			/*

			Enable Extensions

			*/

			if (extensions) {
				// Enable loading of extensions
				rc = sqlite3_enable_load_extension(db, 1);
				if(rc != SQLITE_OK) {
					std::cerr << "Can't enable extension loading: " << sqlite3_errmsg(db) << std::endl;
					sqlite3_close(db);
					return false;
				}
			}

			return true;
		}

		/*

		Query

		*/

		bool query(const std::string querySrc) {
			
			const char* query = querySrc.c_str();

			char* errMsg = nullptr;
			int rc = sqlite3_exec(db, query, 0, 0, &errMsg);
			if(rc != SQLITE_OK) {
				std::cerr << "@surfy:SQLite Error: " << errMsg << std::endl;
				sqlite3_free(errMsg);
				return false;
			}

			return true;
		}

		/*

		Find One

		*/

		json findOne(const char* query, const std::vector<std::string>& params = {}) {
			
			json result;

			sqlite3_stmt* stmt;
			int rc = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
			if (rc != SQLITE_OK) {
				sqlite3_finalize(stmt);
				result["status"] = false;
				std::string errorMessage = sqlite3_errmsg(db);
				result["msg"] = "Preparation failed: " + errorMessage;
				return result;
			}

			// Apply Params
			if (!params.empty()) {
				for (int i = 0; i < params.size(); ++i) {
					int column = i + 1;
					const std::string val = params[i];

					rc = sqlite3_bind_text(stmt, column, val.c_str(), -1, SQLITE_TRANSIENT);

					if (rc != SQLITE_OK) {
						sqlite3_finalize(stmt);
						result["status"] = false;
						std::string errorMessage = sqlite3_errmsg(db);
						result["msg"] = "Error binding text: " + errorMessage + '\n' + std::to_string(column) + ":" + val;
						return result;
					}

					
					rc = sqlite3_step(stmt);
					if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
			            std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << std::endl;
			            sqlite3_finalize(stmt);
			            sqlite3_close(db);
			            return false;
			        }
					sqlite3_reset(stmt);
				}
			}

			// Go

			rc = sqlite3_step(stmt);
			
			if (rc == SQLITE_DONE) {
				// Empty result set
				sqlite3_finalize(stmt);
				result["status"] = false;
				result["msg"] = "Query returned no results.";
				return result;
			}

			result = getData(stmt);

			sqlite3_finalize(stmt);
			return result;
		}

		/*

		Find
		@Return results

		*/

		// template<typename FindCallback>
		using Callback = std::function<void(const json&)>;
		json find(const std::string querySrc, const std::vector<std::string>& params = {}, Callback callback = nullptr) {			

			const char* query = querySrc.c_str();
			
			json result;

			sqlite3_stmt* stmt;
			int rc = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
			if (rc != SQLITE_OK) {
				sqlite3_finalize(stmt);
				result["status"] = false;
				std::string errorMessage = sqlite3_errmsg(db);
				result["msg"] = "Preparation failed: " + errorMessage;
				result["query"] = query;

				return result;
			}

			// Apply Params
			if (!params.empty()) {
				for (int i = 0; i < params.size(); ++i) {
					int column = i + 1;
					const std::string val = params[i];

					rc = sqlite3_bind_text(stmt, column, val.c_str(), -1, SQLITE_TRANSIENT);

					if (rc != SQLITE_OK) {
						sqlite3_finalize(stmt);
						result["status"] = false;
						std::string errorMessage = sqlite3_errmsg(db);
						result["msg"] = "Error binding text: " + errorMessage + '\n' + std::to_string(column) + ":" + val;
						return result;
					}

					
					rc = sqlite3_step(stmt);
					if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
			            std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << std::endl;
			            sqlite3_finalize(stmt);
			            sqlite3_close(db);
			            return false;
			        }
					sqlite3_reset(stmt);
				}
			}

			// Go

			result = {
				{ "status", true },
				{ "query", query }
			};

			std::vector<json> rows;

			rc = sqlite3_step(stmt);
			
			if (rc == SQLITE_DONE) {
				// Empty result set
				sqlite3_finalize(stmt);
				return result;
			}			

			while (rc == SQLITE_ROW) {

				json row = getData(stmt);
				if (callback) {
					callback(row);
				} else {
					rows.push_back(row);
				}

				// Next Row
				rc = sqlite3_step(stmt);
			}

			if (!callback) {
				result["rows"] = rows;
			}

			sqlite3_finalize(stmt);
			return result;
		}

		/*

		Process Row Data

		*/

		json getData(sqlite3_stmt* stmt) {
			
			json row;

			int numColumns = sqlite3_column_count(stmt);
			for (int i = 0; i < numColumns; ++i) {
				const char* columnName = sqlite3_column_name(stmt, i);
				int columnType = sqlite3_column_type(stmt, i);

				switch (columnType) {
					case SQLITE_INTEGER:
						row[columnName] = sqlite3_column_int(stmt, i);
						break;
					case SQLITE_FLOAT:
						row[columnName] = sqlite3_column_double(stmt, i);
						break;
					case SQLITE_TEXT:
						{
							const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
							json j = json::parse(text, nullptr, false);
							if (j.is_discarded()) {
								std::string str(text);
								row[columnName] = str;
							} else {
								row[columnName] = j;
							}
						}
						break;
					case SQLITE_BLOB:
						// BLOB data
						break;
					case SQLITE_NULL:
						row[columnName] = nullptr;
						break;
					default:
						break;
				}
			}

			return row;
		}
	};
}

#endif