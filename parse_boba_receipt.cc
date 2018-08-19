#include <cmath>
#include <iomanip>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

class ReceiptLine {
public:
	enum Type { Total, Name, Item, Extra };
	ReceiptLine(const std::string &text) {
		auto itotal = std::sregex_iterator(text.begin(), text.end(), totalr);
		if (itotal != endri) {
			type_ = Total;
			price_ = std::strtod(itotal->str().c_str() + 1, nullptr);
			return;
		}
		auto iprice = std::sregex_iterator(text.begin(), text.end(), pricer);
		if (iprice != endri) {
			type_ = Item;
			price_ = std::strtod(iprice->str().c_str() + 1, nullptr);
			return;
		}
		auto iname = std::sregex_iterator(text.begin(), text.end(), namer);
		if (iname != endri) {
			type_ = Name;
			name_ = iname->str();
			name_ = name_.substr(name_.find(":") + 2);
			return;
		}
		type_ = Extra;
	}

	Type type() const { return type_; }
	const std::string &name() const { return name_; }
	double price() const { return price_; }
private:
	Type type_;
	std::string name_;
	double price_;

	const static std::regex pricer;
	const static std::regex totalr;
	const static std::regex namer;
	const static std::sregex_iterator endri;
};

class Tab {
public:
	Tab(const std::string &name) : name_(name), total_(0) {}
	void add(double v) { total_ += v; }
	const std::string name() const { return name_; }
	double total() const { return total_; }
	void print(std::ostream& os, double multiplier) const {
		if (total_ == 0) return;
		int cents = std::ceil(total_ * multiplier * 100);
		os << name_ << ": $" <<
			std::fixed << std::setprecision(2) << cents / 100.0 <<
			std::endl;
	}
private:
	std::string name_;
	double total_;
};

std::ostream& operator<<(std::ostream &os, const ReceiptLine::Type &t) {
	switch (t) {
		case ReceiptLine::Type::Total:
			os << "Total";
			break;
		case ReceiptLine::Type::Extra:
			os << "Extra";
			break;
		case ReceiptLine::Type::Name:
			os << "Name";
			break;
		case ReceiptLine::Type::Item:
			os << "Item";
			break;
	}
	return os;
}

const std::regex ReceiptLine::pricer(R"aw(\$[0-9]+\.[0-9]{2})aw");
const std::regex ReceiptLine::totalr(R"aw(\$[0-9]+\.[0-9]{2}(?=\sPaid\swith))aw");
const std::regex ReceiptLine::namer(R"aw(-\sFor:\s.*(?=\s*-))aw");
const std::sregex_iterator ReceiptLine::endri;

int main(int argc, char **argv) {
	std::vector<Tab> tabs;
	double total = -1;
	double subtotal = 0;
	for (std::string line; std::getline(std::cin, line); ) {
		ReceiptLine rl(line);
		switch (rl.type()) {
			case ReceiptLine::Type::Total:
				total = rl.price();
				break;
			case ReceiptLine::Type::Name:
				tabs.emplace_back(rl.name());
				break;
			case ReceiptLine::Type::Item:
				if (tabs.empty()) {
					std::cerr << "Item before Name" << std::endl;
					return 1;
				}
				tabs.back().add(rl.price());
				subtotal += rl.price();
				break;
			case ReceiptLine::Type::Extra:
				break;
		}
	}
	if (total < 0) {
		std::cerr << "Total price not found" << std::endl;
	}
	double multiplier = total / subtotal;
	for (auto& tab : tabs) {
		tab.print(std::cout, multiplier);
	}
	return 0;
}
