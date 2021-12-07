
/*
 *  UCF COP3330 Fall 2021 Assignment 6 Solution
 *  Copyright 2021 barhom alko
 */


//**************************************************CH7E1
#include "std_lib_facilities.h"
const char name = 'a';      //name token
const char let = 'L';       //declaration token
const std::string dec = "let"; //declaration keyword
const char func = 'F';      //function ticket
const char number = '8';    //number ticket
const char quit = 'q';      //quit ticket
const std::string declexit = "exit";
const char print = ';';     //print ticket

struct ticket
{
	char type;
	double value;
	std::string name;
	ticket(char ch, double val = 0.0) :type(ch), value(val), name("") {}
	ticket(char ch, std::string s) :type(ch), value(0.0), name(s) {}
};

//place to hold variable name value(s)
struct var
{
	std::string name;
	double value{};
};

bool operator==(const var& v, const std::string s) { return v.name == s; }

//package for all the variables
std::vector<var> names;

//returns the value of the var with the input name.
double get_value(const std::string s)
{
	auto vt_itr = std::find(names.cbegin(), names.cend(), s);
	if(vt_itr == names.cend()){
		std_lib_facilities::error("get: undefined name ", s);
	}
	return vt_itr->value;
}

//sets the variable of the named to a double value.
void set_value(const std::string s, const double d)
{
	auto vt_itr = std::find(names.begin(), names.end(), s);
	if(vt_itr == names.cend()){
		std_lib_facilities::error("set: undefined name ", s);
	}
	vt_itr->value = d;
}

//is a name already declared
bool is_declared(const std::string s)
{
	return names.cend() != std::find(names.cbegin(), names.cend(), s);
}

//adds a name value to a vector of variables
double define_name(const std::string s, const double d)
{
	if(is_declared(s)) std_lib_facilities::error(s, " declared twice");
	names.push_back(var{s, d});
	return d;
}

//place to hold valid tickets from cin
class ticket_stream
{
public:
	ticket_stream() :full(false), buffer('\0') {}

	//gets a ticket to place in the stream
	ticket get();

	//returns a ticket
	void putback(const ticket t);

	//discard characters up to and including the given input type token
	void ignore(const char c);

private:
	bool full;      //checks for ticket already in buffer
	ticket buffer;
};

ticket ticket_stream::get()
{
	ticket t{'\0'};
	if(full){
		full = false;
		t = buffer;
	} else{
		char ch;
		std::cin >> ch;
		switch(ch){
			case print:
			case quit:
			case '(':
			case ')':
			case '+':
			case '-':
			case '*':
			case '/':
			case ',':
			case '%':
				t.type = ch;
				break;
			case '=':
				if(this->buffer.type != let){
					const std::string s = (this->buffer.type == name ? this->buffer.name : std::to_string(this->buffer.value));
					std_lib_facilities::error(s, " cannot be reassigned");
				}
				t.type = ch;
				break;
			case '.':
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				{
					std::cin.putback(ch);
					double val;
					std::cin >> val;
					t.type = number;
					t.value = val;
					break;
				}
			default:
				if(isalpha(ch) || ch == '_'){
					std::string s;
					s += ch;
					while(std::cin.get(ch) && (isalpha(ch) || isdigit(ch) || ch == '_')) s += ch;
					std::cin.putback(ch);
					if(s == dec){
						t.type = let;
					} else if(ch == '('){
						t.type = func;
						t.name = s;
					} else if(s == declexit){
						t.type = quit;
					} else{
						t.type = name;
						t.name = s;
					}
				} else{
					std_lib_facilities::error("Bad ticket");
				}
		}
	}
	return t;
}

void ticket_stream::putback(const ticket t)
{
	if(full){
		std_lib_facilities::error("putback() into a full buffer");
	}
	buffer = t;
	full = true;
}

void ticket_stream::ignore(const char c)
{
	if(full && c == buffer.type){
		full = false;
	} else{
		full = false;
		std::cin.clear();
		std::cin.ignore(std::cin.rdbuf()->in_avail(), c);
	}
}

