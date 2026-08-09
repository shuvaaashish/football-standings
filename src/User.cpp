#include "User.h"

User::User(const std::string& username) : username(username) {
}

User::~User() {
}

std::string User::getUsername() const {
    return username;
}

Admin::Admin(const std::string& username) : User(username) {
}

Admin::~Admin() {
}

bool Admin::canEditResults() const {
    return true;
}

bool Admin::canManageTeams() const {
    return true;
}

std::string Admin::getRole() const {
    return "Admin";
}

Viewer::Viewer(const std::string& username) : User(username) {
}

Viewer::~Viewer() {
}

bool Viewer::canEditResults() const {
    return false;
}

bool Viewer::canManageTeams() const {
    return false;
}

std::string Viewer::getRole() const {
    return "Viewer";
}
