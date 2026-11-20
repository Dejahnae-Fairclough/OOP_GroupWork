#ifndef KPFL_STADIUM_HPP
#define KPFL_STADIUM_HPP

#include "FootballTeam.hpp"

namespace kpfl {

// Field condition  

enum class FieldCondition {
    Good,
    Fair,
    Poor,
    Closed
};

inline std::string conditionName(FieldCondition condition) {
    switch (condition) {
        case FieldCondition::Good:
            return "Good";

        case FieldCondition::Fair:
            return "Fair";

        case FieldCondition::Poor:
            return "Poor";

        case FieldCondition::Closed:
            return "Closed";
    }

    throw ValidationError("Invalid field condition.");
}

// Stadium class 

class Stadium {
private:
    Id id_;
    std::string name_;
    std::string location_;

    int seatingCapacity_;
    FieldCondition fieldCondition_;

    // Ticket price is in cents and includes 15% GCT.
    Money ticketPrice_;

    std::set<Id> practiceTeamIds_;

    friend class FootballLeague;

public:
    Stadium(
        Id id,
        std::string name,
        std::string location,
        int seatingCapacity,
        FieldCondition fieldCondition,
        Money ticketPrice
    )
        : id_(id),
          name_(std::move(name)),
          location_(std::move(location)),
          seatingCapacity_(seatingCapacity),
          fieldCondition_(fieldCondition),
          ticketPrice_(ticketPrice) {

        validateId(id_);
        validateText(name_, "Stadium name");
        validateText(location_, "Stadium location");

        require(
            seatingCapacity_ >= 1 &&
            seatingCapacity_ <= MAX_CAPACITY,
            "Seating capacity must be between 1 and 1000000."
        );

        // Also rejects invalid enum values.
        (void)conditionName(fieldCondition_);

        validateMoney(ticketPrice_);
    }

    Id id() const noexcept {
        return id_;
    }

    const std::string& name() const noexcept {
        return name_;
    }

    const std::string& location() const noexcept {
        return location_;
    }

    int seatingCapacity() const noexcept {
        return seatingCapacity_;
    }

    FieldCondition fieldCondition() const noexcept {
        return fieldCondition_;
    }

    Money ticketPrice() const noexcept {
        return ticketPrice_;
    }

    bool isOpen() const noexcept {
        return fieldCondition_ != FieldCondition::Closed;
    }

    bool hasPracticePermission(Id teamId) const {
        return practiceTeamIds_.count(teamId) != 0;
    }

    const std::set<Id>& practiceTeams() const noexcept {
        return practiceTeamIds_;
    }

    void display(std::ostream& output) const {
        output << id_ << " | " << name_
               << " | " << location_
               << " | Capacity: " << seatingCapacity_
               << " | Field: " << conditionName(fieldCondition_)
               << " | Ticket: " << formatMoney(ticketPrice_)
               << " (15% GCT included)\n";
    }
};

} // namespace kpfl

#endif // KPFL_STADIUM_HPP