ticket_stream ts;

double expression();
double primary()
{
	ticket t = ts.get();
	double d{};
	switch(t.type){
		case '(':
			{
				d = expression();
				t = ts.get();
				if(t.type != ')'){
					std_lib_facilities::error("')' expected");
				}
				return d;
			}
		case '-':
			return -1 * primary();
		case '+':
			return primary();
		case number:
			return t.value;
		case name:
			return get_value(t.name);
		default:
			std_lib_facilities::error("primary expected");
	}
}

double term()
{
	double left = primary();
	while(true){
		ticket t = ts.get();
		switch(t.type){
			case '*':
				left *= primary();
				break;
			case '/':
				{
					double d = primary();
					if(d == 0) std_lib_facilities::error("division by zero");
					left /= d;
					break;
				}
			case '%':
				{
					double d = primary();
					if(d == 0){
						std_lib_facilities::error("division by zero");
					}
					left = fmod(left, d);
					break;
				}
			default:
				ts.putback(t);
				return left;
		}
	}
}

double expression()
{
	double left = term();
	while(true){
		ticket t = ts.get();
		switch(t.type){
			case '+':
				left += term();
				break;
			case '-':
				left -= term();
				break;
			default:
				ts.putback(t);
				return left;
		}
	}
}

double declaration()
{
	ticket t = ts.get();
	if(t.type != name){
		std_lib_facilities::error("name expected in declaration");
	}
	if(is_declared(t.name)){
		std_lib_facilities::error(t.name, " was declared twice");
	}
	ticket t2 = ts.get();
	if(t2.type != '='){
		std_lib_facilities::error("= missing the declaration of ", t.name);
	}
	double d = expression();
	define_name(t.name, d);
	return d;
}

double function(const std::string& s)
{
	ticket t = ts.get();
	double d{};
	std::vector<double> func_args;
	if(t.type != '('){
		std_lib_facilities::error("expected '(', improper function call");
	} else{
		do{
			t = ts.get();
			//checks if any argument is a function call
			if(t.type == func){
				t.type = number;
				//recursive call
				t.value = function(t.name);
				ts.putback(t);
			}
			//checks if any empty arguments
			if(t.type == ')'){
				break;
			} else{
				ts.putback(t);
			}
			// push valid function argument
			func_args.push_back(expression());
			t = ts.get();
			if(t.type == ')'){
				break;
			}
			if(t.type != ','){
				std_lib_facilities::error("expected ')', improper function call");
			}
		} while(t.type == ',');
	}

	if(s == "sqrt"){
		if(func_args.size() != 1) std_lib_facilities::error("sqrt() expected one argument");
		if(func_args[0] < 0) std_lib_facilities::error("sqrt() expected argument value greater than or equal to zero");
		d = sqrt(func_args[0]);
	} else if(s == "exp"){
		if(func_args.size() != 2) std_lib_facilities::error("exp() expected two arguments");
		d = func_args[0];
		auto multiplier = func_args[0];
		auto p = std_lib_facilities::narrow_cast<int>(func_args[1]);
		for(; p > 1; --p){
			d *= multiplier;
		}
	} else{
		std_lib_facilities::error("invalid function");
	}
	return d;
}

double statement()
{
	ticket t = ts.get();
	switch(t.type){
		case let:
			return declaration();
		case func:
			return function(t.name);
		default:
			ts.putback(t);
			return expression();
	}
}

void clean()
{
	ts.ignore(print);
}

void calculate()
{
	const std::string prompt = "> ";
	const std::string result = "= ";

	while(true) try{
		std::cout << prompt;
		ticket t = ts.get();
		while(t.type == print) t = ts.get();
		if(t.type == quit) return;
		ts.putback(t);
		std::cout << result << statement() << std::endl;
	}
	catch(std::runtime_error& e){
		std::cerr << e.what() << std::endl;
		clean();
	}
}

