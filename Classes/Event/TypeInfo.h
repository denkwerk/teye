/*
 * 		TypeInfo
 *
 *  	Created on: 14.07.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#ifndef TYPEINFO_
#define TYPEINFO_

#include <typeinfo>

using namespace std;

class TypeInfo {
	public:
		explicit TypeInfo(const type_info& info) : _typeInfo(info) {};

		bool operator < (const TypeInfo& rhs) const {
			return _typeInfo.before(rhs._typeInfo) != 0;
		}

	private:
		const type_info& _typeInfo;
};

#endif /* TYPEINFO_ */
