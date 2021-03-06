//#include "Dictionary.h"
//#include "DictionaryMap.h"

#include <roxlu/io/Dictionary.h>
#include <roxlu/io/DictionaryMap.h>

namespace roxlu {

//int Dictionary::num_maps = 0;


// Constructors
//------------------------------------------------------------------------------
Dictionary::Dictionary() {
	type = D_NULL;
	memset(&value, 0, sizeof(value));
}

Dictionary::~Dictionary() {
	reset();
}

Dictionary::Dictionary(IOBuffer& buffer){
	type = D_NULL;
	memset(&value, 0, sizeof(value));
	fromBinary(buffer);
}

Dictionary::Dictionary(const Dictionary& val) {
	copyFrom(val);
}

Dictionary::Dictionary(const bool& val) {
	type = D_BOOL;
	memset(&value, 0, sizeof(value));
	value.b = val;	
}

Dictionary::Dictionary(const rx_int8 &val) {
	type = D_INT8;
	memset(&value, 0, sizeof(value));
	value.i8 = val;
}

Dictionary::Dictionary(const rx_int16 &val) {
	type = D_INT16;
	memset(&value, 0, sizeof(value));
	value.i16 = val;
}

Dictionary::Dictionary(const rx_int32 &val) {
	type = D_INT32;
	memset(&value, 0, sizeof(value));
	value.i32 = val;
}

Dictionary::Dictionary(const rx_int64 &val) {
	type = D_INT64;
	memset(&value, 0, sizeof(value));
	value.i64 = val;
}

Dictionary::Dictionary(const rx_uint8 &val) {
	type = D_UINT8;
	memset(&value, 0, sizeof(value));
	value.ui8 = val;
}

Dictionary::Dictionary(const rx_uint16 &val) {
	type = D_UINT16;
	memset(&value, 0, sizeof(value));
	value.ui16 = val;
}

Dictionary::Dictionary(const rx_uint32 &val) {
	type = D_UINT32;
	memset(&value, 0, sizeof(value));
	value.ui32 = val;
}

Dictionary::Dictionary(const rx_uint64 &val) {
	type = D_UINT64;
	memset(&value, 0, sizeof(value));
	value.ui64 = val;
}

Dictionary::Dictionary(const double &val) {
	type = D_DOUBLE;
	memset(&value, 0, sizeof(value));
	value.d = val;
}

Dictionary::Dictionary(const string &val) {
	type = D_STRING;
	memset(&value, 0, sizeof(value));
	value.s = new string(val);
}

Dictionary::Dictionary(const char* val) {
	type = D_STRING;
	memset(&value, 0, sizeof(value));
	value.s = new string(val);
}

// Destructor
//------------------------------------------------------------------------------
void Dictionary::reset(bool isUndefined) {
	switch(type) {
		case D_BYTEARRAY:
		case D_STRING: {
			delete value.s;
			break;
		}
		case D_MAP: {
			delete value.m; 
			break;
		}
		case D_NULL: {
			break;
		}
		default: {
			//printf("@todo reset, check if all allocated types are supported!\n");
			break;
		}
	};
	if(isUndefined) {
		type = D_UNDEFINED;
	}
	else {
		type = D_NULL;
	}
		
	memset(&value, 0, sizeof(value));
}


// Operators
// -----------------------------------------------------------------------------
Dictionary& Dictionary::operator=(const Dictionary& other) {
	reset();
	copyFrom(other);
	return *this;
}

// we define this inlined function (so external usage will give unresolved)
void Dictionary::copyFrom(const Dictionary& other) {
	
	type = other.type;
	memset(&value, 0, sizeof(value));
	switch(other.type) {
		case D_BYTEARRAY:
		case D_STRING: {
			value.s = new string(*other.value.s);
			break;
		}
		case D_MAP: {	
			value.m = new DictionaryMap(*other.value.m);
			break;
		}
		default:{
			memcpy(&value, &other.value, sizeof(value));
			break;
		}
	};
}

	
Dictionary& Dictionary::operator=(const bool &val) {
	reset();
	type = D_BOOL;
	value.b = val;
	return *this;
}

Dictionary& Dictionary::operator=(const double &val) {
	reset();
	type = D_DOUBLE;
	value.d = val;
	return *this;
}

Dictionary& Dictionary::operator=(const string &val) {
	reset();
	type = D_STRING;
	value.s = new string(val);
	return *this;
}

Dictionary& Dictionary::operator=(const char* chars) {
	reset();
	type = D_STRING;
	value.s = new string(chars);
	return *this;
}


Dictionary& Dictionary::operator=(const rx_int8 &val) {
	reset();
	type = D_INT8;
	value.i8 = val;
	return *this;
}

Dictionary& Dictionary::operator=(const rx_int16 &val) {
	reset();
	type = D_INT16;
	value.i16 = val;
	return *this;
}

Dictionary& Dictionary::operator=(const rx_int32 &val) {
	reset();
	type = D_INT32;
	value.i32 = val;
	return *this;
}

Dictionary& Dictionary::operator=(const rx_int64 &val) {
	reset();
	type = D_INT64;
	value.i64 = val;
	return *this;
}

Dictionary& Dictionary::operator=(const rx_uint8 &val) {
	reset();
	type = D_UINT8;
	value.ui8 = val;
	return *this;
}

Dictionary& Dictionary::operator=(const rx_uint16 &val) {
	reset();
	type = D_UINT16;
	value.ui16 = val;
	return *this;
}

Dictionary& Dictionary::operator=(const rx_uint32 &val) {
	reset();
	type = D_UINT32;
	value.ui32 = val;
	return *this;
}

Dictionary& Dictionary::operator=(const rx_uint64 &val) {
	reset();
	type = D_UINT64;
	value.ui64 = val;
	return *this;
}

// @todo pass by ref?
bool Dictionary::operator==(Dictionary other) {
	return toString() == other.toString();
}

bool Dictionary::operator!=(Dictionary other) {
	return !operator==(other);
}

bool Dictionary::operator==(DictionaryType checkType) {
	if(checkType == D_NUMERIC) {
		return type == D_INT8 
				|| type == D_INT16
				|| type == D_INT32
				|| type == D_INT64
				|| type == D_UINT8
				|| type == D_UINT32
				|| type == D_UINT64
				|| type == D_DOUBLE;
				
	}	
	else  {
		return type == checkType;
	}
}

bool Dictionary::operator!=(DictionaryType checkType) {
	return !operator==(checkType);
}



Dictionary& Dictionary::operator[](const rx_uint32& key) {
	stringstream ss;
	ss << VAR_INDEX_VALUE << key;
	return operator[](ss.str());
}

Dictionary& Dictionary::operator[](const string& key) {
	if(	(type != D_MAP) && (type != D_NULL) && (type != D_UNDEFINED)) {
		cout << "operator[]: Key index Applied to incorrect dictionary" << endl;
	}
	if(type == D_NULL || type == D_UNDEFINED) {
		type = D_MAP;
		//num_maps++; // tmp
		value.m = new DictionaryMap;
	}
	
	// add a new entry when it doesnt exist.
	if(!IN_MAP(value.m->children, key)) {
		value.m->children[key] = Dictionary();
	}
	
	// return reference to child.
	return value.m->children[key];
}

Dictionary& Dictionary::operator[](const char* key) {
	return operator[](string(key));
}

Dictionary& Dictionary::operator[](Dictionary& key) {
	stringstream ss;
	switch(key.type) {
		case D_BOOL:
		case D_INT8:
		case D_INT16:
		case D_INT32:
		case D_INT64:
		case D_UINT8:
		case D_UINT16:
		case D_UINT32:
		case D_UINT64:
		case D_DOUBLE: {
			ss << VAR_INDEX_VALUE << (string)key;
			break;
		}
		case D_STRING: {
			ss << *key.value.s;
			break;
		}
		case D_NULL:
		case D_UNDEFINED:
		case D_MAP: 
		default: {
			printf("Dictionary type cannot be used as index.\n");
			break;
		}
	}
	return operator[](ss.str());
}

// type casting
//------------------------------------------------------------------------------
#define DICT_OPERATOR(ctype) \
	Dictionary::operator ctype() { \
		switch(type) { \
			case D_UNDEFINED: \
			case D_NULL: { \
				return 0; \
			} \
			case D_BOOL: { \
				return (ctype) value.b; \
			} \
			case D_INT8: { \
				return (ctype) value.i8; \
			} \
			case D_INT16: { \
				return (ctype) value.i16; \
			} \
			case D_INT32: { \
				return (ctype) value.i32; \
			} \
			case D_INT64: { \
				return (ctype) value.i64; \
			} \
			case D_UINT8: { \
				return (ctype) value.ui8; \
			} \
			case D_UINT16: { \
				return (ctype) value.ui16; \
			} \
			case D_UINT32: { \
				return (ctype) value.ui32; \
			} \
			case D_UINT64: { \
				return (ctype) value.ui64; \
			} \
			case D_DOUBLE: { \
				return (ctype) value.d; \
			} \
			case D_MAP: \
			case D_STRING: \
			default: { \
				printf("Dictionary cast failed.\n"); \
				return 0; \
			} \
		} \
	}  
			
//DICT_OPERATOR(bool);	
DICT_OPERATOR(double);
DICT_OPERATOR(rx_int8);	
DICT_OPERATOR(rx_int16);			
DICT_OPERATOR(rx_int32);	
DICT_OPERATOR(rx_int64);	
DICT_OPERATOR(rx_uint8);	
DICT_OPERATOR(rx_uint16);	
DICT_OPERATOR(rx_uint32);	
DICT_OPERATOR(rx_uint64);	

Dictionary::operator bool() {
	switch(type) {
		case D_NULL:
		case D_UNDEFINED: {
			return false;
		}
		
		case D_BOOL: {
			return value.b;
		}
		
		case D_INT8:
		case D_INT16:
		case D_INT32:
		case D_INT64:
		case D_UINT8:
		case D_UINT16:
		case D_UINT32:
		case D_UINT64:
		case D_DOUBLE: {
			bool result = false;
			result |= (value.i8 != 0);
			result |= (value.i16 != 0);
			result |= (value.i32 != 0);
			result |= (value.i64 != 0);
			result |= (value.ui8 != 0);
			result |= (value.ui16 != 0);
			result |= (value.ui32 != 0);
			result |= (value.ui64 != 0);
			result |= (value.d != 0); //!not in variant!
			return result;
		}
		
		case D_STRING:
		case D_MAP:
		default: {
			return false;
		}
	};
}

bool Dictionary::isNumeric() {
	switch(type) {
		case D_INT8:
		case D_INT16:
		case D_INT32:
		case D_INT64:
		case D_UINT8:
		case D_UINT16:
		case D_UINT32:
		case D_UINT64:
		case D_DOUBLE: {
			return true;
		}
		default: {
			return false;
		}
	}
	//return false;
}

bool Dictionary::isBoolean() {
	return type == D_BOOL;
}

Dictionary::operator string() {
	switch(type) {
		case D_BOOL: {
			return (value.m) ? "true" : "false";
		}
		
		case D_INT8: {
			stringstream ss;
			ss << value.i8;
			return ss.str();
		}
		
		case D_INT16: {
			stringstream ss;
			ss << value.i16;
			return ss.str();
		}
		
		case D_INT32: {
			stringstream ss;
			ss << value.i32;
			return ss.str();
		}
		
		case D_INT64: {
			stringstream ss;
			ss << value.i64;
			return ss.str();
		};
		
		case D_UINT8: {
			stringstream ss;
			ss << value.ui8;
			return ss.str();
		}
		
		case D_UINT16: {
			stringstream ss;
			ss << value.ui16;
			return ss.str();
		}
		
		case D_UINT32: {
			stringstream ss;
			ss << value.ui32;
			return ss.str();
		}
		
		case D_UINT64: {
			stringstream ss;
			ss << value.ui64;
			return ss.str();
		};
		
		case D_DOUBLE: {
			stringstream ss;
			ss << value.d;
			return ss.str();
		}
		
		case D_BYTEARRAY:
		case D_STRING: {
			return *value.s;
		}
		
		case D_UNDEFINED:
		case D_NULL:
		case D_MAP: 
		default: {
			//printf("Cannot cast to string, type: %d", type);
			return "";
		}
	};
	return "";
}


// Retrieve as forced type
//------------------------------------------------------------------------------
rx_uint8 Dictionary::getAsUInt8() {
	rx_uint8 val;
	string as_str =(string)(*this);
	istringstream is(as_str);
	is >> val;
	return val;
}

rx_uint16 Dictionary::getAsUInt16() {
	rx_uint16 val;
	string as_str =(string)(*this);
	istringstream is(as_str);
	is >> val;
	return val;
}

rx_uint32 Dictionary::getAsUInt32() {
	rx_uint32 val;
	string as_str =(string)(*this);
	istringstream is(as_str);
	is >> val;
	return val;
}

rx_uint64 Dictionary::getAsUint64() {
	rx_uint64 val;
	string as_str =(string)(*this);
	istringstream is(as_str);
	is >> val;
	return val;
}

rx_int8 Dictionary::getAsInt8() {
	rx_int8 val;
	string as_str =(string)(*this);
	istringstream is(as_str);
	is >> val;
	return val;
}

rx_int16 Dictionary::getAsInt16() {
	rx_int16 val;
	string as_str =(string)(*this);
	istringstream is(as_str);
	is >> val;
	return val;
}

rx_int32  Dictionary::getAsInt32() {
	rx_int32 val;
	string as_str = (string)(*this);
	istringstream is(as_str);
	is >> val;
	return val;
}

rx_int64  Dictionary::getAsInt64() {
	rx_int64 val;
	string as_str =(string)(*this);
	istringstream is(as_str);
	is >> val;
	return val;
}

// Serialization
//------------------------------------------------------------------------------
string Dictionary::toJSON() {
	string result = "";
	if(jsonSerialize(result)) {
		return result;
	}
	return "";
}

string Dictionary::toXML() {
	return toString();
}

bool Dictionary::jsonSerialize(string &result) {
	switch(type) {
		case D_NULL: {
			result += "null";
			break;
		}
		case D_BOOL: {
			result += ((bool)(*this)) ? "true" : "false";
			break;
		}
		case D_UINT8:
		case D_UINT16:
		case D_UINT32:
		case D_UINT64: 
		case D_INT8:
		case D_INT16:
		case D_INT32:
		case D_INT64: {
			rx_int64 val = (rx_int64)(*this);
			stringstream ss;
			ss << val;
			result += ss.str();
			break;
		}

		case D_DOUBLE: {
			stringstream ss;
			ss << setprecision(4) << value.d;
			result += ss.str();
			break;
		}
		
		case D_STRING: {
			string str = (string)(*this);
//			escapeJSON(str);
			jsonEscape(str);
			result += str;
			break;
		}
		case D_BYTEARRAY: {
			result += "\"bytearray not supported\"";
			break;
		}
		case D_MAP: {
			result += (isArray()) ? "[" : "{";

			// serialze all children.
			map<string, Dictionary>::iterator it = value.m->children.begin();
			while(it != value.m->children.end()) {
				// get key.
				string map_key = it->first;
				if(!isArray()) {
					//escapeJSON(map_key);
					jsonEscape(map_key);
					result += map_key +":";
				}	
				// get values.
				Dictionary map_val = it->second;
				if(!map_val.jsonSerialize(result)) {
					printf("Error while converting to json for key: %s\n", map_key.c_str());
					return false;
				}
				result += ",";
				++it;
			}	
			
			// close
			if(value.m->children.size() > 0) {
				result[result.size()-1] = (isArray()) ? ']' :'}';
			}
			else {
				result += (isArray()) ? "]" : "}";
			}
			break;
		}
		default: {
			printf("Unhandled type which we cannot serialize");
			break;
		};
	};
	return true;
}



// Create a binary buffer from this dictionary
//------------------------------------------------------------------------------
bool Dictionary::toBinary(IOBuffer& buffer) {
	buffer.storeByte(type);
	switch(type) {
		case D_NULL:
		case D_UNDEFINED: {
			return true;
			break;
		}
		case D_BOOL: {
			buffer.storeByte((rx_uint8)value.b);
			return true;
			break;
		}
		case D_INT8: {
			buffer.storeByte(value.i8);
			return true;
			break;
		}
		case D_INT16: {
			buffer.storeUI16BE(value.i16);
			return true;
			break;
		}
		case D_INT32: {
			buffer.storeUI32BE(value.i32);
			return true;
			break;
		}
		case D_INT64: {
			buffer.storeUI64BE(value.i64);
			return true;
			break;
		}
		case D_UINT8: {
			buffer.storeByte(value.ui8);
			return true;
			break;
		}
		case D_UINT16: {
			buffer.storeUI16BE(value.ui16);
			return true;
			break;
		}
		case D_UINT32: {
			buffer.storeUI32BE(value.ui32);
			return true;
			break;
		}
		case D_UINT64: {
			buffer.storeUI64BE(value.ui64);
			return true;
			break;
		}
		case D_DOUBLE: {
			buffer.storeDoubleBE(value.d);
			return true;
			break;
		}
		case D_STRING: {
			buffer.storeStringWithSizeBE(*value.s);
			return true; 
			break;
		}
		case D_MAP: {
			bool is_array = isArray();
			buffer.storeByte(is_array);
			
			rx_uint32 length = getMapSize();
			buffer.storeUI32BE(length);
			
			Dictionary::iterator it = begin();
			while(it != end()) {
				buffer.storeStringWithSizeBE(it->first);
				IOBuffer recursed_buffer;
				if(!it->second.toBinary(recursed_buffer)) {
					printf("Dictionary.toBinary(): unable to serialize dictionary\n");
					return false;
				}
				else {
					buffer.storeBuffer(recursed_buffer);
				}
				++it;
			}
			return true;
			break;
		}
		
		default: {
			printf("Dictionary.toBinary(): type not handled: %u\n", type);
			break;
		}
	};
	return true;
}

bool Dictionary::fromBinary(IOBuffer& buffer) {
	return binaryDeserialize(buffer, *this);
}

bool Dictionary::binaryDeserialize(IOBuffer& buffer, Dictionary& result) {
	rx_uint8 stored_type = buffer.consumeByte();
	switch(stored_type) {
		case D_NULL: {
			result.reset(false);
			return true;
		}
		case D_UNDEFINED: {
			result.reset(true);
			return true;
		}
		case D_BOOL: {
			result = (bool)buffer.consumeByte();
			return true;
		}
		case D_INT8: {
			result = (rx_int8)buffer.consumeByte();
			return true;
		}
		case D_INT16: {
			result = (rx_int16)buffer.consumeI16BE();
			return true;
		}
		case D_INT32: {
			result = (rx_int32)buffer.consumeI32BE();
			return true;
		}
		case D_UINT8: {
			result = (rx_uint8)buffer.consumeByte();
			return true;
		}
		case D_UINT16: {
			result = (rx_uint16)buffer.consumeUI16BE();
			return true;
		}
		case D_UINT32: {
			result = (rx_uint32)buffer.consumeUI32BE();
			return true;
		}
		case D_UINT64: {
			result = (rx_uint64)buffer.consumeUI64BE();
			return true;
		}
		case D_DOUBLE: {
			result = (double)buffer.consumeDoubleBE();
			return true;
		}
		case D_STRING: {
			result = (string)buffer.consumeStringWithSizeBE();
			return true;
		}
		case D_MAP: {
			bool is_array = (bool)buffer.consumeByte();
			result.isArray(is_array);
			rx_uint32 length = buffer.consumeUI32BE();
			for(rx_uint32 i = 0; i < length; i++) {
				string key = buffer.consumeStringWithSizeBE();
				if(!Dictionary::binaryDeserialize(buffer, result[key])) {
					printf("Dictionary.fromBinary: cannot deserialize map for key: %s\n", key.c_str());
					return false;
				}
			}
			return true;
		}
		default: {
			printf("Dictionary.fromBinary, unhandled type: %02X\n", stored_type);
			return false;
		}
		
	};
	return true;
}

// String functions.
//------------------------------------------------------------------------------
void Dictionary::jsonEscape(string& v) {
	replaceString(v, 	"\\",	"\\\\");
	replaceString(v, 	"/", 	"\\/"); 
	replaceString(v, 	"\"", 	"\\\""); 
	replaceString(v, 	"\b", 	"\\b"); 
	replaceString(v, 	"\f", 	"\\f"); 
	replaceString(v, 	"\n", 	"\\n"); 
	replaceString(v, 	"\r", 	"\\r"); 
	replaceString(v, 	"\t", 	"\\t"); 
	v = "\"" +v +"\"";
}

void Dictionary::jsonUnEscape(string& v) {
	replaceString(v,	"\\/", 	"/");
	replaceString(v,	"\\\"", "\"");
	replaceString(v,	"\\b",	"\b");
	replaceString(v,	"\\f", 	"\f");
	replaceString(v,	"\\n", 	"\n");
	replaceString(v,	"\\r", 	"\r");
	replaceString(v,	"\\t", 	"\t");
	replaceString(v,	"\\\\", "\\");
}


void Dictionary::replaceString(string& target, string search, string replacement) {
	if (search == replacement) {
		return;
	}
	if (search == "") {
		return;
	}
	string::size_type i = string::npos;
	string::size_type last_pos = 0;
	while ((i = target.find(search, last_pos)) != string::npos) {
		target.replace(i, search.length(), replacement);
		last_pos = i + replacement.length();
	}
}

string Dictionary::stringToLower(string value) {
	string result = "";
	for (string::size_type i = 0; i < value.length(); i++) {
			result += tolower(value[i]);
	}
	return result;
}

bool Dictionary::jsonReadWhiteSpace(string& raw, rx_uint32& start) {
	for(; start < raw.length(); ++start){ 
		if((raw[start] != ' '
			&& raw[start] != '\t'
			&& raw[start] != '\r'
			&& raw[start] != '\n')
		)
		{
			break;
		}
	}
	return true;
}

bool Dictionary::jsonReadDelimiter(string& raw, rx_uint32& start, char& c) {
	if(!jsonReadWhiteSpace(raw, start)) {
		printf("jsonReadDelimiter: invalid object\n");
		return false;
	}
	if(raw.size() - start < 1) {
		printf("jsonReadDelimiter: invalid json delimiter\n");
		return false;
	}
	c = raw[start];
	start++;
	return jsonReadWhiteSpace(raw, start);
	return true;
}

bool Dictionary::jsonReadString(string& raw, Dictionary& result, rx_uint32& start) {
	if((raw.size() - start) < 2) {
		printf("jsonReadString: invalid string");
		return false;
	}
	if(raw[start] != '\"') {
		printf("jsonReadString: error invalid string: %u\n", start);
		return false;
	}
	start++;
	string::size_type pos = start;
	while(true) {
		pos = raw.find('\"', pos);
		if(pos == string::npos) {
			printf("jsonReadString: invalid json string.\n");
			return false;
		}
		if(raw[pos-1] == '\\') {
			pos++;
		}
		else {
			string value = raw.substr(start, pos-start);
			jsonUnEscape(value);
			result = value;
			start = pos + 1;
			//printf("DEBUG: jsonReadString = %s\n", ((string)(result)).c_str());
			return true;
		}
	}
	//printf("DEBUG: jsonReadString .. here??\n");
	return false;
}

bool Dictionary::jsonReadNumber(string& raw, Dictionary& result, rx_uint32& start) {
	string str = "";
	for(; start < raw.length(); ++start) {
		if((raw[start] < '0') || (raw[start] > '9')) {
			break;
		}
		str += raw[start];
	}
	if(str == "") {
		printf("jsonReadNumber: invalid json number\n");
		return false;
	}
// @todo make this work on windows...
//	result = (rx_int64)atoll(str.c_str());
	return true;
}

bool Dictionary::jsonReadObject(string& raw, Dictionary& result, rx_uint32& start) {
	result.reset();
	result.isArray(false);
	if((raw.size() - start) < 2) {
		printf("jsonReadObject: invalid json object.\n");
		return false;
	}
	if(raw[start] != '{') {
		printf("jsonReadObject: invalid start of json object.\n");
		return false;
	}
	start++;
	char c;
	while(start < raw.length()) {
		if(raw[start] == '}') {
			start++;
			return true;
		}
		
		Dictionary key;
		if(!Dictionary::jsonDeserialize(raw,key,start)) {
			printf("jsonReadObject: invalid json object (3).\n");
			return false;
		}
		
		if(!jsonReadDelimiter(raw, start, c)) {
			printf("jsonReadObject: invald json object (4).\n");
			return false;
		}
		
		if(c != ':') {
			printf("jsonReadObject: invalid json object (5).\n");
			return false;
		}
		
		Dictionary value;
		if(!Dictionary::jsonDeserialize(raw, value, start)) {
			printf("jsonReadObject: invalid json object (6).\n");
			return false;
		}
		
		result[key] = value;
		
		if(!jsonReadDelimiter(raw, start, c)) {
			printf("jsonReadObject: invalid json object (7).\n");
			return false;
		}
		if (c == '}') {
			return true;
		}
		else if (c == ',') {
			continue;		
		}
		else {
			printf("jsonReadObject: invalid json object (8).\n");
			return false;
		}
		
	}
	
	return false;
}

bool Dictionary::jsonReadArray(string& raw, Dictionary& result, rx_uint32& start) {
	result.reset();
	result.isArray(true);
	if((raw.size() - start) < 2) {
		printf("jsonReadArray: invalid json array.\n");
		return false;
	}
	if(raw[start] != '[') {
		printf("jsonReadArray: invalid json array (start).\n");
		return false;
	}
	start++;
	char c;
	while(start < raw.length()) {
		if(raw[start] == ']') {
			start++;
			return true;
		}
		Dictionary value;
		if(!Dictionary::jsonDeserialize(raw, value, start)) {
			printf("jsonReadArray: invalid json array (3)\n");
			return false;
		}
		result.pushToArray(value);
		
		if(!jsonReadDelimiter(raw, start, c)) {
			printf("jsonReadArray: invalid json array (4).\n");
			return false;
		}
		
		if(c == ']') {
			return true;
		}
		else if(c == ',') {
			continue;
		}
		else {
			printf("jsonReadArray: invalid json array (5).\n");
			return false;
		}
	}
	
	return false;
}

bool Dictionary::jsonReadBool(string& raw, Dictionary& result, rx_uint32& start, string wanted) {
	if((raw.size() - start) < wanted.size()) {
		printf("jsonReadBool: invald json bool\n");
		return false;
	}
	string temp = stringToLower(raw.substr(start, wanted.size()));
	if(temp != wanted) {
		printf("jsonReadBool: invalid bool\n");
		return false;	
	}
	start += wanted.size();
	result = (bool)(wanted == "true");
	return true;
}

bool Dictionary::jsonReadNull(string& raw, Dictionary& result, rx_uint32& start) {
	if((raw.size() - start) < 4) {
		printf("jsonReadNull: invalid json null.\n");
		return false;
	}
	string temp = stringToLower(raw.substr(start, 4));
	if(temp != "null") {
		printf("jsonReadNull: invalid json null (2).\n");
		return false;
	}
	start += 4;
	result.reset();
	return true;
}

bool Dictionary::fromJSON(string json) {
	rx_uint32 start = 0;
	return jsonDeserialize(json, *this, start);
}

bool Dictionary::jsonDeserialize(string& raw, Dictionary& result, rx_uint32& start) {
	result.reset();
	if(start >= raw.size()) {
		return false;
	}
	if(!jsonReadWhiteSpace(raw, start)) {
		printf("Invalid JSON string\n");
		return false;
	}
	switch(raw[start]) {
		case '\"': {
			return jsonReadString(raw, result, start);
		}
		case '-':
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9': {
			return jsonReadNumber(raw, result, start);
		}
		case '{': {
			return jsonReadObject(raw, result, start);
		}
		case '[': {
			return jsonReadArray(raw, result, start);
		}
		case 't':
		case 'T': {
			return jsonReadBool(raw, result, start, "true");
		}
		case 'f':
		case 'F': {
			return jsonReadBool(raw, result, start, "false");
		}
		case 'n':
		case 'N': {
			return jsonReadNull(raw, result, start);
		}
		default: {
			result.reset();
			return false;
		}
		
	}
	return true;
}