int main()
try{
	std::cin.sync_with_stdio(false);
	calculate();
	return 0;
}
catch(std::exception& e){
	std::cerr << "exception: " << e.what() << std::endl;
	std_lib_facilities::keep_window_open();
	return 1;
}
catch(...){
	std::cerr << "exception\n";
	std_lib_facilities::keep_window_open();
	return 2;
}





//**************************************************CH7E2

const char number = '8';
const char quit = 'q';
const std::string declexit = "exit";
const char print = ';';
const char name = 'a';
const char let = 'L';
const std::string dec = "let";
const char func = 'F';

struct ticket
{
	char type;
	double value;
	std::string name;
	ticket(char ch, double val = 0.0) :type(ch), value(val), name("") {}
	ticket(char ch, std::string s) :type(ch), value(0.0), name(s) {}
};

struct var
{
	std::string name;
	double value{};
};

bool operator==(const var& v, const std::string s) { return v.name == s; }

std::vector<var> names;
double get_value(const std::string s)
{
	auto vt_itr = std::find(names.cbegin(), names.cend(), s);
	if(vt_itr == names.cend()){
		std_lib_facilities::error("get: undefined name ", s);
	}
	return vt_itr->value;
}

void set_value(const std::string s, const double d)
{
	auto vt_itr = std::find(names.begin(), names.end(), s);
	if(vt_itr == names.cend()){
		std_lib_facilities::error("set: undefined name ", s);
	}
	vt_itr->value = d;
}

bool is_declared(const std::string s)
{
	return names.cend() != std::find(names.cbegin(), names.cend(), s);
}

double define_name(const std::string s, const double d)
{
	if(is_declared(s)) std_lib_facilities::error(s, " was declared twice");
	names.push_back(var{s, d});
	return d;
}

class ticket_stream
{
    public:
            ticket_stream() :full(false), buffer('\0')
            ticket get();

	void putback(const ticket t);

            void ignore(const char c);

    private:
        bool full;
        ticket buffer;
};

ticket ticket_stream::get()
{
	ticket t{'\0'};
	if(full){
		full = false;
		t = buffer;
	} else{
		char ch;
		std::cin >> ch;
		switch(ch){
			case print:
			case quit:
			case '(':
			case ')':
			case '+':
			case '-':
			case '*':
			case '/':
			case '%':
			case ',':
				t.type = ch;
				break;
			case '=':
				if(this->buffer.type != let) std_lib_facilities::error("incorrect reassignment");
				t.type = ch;
				break;
			case '.':
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				{
					std::cin.putback(ch);
					double val;
					std::cin >> val;
					t.type = number;
					t.value = val;
					break;
				}
			default:
				if(isalpha(ch) || ch == '_'){
					std::string s;
					s += ch;
					while(std::cin.get(ch) && (isalpha(ch) || isdigit(ch) || ch == '_')) s += ch;
					std::cin.putback(ch);
					if(s == dec){
						t.type = let;
					} else if(ch == '('){
						t.type = func;
						t.name = s;
					} else if(s == declexit){
						t.type = quit;
					} else{
						t.type = name;
						t.name = s;
					}
				} else{
					std_lib_facilities::error("Bad type");
				}
		}
	}
	return t;
}

void ticket_stream::putback(const ticket t)
{
	if(full){
		std_lib_facilities::error("putback() into a full buffer");
	}
	buffer = t;
	full = true;
}

void ticket_stream::ignore(const char c)
{
	if(full && c == buffer.type){
		full = false;
	} else{
		full = false;
		std::cin.clear();
		std::cin.ignore(std::cin.rdbuf()->in_avail(), c);
	}
}

ticket_stream ts;

double expression();
double function(const std::string& s);
double primary()
{
	ticket t = ts.get();
	double d{};
	switch(t.type){
		case '(':
			{
				d = expression();
				t = ts.get();
				if(t.type != ')'){
					std_lib_facilities::error("')' was expected");
				}
				break;
			}
		case '-':
			d = -1 * primary();
			break;
		case '+':
			d = primary();
			break;
		case number:
			d = t.value;
			break;
		case name:
			d = get_value(t.name);
			break;
		case func:
			d = function(t.name);
			break;
		default:
			std_lib_facilities::error("primary expected");
	}
	return d;
}

