#ifndef KPFL_FOOTBALL_TEAM_HPP
#define KPFL_FOOTBALL_TEAM_HPP

#include <cctype>
#include <cstdint>
#include <iomanip>
#include <ostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace kpfl {

// Shared types used by the three classes.
using Id = int;
using Money = std::int64_t;

// Money is stored in cents.
constexpr Id MAX_ID = 1'000'000'000;
constexpr Money MAX_MONEY = 1'000'000'000'000'000LL;
constexpr int MAX_CAPACITY = 1'000'000;
constexpr int MAX_SCORE = 1'000'000;
constexpr std::size_t MAX_RECORDS = 100'000;

// Custom exceptions 

class ValidationError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class ConflictError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// Shared validation 

inline void require(bool condition, const std::string& message) {
    if (!condition) {
        throw ValidationError(message);
    }
}

inline void validateId(Id id) {
    require(
        id >= 1 && id <= MAX_ID,
        "ID must be between 1 and 1000000000."
    );
}

inline void validateText(
    const std::string& text,
    const std::string& label
) {
    require(!text.empty(), label + " cannot be empty.");

    require(
        text.size() <= 200,
        label + " cannot exceed 200 characters."
    );

    bool hasVisibleCharacter = false;

    for (unsigned char character : text) {
        require(
            character >= 32 && character != 127,
            label + " cannot contain control characters."
        );

        if (!std::isspace(character)) {
            hasVisibleCharacter = true;
        }
    }

    require(
        hasVisibleCharacter,
        label + " cannot contain only spaces."
    );
}

inline void validateMoney(Money amount) {
    require(
        amount >= 0 && amount <= MAX_MONEY,
        "Money amount is outside the supported range."
    );
}

inline std::string lowercase(std::string text) {
    for (char& character : text) {
        character = static_cast<char>(
            std::tolower(static_cast<unsigned char>(character))
        );
    }

    return text;
}

inline std::string formatMoney(Money amount) {
    validateMoney(amount);

    std::ostringstream output;

    output << "JMD " << amount / 100 << '.'
           << std::setw(2)
           << std::setfill('0')
           << amount % 100;

    return output.str();
}

// Team status 

enum class TeamStatus {
    Pending,
    Active,
    Suspended
};

inline std::string statusName(TeamStatus status) {
    switch (status) {
        case TeamStatus::Pending:
            return "Pending";

        case TeamStatus::Active:
            return "Active";

        case TeamStatus::Suspended:
            return "Suspended";
    }

    throw ValidationError("Invalid team status.");
}

// FootballTeam class 

class FootballTeam {
private:
    Id id_;
    std::string name_;
    std::string parish_;
    std::string registrationNumber_;

    TeamStatus status_{TeamStatus::Pending};
    Id approvedBy_{0};

    // The team refers to people using their IDs.
    std::set<Id> playerIds_;
    std::set<Id> coachIds_;

    // The league controls changes that affect other records.
    friend class FootballLeague;

public:
    FootballTeam(
        Id id,
        std::string name,
        std::string parish,
        std::string registrationNumber
    )
        : id_(id),
          name_(std::move(name)),
          parish_(std::move(parish)),
          registrationNumber_(std::move(registrationNumber)) {

        validateId(id_);
        validateText(name_, "Team name");
        validateText(parish_, "Parish");
        validateText(registrationNumber_, "Registration number");
    }

    Id id() const noexcept {
        return id_;
    }

    const std::string& name() const noexcept {
        return name_;
    }

    const std::string& parish() const noexcept {
        return parish_;
    }

    const std::string& registrationNumber() const noexcept {
        return registrationNumber_;
    }

    TeamStatus status() const noexcept {
        return status_;
    }

    Id approvedBy() const noexcept {
        return approvedBy_;
    }

    bool isActive() const noexcept {
        return status_ == TeamStatus::Active;
    }

    // Calculate squad size to avoid storing duplicate information.
    std::size_t squadSize() const noexcept {
        return playerIds_.size();
    }

    const std::set<Id>& players() const noexcept {
        return playerIds_;
    }

    const std::set<Id>& coaches() const noexcept {
        return coachIds_;
    }

    void display(std::ostream& output) const {
        output << id_ << " | " << name_
               << " | " << parish_
               << " | Registration: " << registrationNumber_
               << " | Status: " << statusName(status_)
               << " | Players: " << playerIds_.size()
               << " | Coaches: " << coachIds_.size()
               << '\n';
    }
};

} // namespace kpfl

#endif // KPFL_FOOTBALL_TEAM_HPP
