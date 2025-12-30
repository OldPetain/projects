#ifndef PEOPLE_H
#define PEOPLE_H

#include <string>
#include <iostream>

class People {
public:
    int id;
    std::string name;

    People() : id(0), name("") {}
    People(int i, const std::string& n) : id(i), name(n) {}

    bool operator<(const People& other) const {
        return id < other.id;
    }

    friend std::ostream& operator<<(std::ostream& os, const People& p) {
        os << "ID: " << p.id << ", Name: " << p.name;
        return os;
    }
};

#endif