double term()
{
	double left = primary();
	while(true){
		ticket t = ts.get();
		switch(t.type){
			case '*':
				left *= primary();
				break;
			case '/':
				{
					double d = primary();
					if(d == 0) std_lib_facilities::error("division by zero");
					left /= d;
					break;
				}
			case '%':
				{
					double d = primary();
					if(d == 0){
						std_lib_facilities::error("division by zero");
					}
					left = fmod(left, d);
					break;
				}
			default:
				ts.putback(t);
				return left;
		}
	}
}

double expression()
{
	double left = term();
	while(true){
		ticket t = ts.get();
		switch(t.type){
			case '+':
				left += term();
				break;
			case '-':
				left -= term();
				break;
			default:
				ts.putback(t);
				return left;
		}
	}
}

double declaration()
{
	ticket t = ts.get();
	if(t.type != name){
		std_lib_facilities::error("name expected in declaration");
	}
	if(is_declared(t.name)){
		std_lib_facilities::error(t.name, " was declared twice");
	}
	ticket t2 = ts.get();
	if(t2.type != '='){
		std_lib_facilities::error(" = is missing in the declaration of ", t.name);
	}
	double d = expression();
	define_name(t.name, d);
	return d;
}

double func_available(const std::string& s, const std::vector<double>& args)
{
	double d{};
	if(s == "sqrt"){
		if(args.size() != 1) std_lib_facilities::error("sqrt() expected one argument");
		if(args[0] < 0) std_lib_facilities::error("sqrt() expected argument value greater than or equal to zero");
		d = sqrt(args[0]);
	} else if(s == "mul"){
		if(args.size() != 2) std_lib_facilities::error("exp() expected two arguments");
		d = args[0];
		auto exptiplier = args[0];
		auto p = std_lib_facilities::narrow_cast<int>(args[1]);
		for(; p > 1; --p){
			d *= exptiplier;
		}
	} else{
		std_lib_facilities::error("unknown function");
	}
	return d;
}

double function(const std::string& s)
{
	ticket t = ts.get();
	std::vector<double> func_args;
	if(t.type != '('){
		std_lib_facilities::error("expected '(', incoprrect function call");
	} else{
		do{
			t = ts.get();
			// check for arguments
			if(t.type != ')'){
				ts.putback(t);
				func_args.push_back(expression());
				t = ts.get();
				if(t.type != ',' && t.type != ')') std_lib_facilities::error("expected ')', incorrect function call");
			}
		} while(t.type != ')');
	}
	return func_available(s, func_args);
}

double statement()
{
	ticket t = ts.get();
	switch(t.type){
		case let:
			return declaration();
		case name:
			{
				char ch{};
				while(std::cin.get(ch) && ch == 32);
				if(ch != '='){
					std::cin.putback(ch);
				} else{
					auto d = expression();
					ticket t2 = ts.get();
					if(t2.type != print) std_lib_facilities::error("expected a print ticket ", print);
					set_value(t.name, d);
					std::cin.putback(print);
				}
			}
			[[fallthrough]] ;
		default:
			ts.putback(t);
			auto d = expression();
			t = ts.get();
			if(t.type == ','){
				std_lib_facilities::error("unexpected ticket ", t.type);
			}
			ts.putback(t);
			return d;
	}
}

void clean()
{
	ts.ignore(print);
}

void calculate()
{
	const std::string prompt = "> ";
	const std::string result = "= ";

	while(true) try{
		std::cout << prompt;
		ticket t = ts.get();
		while(t.type == print) t = ts.get();
		if(t.type == quit) return;
		ts.putback(t);
		std::cout << result << statement() << std::endl;
	}
	catch(std::runtime_error& e){
		std::cerr << e.what() << std::endl;
		clean();
	}
}

int main()
try{
	std::cin.sync_with_stdio(false);
	calculate();
	return 0;
}
catch(std::exception& e){
	std::cerr << "exception: " << e.what() << std::endl;
	std_lib_facilities::keep_window_open();
	return 1;
}
catch(...){
	std::cerr << "exception\n";
	std_lib_facilities::keep_window_open();
	return 2;
}





