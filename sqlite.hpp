#ifndef SURFY_SQLITE_HPP
#define SURFY_SQLITE_HPP

#include <sqlite3.h>
#include <iostream>
#include "json.hpp"

namespace surfy {
	using json = nlohmann::ordered_json;

	class SQLiteDB {
		sqlite3* db;
		// sqlite3_stmt* stmt;

	public:
		SQLiteDB() {
			
		}

		~SQLiteDB() {
			sqlite3_close(db);
		}

		bool connect(const char* dbName) {
			int rc = sqlite3_open(dbName, &db);
			if (rc) {
				std::cerr << "@surfy::SQLiteDB:: Can't open database: " << sqlite3_errmsg(db) << std::endl;
				sqlite3_close(db);
				return false;
			} else {
				std::cout << "@surfy::SQLiteDB:: Connected to " << dbName << std::endl;
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
					if (sqlite3_bind_text(stmt, column, val.c_str(), -1, SQLITE_STATIC) != SQLITE_OK) {
						sqlite3_finalize(stmt);
						result["status"] = false;
						std::string errorMessage = sqlite3_errmsg(db);
						result["msg"] = "Error binding text: " + errorMessage + "\n" + std::to_string(column) + ":" + val;
						return result;
					}
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

			if (rc != SQLITE_ROW) {
				sqlite3_finalize(stmt);
				result["status"] = false;
				result["msg"] = "Something's wrong. It shouldn't be like that.";
				return result;
			}

			result = getData(stmt);

			sqlite3_finalize(stmt);
			return result;
		}

		/*

		Find Sync

		*/

		json findSync(const char* query, const std::vector<std::string>& params = {}) {
			
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
					if (sqlite3_bind_text(stmt, column, val.c_str(), -1, SQLITE_STATIC) != SQLITE_OK) {
						sqlite3_finalize(stmt);
						result["status"] = false;
						std::string errorMessage = sqlite3_errmsg(db);
						result["msg"] = "Error binding text: " + errorMessage + "\n" + std::to_string(column) + ":" + val;
						return result;
					}
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

			if (rc != SQLITE_ROW) {
				sqlite3_finalize(stmt);
				result["status"] = false;
				result["msg"] = "Something's wrong. It shouldn't be like that.";
				return result;
			}

			result = json::array();

			while (rc == SQLITE_ROW) {

				json row = getData(stmt);
				result.push_back(row);

				// Next Row
				rc = sqlite3_step(stmt);
			}

			sqlite3_finalize(stmt);
			return result;
		}

		/*

		Find Async

		*/

		using Callback = std::function<void(const json&)>;
		json find(const char* query, Callback callback = nullptr, const std::vector<std::string>& params = {}) {
			
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
					if (sqlite3_bind_text(stmt, column, val.c_str(), -1, SQLITE_STATIC) != SQLITE_OK) {
						sqlite3_finalize(stmt);
						result["status"] = false;
						std::string errorMessage = sqlite3_errmsg(db);
						result["msg"] = "Error binding text: " + errorMessage + "\n" + std::to_string(column) + ":" + val;
						return result;
					}
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

			if (rc != SQLITE_ROW) {
				sqlite3_finalize(stmt);
				result["status"] = false;
				result["msg"] = "Something's wrong. It shouldn't be like that.";
				return result;
			}

			while (rc == SQLITE_ROW) {

				json row = getData(stmt);
				callback(row);

				// Next Row
				rc = sqlite3_step(stmt);
			}

			result["status"] = true;

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