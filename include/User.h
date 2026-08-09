#ifndef USER_H
#define USER_H

#include <string>

// Base class for every account in the app.
class User {
protected:
    std::string username;

public:
    User(const std::string& username);
    virtual ~User();

    std::string getUsername() const;

    // Role permissions.
    virtual bool canEditResults() const = 0;
    virtual bool canManageTeams() const = 0;
    virtual std::string getRole() const = 0;
};

class Admin : public User {
public:
    Admin(const std::string& username);
    virtual ~Admin();

    bool canEditResults() const;
    bool canManageTeams() const;
    std::string getRole() const;
};

class Viewer : public User {
public:
    Viewer(const std::string& username);
    virtual ~Viewer();

    bool canEditResults() const;
    bool canManageTeams() const;
    std::string getRole() const;
};

#endif