//**************************************************CH7E3
const char number = '8';
const char quit = 'q';
const std::string declexit = "exit";
const char print = ';';

const char name = 'a';
const char let = 'L';
const std::string dec = "let";
const char constant = 'C';
const std::string dec_const = "const";

const char func = 'F';

struct ticket
{
	char type;
	double value;
	std::string name;
	ticket(char ch, double val = 0.0) :type(ch), value(val), name("") {}
	ticket(char ch, std::string s) :type(ch), value(0.0), name(s) {}
};

struct var
{
	std::string name;
	double value{};
	bool is_const{false};
};

bool operator==(const var& v, const std::string s) { return v.name == s; }

std::vector<var> names;

double get_value(const std::string s)
{
	auto vt_itr = std::find(names.cbegin(), names.cend(), s);
	if(vt_itr == names.cend()) std_lib_facilities::error("get: undefined name ", s);
	return vt_itr->value;
}

void set_value(const std::string s, const double d)
{
	auto vt_itr = std::find(names.begin(), names.end(), s);
	if(vt_itr == names.cend()) std_lib_facilities::error("set: undefined name ", s);
	if(vt_itr->is_const) std_lib_facilities::error(s, ": is a constant ");
	vt_itr->value = d;
}

bool is_declared(const std::string s)
{
	return names.cend() != std::find(names.cbegin(), names.cend(), s);
}

double define_name(const std::string s, const double d, const bool set_const = false)
{
	if(is_declared(s)) std_lib_facilities::error(s, " declared twice");
	names.push_back(var{s, d, set_const});
	return d;
}

class ticket_stream
{
public:
	ticket_stream() :full(false), buffer('\0') {}
	ticket get();

	void putback(const ticket t);

	void ignore(const char c);

private:
	bool full;
	ticket buffer;
};

ticket ticket_stream::get()
{
	ticket t{'\0'};
	if(full){
		full = false;
		t = buffer;
	} else{
		char ch;
		std::cin >> ch;
		switch(ch){
			case print:
			case quit:
			case '(':
			case ')':
			case '+':
			case '-':
			case '*':
			case '/':
			case '%':
			case ',':
			case '=':
				t.type = ch;
				break;
			case '.':
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				{
					std::cin.putback(ch);
					double val;
					std::cin >> val;
					t.type = number;
					t.value = val;
					break;
				}
			default:
				if(isalpha(ch) || ch == '_'){
					std::string s;
					s += ch;
					while(std::cin.get(ch) && (isalpha(ch) || isdigit(ch) || ch == '_')) s += ch;
					std::cin.putback(ch);
					if(s == dec){
						t.type = let;
					} else if(s == dec_const){
						t.type = constant;
					} else if(ch == '('){
						t.type = func;
						t.name = s;
					} else if(s == declexit){
						t.type = quit;
					} else{
						t.type = name;
						t.name = s;
					}
				} else{
					std_lib_facilities::error("incorrect ticket");
				}
		}
	}
	return t;
}

void ticket_stream::putback(const ticket t)
{
	if(full){
		std_lib_facilities::error("putback() into a full buffer");
	}
	buffer = t;
	full = true;
}

void ticket_stream::ignore(const char c)
{
	if(full && c == buffer.type){
		full = false;
	} else{
		full = false;
		std::cin.clear();
		std::cin.ignore(std::cin.rdbuf()->in_avail(), c);
	}
}

ticket_stream ts;

double expression();
double function(const std::string& s);
double primary()
{
	ticket t = ts.get();
	double d{};
	switch(t.type){
		case '(':
			{
				d = expression();
				t = ts.get();
				if(t.type != ')'){
					std_lib_facilities::error("')' was expected");
				}
				break;
			}
		case '-':
			d = -1 * primary();
			break;
		case '+':
			d = primary();
			break;
		case number:
			d = t.value;
			break;
		case name:
			d = get_value(t.name);
			break;
		case func:
			d = function(t.name);
			break;
		default:
			std_lib_facilities::error("primary expected");
	}
	return d;
}

