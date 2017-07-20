#pragma once

#include <sstream>

namespace yw {
	namespace sqlite {

		class TableRow {
		public:
			virtual std::string elements() const { return std::string(""); };
			virtual std::string str() const {
				std::stringstream ss;
				ss << typeid(*this).name() << "{" << elements() << "}";
				return ss.str();
			}
		};

		inline std::wstringstream& operator<<(std::wstringstream& wss, const yw::sqlite::TableRow& row) {
			wss << row.str().c_str();
			return wss;
		}

		inline bool operator==(const yw::sqlite::TableRow& lhs, const yw::sqlite::TableRow& rhs) {
			return lhs.str() == rhs.str();
		};
	}
}