string Dictionary::toString(string name, rx_uint32 indent) {
	string result = "";
	string str_indent = string(indent*4, ' ');
	switch(type) {
		case D_NULL: {
			result += str_indent +"<NULL name=\"" +name +"\"></NULL>";
			break;
		}
		case D_UNDEFINED: {
			result += str_indent +"<UNDEFINED name=\"" +name +"\"></UNDEFINED>";
			break;
		}
		case D_BOOL: {
			result += str_indent +"<BOOL name=\"" +name +"\">" 
										+((value.b) ? "true":"false") 
								+"</BOOL>";
			break;
		}
		case D_INT8: {
			stringstream ss;
			ss << value.i8;
			result += str_indent +"<INT8 name=\"" +name +"\">" +ss.str() +"</INT8>";
			break;
		}
		case D_INT16: {
			stringstream ss;
			ss << value.i16;
			result += str_indent +"<INT16 name=\"" +name +"\">" +ss.str() +"</INT16>";
			break;
		}
		case D_INT32: {
			stringstream ss;
			ss << value.i32;
			result += str_indent +"<INT32 name=\"" +name +"\">" +ss.str() +"</INT32>";
			break;
		}
		case D_INT64: {
			stringstream ss;
			ss << value.i64;
			result += str_indent +"<INT64 name=\"" +name +"\">" +ss.str() +"</INT64>";
			break;
		}
		case D_UINT8: {
			stringstream ss;
			ss << value.ui8;
			result += str_indent +"<UINT8 name=\"" +name +"\">" +ss.str() +"</UINT8>";
			break;
		}
		case D_UINT16: {
			stringstream ss;
			ss << value.ui16;
			result += str_indent +"<UINT16 name=\"" +name +"\">" +ss.str() +"</UINT16>";
			break;
		}
		case D_UINT32: {
			stringstream ss;
			ss << value.ui32;
			result += str_indent +"<UINT32 name=\"" +name +"\">" +ss.str() +"</UINT32>";
			break;
		}
		case D_UINT64: {
			stringstream ss;
			ss << value.ui64;
			result += str_indent +"<UINT64 name=\"" +name +"\">" +ss.str() +"</UINT64>";
			break;
		}
		case D_DOUBLE: {
			stringstream ss;
			ss << value.d;
			result += str_indent +"<DOUBLE name=\"" +name +"\">" +ss.str() +"</DOUBLE>";
			break;
		}
		case D_STRING: {
			result += str_indent +"<STR name=\"" +name +"\">" +(*value.s) +"</STR>";
			break;
		}
		case D_BYTEARRAY: {
			stringstream ss;
			ss << (*value.s).size();
			result += str_indent +"<BYTEARRAY name=\"" +name +"\">" +ss.str() +" bytes</BYTEARRAY>";
			break;
		}
		case D_MAP: {
			map<string, Dictionary>::iterator it = value.m->children.begin();
			result += str_indent +"<MAP name=\"" +name +"\" is_array=\"" +((value.m->is_array) ? "true":"false") +"\">\n";
			while(it != value.m->children.end()) {
				result += it->second.toString((string)it->first, indent+1) +"\n";
				++it;
			}
			result += str_indent +"</MAP>";
			break;
		}
		default: {
			printf("dictionary: cannot convert unhandled type toString()\n");
			break;
		}
		
	}
	return result;
}