double term()
{
	double left = primary();
	while(true){
		ticket t = ts.get();
		switch(t.type){
			case '*':
				left *= primary();
				break;
			case '/':
				{
					double d = primary();
					if(d == 0) std_lib_facilities::error("division by zero");
					left /= d;
					break;
				}
			case '%':
				{
					double d = primary();
					if(d == 0){
						std_lib_facilities::error("division by zero");
					}
					left = fmod(left, d);
					break;
				}
			default:
				ts.putback(t);
				return left;
		}
	}
}

double expression()
{
	double left = term();
	while(true){
		ticket t = ts.get();
		switch(t.type){
			case '+':
				left += term();
				break;
			case '-':
				left -= term();
				break;
			case ',':
			default:
				ts.putback(t);
				return left;
		}
	}
}

double declaration(const bool is_const = false)
{
	ticket t = ts.get();
	if(t.type != name){
		std_lib_facilities::error("name was expected in declaration");
	}
	if(is_declared(t.name)){
		std_lib_facilities::error(t.name, " was declared twice");
	}
	ticket t2 = ts.get();
	if(t2.type != '='){
		std_lib_facilities::error("= is missing in declaration of ", t.name);
	}
	double d = expression();
	define_name(t.name, d, is_const);
	return d;
}

double statement()
{
	ticket t = ts.get();
	double d{};
	switch(t.type){
		case let:
			d = declaration();
			break;
		case constant:
			d = declaration(true);
			break;
		case name:
			{
				ticket t2 = ts.get();
				if(t2.type != '='){
					std::cin.putback(t2.type);
					ts.putback(t);
					d = expression();
				} else{
					d = expression();
					set_value(t.name, d);
				}
				break;
			}
		default:
			ts.putback(t);
			d = expression();
	}
	return d;
}

void clean()
{
	ts.ignore(print);
}

void calculate()
{
	const std::string prompt = "> ";
	const std::string result = "= ";

	while(true) try{
		std::cout << prompt;
		ticket t = ts.get();
		while(t.type == print) t = ts.get();
		if(t.type == quit) return;
		ts.putback(t);
		std::cout << result << statement() << std::endl;
	}
	catch(std::runtime_error& e){
		std::cerr << e.what() << std::endl;
		clean();
	}
}

int main()
try{
	std::cin.sync_with_stdio(false);
	calculate();
	return 0;
}
catch(std::exception& e){
	std::cerr << "exception: " << e.what() << std::endl;
	std_lib_facilities::keep_window_open();
	return 1;
}
catch(...){
	std::cerr << "exception\n\n";
	std_lib_facilities::keep_window_open();
	return 2;
}

double func_available(const std::string& s, const std::vector<double>& args)
{
	double d{};
	if(s == "sqrt"){
		if(args.size() != 1) std_lib_facilities::error("sqrt() expected one argument");
		if(args[0] < 0) std_lib_facilities::error("sqrt() expected argument value greater than or equal to zero");
		d = sqrt(args[0]);
	} else if(s == "exp"){
		if(args.size() != 2) std_lib_facilities::error("exp() expected two arguments");
		d = args[0];
		auto exptiplier = args[0];
		auto p = std_lib_facilities::narrow_cast<int>(args[1]);
		for(; p > 1; --p){
			d *= exptiplier;
		}
	} else{
		std_lib_facilities::error("incorrect function");
	}
	return d;
}

double function(const std::string& s)
{
	ticket t = ts.get();
	std::vector<double> func_args;
	if(t.type != '('){
		std_lib_facilities::error("expected '(', incorrect function call");
	} else{
		do{
			t = ts.get();
			//true check for arguments
			//false if no arguments
			if(t.type != ')'){
				ts.putback(t);
				func_args.push_back(expression());
				t = ts.get();
				if(t.type != ',' && t.type != ')') std_lib_facilities::error("expected ')', incorrect function call");
			}
		} while(t.type != ')');
	}
	return func_available(s, func_args);
}