// Array related 
// -----------------------------------------------------------------------------
bool Dictionary::isArray() {
	if(type == D_MAP) {
		return value.m->is_array;
	}
	return false;
}

void Dictionary::isArray(bool makeArray) {
	if(type == D_NULL) {
		
		type = D_MAP;
		value.m = new DictionaryMap;
		//num_maps++;
		//cout << "Map created (2):" << num_maps << " we are:" << this<< endl;
	}
	if(type == D_MAP) {
		value.m->is_array = makeArray;
	}
}

void Dictionary::isByteArray(bool makeByteArray) {	
	if(makeByteArray) {
		if(type == D_STRING) {
			type = D_BYTEARRAY;
		}
	}
	else {
		if(type == D_BYTEARRAY) {
			type = D_STRING;
		}
	}
}

bool Dictionary::isByteArray() {
	return type == D_BYTEARRAY;
}

rx_uint32 Dictionary::getMapSize() {
	if(type == D_NULL || type == D_UNDEFINED) {
		return 0;
	}
	if(type != D_MAP) {
		printf("cannot get map size, we are not a map\n");
		return 0;
	}
	return (rx_uint32) value.m->children.size();

}

// size of 'none' name=value pairs (so total number of alements which 
// are indexed by a rx_uint32. Internally these are stored using a 
// special marker key. See the  "VAR_INDEX_VALUE" define. 
rx_uint32 Dictionary::getMapDenseSize() {
	if(type == D_NULL || type == D_UNDEFINED) {
		return 0;
	}
	if(type != D_MAP) {
		printf("cannot get map dense size, we are not a map\n");
		return 0;
	}
	rx_uint32 count = 0;
	rx_uint32 s = getMapSize();
	for(count = 0; count < s; ++count) {
		stringstream ss;
		ss << VAR_INDEX_VALUE << count;
		string dx_str = ss.str(); 
		if(!IN_MAP(value.m->children, dx_str)) {
			break;
		}
	}
	return count;
}

void Dictionary::pushToArray(Dictionary dict) {
	if(type != D_NULL && type != D_MAP) {
		printf("cannot push to array we are not a map\n");
		return;
	}
	isArray(true);
	(*this)[(rx_uint32)this->getMapDenseSize()] = dict;
}

void Dictionary::removeKey(const string& key) {
	if(type != D_MAP) {
		printf("cannot removeKey(), we are not a map.\n");
		return;
	}
	value.m->children.erase(key);
}

void Dictionary::removeAt(const rx_uint32 index) {
	if(type != D_MAP) {
		printf("cannot removeAt(), we are not a map.\n");
		return;
	}
	stringstream ss;
	ss << VAR_INDEX_VALUE << index;
	value.m->children.erase(ss.str());
}

bool Dictionary::hasKey(const string key) {
	if(type != D_MAP) {
		return false;
	}
	return IN_MAP(value.m->children, key);
}


// Iterate over values.
//------------------------------------------------------------------------------
map<string, Dictionary>::iterator Dictionary::begin() {
	if(type != D_MAP) {
		map<string,Dictionary> tmp;
		return tmp.begin();
	}
	return value.m->children.begin();
}

map<string, Dictionary>::iterator Dictionary::end() {
	if(type != D_MAP) {
		map<string,Dictionary> tmp;
		return tmp.end();
	}
	return value.m->children.end();
}

	
} // namespace roxlu