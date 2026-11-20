#ifndef KPFL_LEAGUE_CORE_HPP
#define KPFL_LEAGUE_CORE_HPP

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <iomanip>
#include <map>
#include <optional>
#include <ostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace kpfl {

// Money is stored in cents to avoid floating-point errors.
using Id = int;
using Money = std::int64_t;

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

// Validation helpers

inline void require(bool condition, const std::string& message) {
    if (!condition) {
        throw ValidationError(message);
    }
}

inline void validateId(Id id) {
    require(id >= 1 && id <= MAX_ID,
            "ID must be between 1 and 1000000000.");
}

inline void validateText(
    const std::string& text,
    const std::string& label
) {
    require(!text.empty(), label + " cannot be empty.");
    require(text.size() <= 200, label + " cannot exceed 200 characters.");

    bool hasVisibleCharacter = false;

    for (unsigned char character : text) {
        require(character >= 32 && character != 127,
                label + " cannot contain control characters.");

        if (!std::isspace(character)) {
            hasVisibleCharacter = true;
        }
    }

    require(hasVisibleCharacter,
            label + " cannot contain only spaces.");
}

inline void validateMoney(Money amount) {
    require(amount >= 0 && amount <= MAX_MONEY,
            "Money amount is outside the supported range.");
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
           << std::setw(2) << std::setfill('0') << amount % 100;

    return output.str();
}

// Status types 

enum class TeamStatus {
    Pending,
    Active,
    Suspended
};

enum class FieldCondition {
    Good,
    Fair,
    Poor,
    Closed
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

// FOOTBALL TEAM 

class FootballTeam {
private:
    Id id_;
    std::string name_;
    std::string parish_;
    std::string registrationNumber_;

    TeamStatus status_{TeamStatus::Pending};
    Id approvedBy_{0};

    // People exist separately; the team stores their IDs.
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

// STADIUM 

class Stadium {
private:
    Id id_;
    std::string name_;
    std::string location_;

    int seatingCapacity_;
    FieldCondition fieldCondition_;
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
            seatingCapacity_ >= 1 && seatingCapacity_ <= MAX_CAPACITY,
            "Seating capacity must be between 1 and 1000000."
        );

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

// Scheduling helpers 

inline bool isLeapYear(int year) {
    return year % 4 == 0 &&
           (year % 100 != 0 || year % 400 == 0);
}

inline int readDigits(
    const std::string& text,
    std::size_t start,
    std::size_t length
) {
    int value = 0;

    for (std::size_t index = start; index < start + length; ++index) {
        unsigned char character =
            static_cast<unsigned char>(text[index]);

        require(std::isdigit(character) != 0,
                "Date and time must contain valid digits.");

        value = value * 10 + (character - '0');
    }

    return value;
}

// Convert a valid date and time into an absolute minute count.
inline std::int64_t absoluteMinute(
    const std::string& date,
    const std::string& time
) {
    require(
        date.size() == 10 && date[4] == '-' && date[7] == '-',
        "Date must use YYYY-MM-DD."
    );

    require(
        time.size() == 5 && time[2] == ':',
        "Time must use HH:MM."
    );

    int year = readDigits(date, 0, 4);
    int month = readDigits(date, 5, 2);
    int day = readDigits(date, 8, 2);

    int hour = readDigits(time, 0, 2);
    int minute = readDigits(time, 3, 2);

    require(year >= 1900 && year <= 9999, "Invalid year.");
    require(month >= 1 && month <= 12, "Invalid month.");
    require(hour <= 23 && minute <= 59, "Invalid time.");

    static const int monthDays[] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };

    int maximumDay = monthDays[month - 1];

    if (month == 2 && isLeapYear(year)) {
        ++maximumDay;
    }

    require(day >= 1 && day <= maximumDay,
            "Invalid day for the selected month.");

    std::int64_t previousYear = year - 1;

    std::int64_t days =
        previousYear * 365 +
        previousYear / 4 -
        previousYear / 100 +
        previousYear / 400;

    for (int currentMonth = 1; currentMonth < month; ++currentMonth) {
        days += monthDays[currentMonth - 1];

        if (currentMonth == 2 && isLeapYear(year)) {
            ++days;
        }
    }

    days += day - 1;

    return days * 1440 + hour * 60 + minute;
}

// Supporting records 

struct MatchResult {
    int homeGoals{};
    int awayGoals{};
};

struct Fixture {
    Id id{};
    Id homeTeamId{};
    Id awayTeamId{};
    Id stadiumId{};

    std::string date;
    std::string time;

    int durationMinutes{120};
    std::optional<MatchResult> result;

    std::int64_t startMinute() const {
        return absoluteMinute(date, time);
    }

    std::int64_t endMinute() const {
        return startMinute() + durationMinutes;
    }

    bool isCompleted() const noexcept {
        return result.has_value();
    }
};

struct Standing {
    Id teamId{};
    std::string teamName;

    int played{};
    int won{};
    int drawn{};
    int lost{};

    std::int64_t goalsFor{};
    std::int64_t goalsAgainst{};

    int points{};

    std::int64_t goalDifference() const noexcept {
        return goalsFor - goalsAgainst;
    }
};

// FOOTBALL LEAGUE 

class FootballLeague {
private:
    std::string name_;
    int minimumPlayerAge_;

    // The league owns its teams, stadiums, and fixtures.
    std::map<Id, FootballTeam> teams_;
    std::map<Id, Stadium> stadiums_;
    std::map<Id, Fixture> fixtures_;

    // These indexes will connect to the full Person hierarchy.
    struct PlayerRegistration {
        int age{};
        int jerseyNumber{};
    };

    std::map<Id, PlayerRegistration> registeredPlayers_;
    std::set<Id> registeredCoaches_;
    std::set<Id> registeredCommissioners_;

    template <typename Collection>
    static void checkRecordLimit(const Collection& records) {
        require(records.size() < MAX_RECORDS,
                "The record limit has been reached.");
    }

    bool personIdExists(Id personId) const {
        return registeredPlayers_.count(personId) != 0 ||
               registeredCoaches_.count(personId) != 0 ||
               registeredCommissioners_.count(personId) != 0;
    }

    void checkNewPersonId(Id personId) const {
        validateId(personId);

        require(!personIdExists(personId),
                "This person ID is already registered.");

        std::size_t count =
            registeredPlayers_.size() +
            registeredCoaches_.size() +
            registeredCommissioners_.size();

        require(count < MAX_RECORDS,
                "The personnel record limit has been reached.");
    }

    void checkCommissioner(Id commissionerId) const {
        require(
            registeredCommissioners_.count(commissionerId) != 0,
            "A registered commissioner must authorize this operation."
        );
    }

    FootballTeam& mutableTeam(Id teamId) {
        auto found = teams_.find(teamId);

        require(found != teams_.end(), "Team ID was not found.");

        return found->second;
    }

    Stadium& mutableStadium(Id stadiumId) {
        auto found = stadiums_.find(stadiumId);

        require(found != stadiums_.end(), "Stadium ID was not found.");

        return found->second;
    }

    Fixture& mutableFixture(Id fixtureId) {
        auto found = fixtures_.find(fixtureId);

        require(found != fixtures_.end(), "Fixture ID was not found.");

        return found->second;
    }

    void checkRegistrationNumber(
        const std::string& registrationNumber,
        Id ignoredTeamId = 0
    ) const {
        validateText(registrationNumber, "Registration number");

        std::string candidate = lowercase(registrationNumber);

        for (const auto& entry : teams_) {
            if (entry.first == ignoredTeamId) {
                continue;
            }

            require(
                lowercase(entry.second.registrationNumber()) != candidate,
                "This registration number is already in use."
            );
        }
    }

    std::optional<Id> memberTeam(Id personId) const {
        for (const auto& entry : teams_) {
            const FootballTeam& currentTeam = entry.second;

            if (currentTeam.players().count(personId) != 0 ||
                currentTeam.coaches().count(personId) != 0) {
                return currentTeam.id();
            }
        }

        return std::nullopt;
    }

    void checkJerseyNumber(
        Id teamId,
        Id playerId,
        int jerseyNumber
    ) const {
        for (Id existingPlayerId : team(teamId).players()) {
            if (existingPlayerId == playerId) {
                continue;
            }

            require(
                registeredPlayers_.at(existingPlayerId).jerseyNumber
                    != jerseyNumber,
                "Another player on this team uses that jersey number."
            );
        }
    }

    static bool overlaps(const Fixture& first, const Fixture& second) {
        return first.startMinute() < second.endMinute() &&
               second.startMinute() < first.endMinute();
    }

    void checkScheduling(const Fixture& candidate) const {
        for (const auto& entry : fixtures_) {
            const Fixture& existing = entry.second;

            if (!overlaps(candidate, existing)) {
                continue;
            }

            bool sharedTeam =
                candidate.homeTeamId == existing.homeTeamId ||
                candidate.homeTeamId == existing.awayTeamId ||
                candidate.awayTeamId == existing.homeTeamId ||
                candidate.awayTeamId == existing.awayTeamId;

            if (sharedTeam) {
                throw ConflictError(
                    "A team already has a fixture during this period."
                );
            }

            if (candidate.stadiumId == existing.stadiumId) {
                throw ConflictError(
                    "The stadium is already booked during this period."
                );
            }
        }
    }

public:
    explicit FootballLeague(
        std::string name = "Kingston Premier Football League",
        int minimumPlayerAge = 16
    )
        : name_(std::move(name)),
          minimumPlayerAge_(minimumPlayerAge) {

        validateText(name_, "League name");

        require(
            minimumPlayerAge_ >= 1 && minimumPlayerAge_ <= 120,
            "Minimum player age must be between 1 and 120."
        );
    }

    const std::string& name() const noexcept {
        return name_;
    }

    int minimumPlayerAge() const noexcept {
        return minimumPlayerAge_;
    }

    // Read-only access prevents callers from bypassing league rules.
    const FootballTeam& team(Id teamId) const {
        auto found = teams_.find(teamId);

        require(found != teams_.end(), "Team ID was not found.");

        return found->second;
    }

    const Stadium& stadium(Id stadiumId) const {
        auto found = stadiums_.find(stadiumId);

        require(found != stadiums_.end(), "Stadium ID was not found.");

        return found->second;
    }

    const Fixture& fixture(Id fixtureId) const {
        auto found = fixtures_.find(fixtureId);

        require(found != fixtures_.end(), "Fixture ID was not found.");

        return found->second;
    }

    // Personnel links 

    void registerPlayer(Id playerId, int age, int jerseyNumber) {
        checkNewPersonId(playerId);

        require(
            age >= minimumPlayerAge_ && age <= 120,
            "Player does not meet the league age requirements."
        );

        require(
            jerseyNumber >= 1 && jerseyNumber <= 99,
            "Jersey number must be between 1 and 99."
        );

        registeredPlayers_.emplace(
            playerId,
            PlayerRegistration{age, jerseyNumber}
        );
    }

    void updatePlayer(Id playerId, int age, int jerseyNumber) {
        auto found = registeredPlayers_.find(playerId);

        require(found != registeredPlayers_.end(),
                "Player ID was not found.");

        require(
            age >= minimumPlayerAge_ && age <= 120,
            "Player does not meet the league age requirements."
        );

        require(
            jerseyNumber >= 1 && jerseyNumber <= 99,
            "Jersey number must be between 1 and 99."
        );

        auto teamId = memberTeam(playerId);

        if (teamId) {
            checkJerseyNumber(*teamId, playerId, jerseyNumber);
        }

        found->second = PlayerRegistration{age, jerseyNumber};
    }

    void registerCoach(Id coachId) {
        checkNewPersonId(coachId);
        registeredCoaches_.insert(coachId);
    }

    void registerCommissioner(Id commissionerId) {
        checkNewPersonId(commissionerId);
        registeredCommissioners_.insert(commissionerId);
    }

    // ------------------------- Team management -------------------------

    void registerTeam(
        Id teamId,
        const std::string& name,
        const std::string& parish,
        const std::string& registrationNumber
    ) {
        validateId(teamId);

        require(teams_.count(teamId) == 0,
                "This team ID is already registered.");

        checkRecordLimit(teams_);
        checkRegistrationNumber(registrationNumber);

        FootballTeam candidate(
            teamId,
            name,
            parish,
            registrationNumber
        );

        teams_.emplace(teamId, std::move(candidate));
    }

    void updateTeam(
        Id teamId,
        const std::string& name,
        const std::string& parish,
        const std::string& registrationNumber
    ) {
        FootballTeam candidate = team(teamId);

        validateText(name, "Team name");
        validateText(parish, "Parish");
        checkRegistrationNumber(registrationNumber, teamId);

        candidate.name_ = name;
        candidate.parish_ = parish;
        candidate.registrationNumber_ = registrationNumber;

        // Replace the original only after validation succeeds.
        mutableTeam(teamId) = std::move(candidate);
    }

    void approveTeam(Id teamId, Id commissionerId) {
        checkCommissioner(commissionerId);

        FootballTeam& selectedTeam = mutableTeam(teamId);

        require(
            selectedTeam.status_ == TeamStatus::Pending,
            "Only a pending team can be approved."
        );

        selectedTeam.approvedBy_ = commissionerId;
        selectedTeam.status_ = TeamStatus::Active;
    }

    void setSuspended(
        Id teamId,
        bool suspended,
        Id commissionerId
    ) {
        checkCommissioner(commissionerId);

        FootballTeam& selectedTeam = mutableTeam(teamId);

        require(
            selectedTeam.approvedBy_ != 0,
            "A pending team must be approved first."
        );

        selectedTeam.status_ = suspended
            ? TeamStatus::Suspended
            : TeamStatus::Active;

        // Suspension removes current practice permissions.
        if (suspended) {
            for (auto& entry : stadiums_) {
                entry.second.practiceTeamIds_.erase(teamId);
            }
        }
    }

    void addPlayer(Id teamId, Id playerId) {
        FootballTeam& selectedTeam = mutableTeam(teamId);

        auto found = registeredPlayers_.find(playerId);

        require(found != registeredPlayers_.end(),
                "Player must be registered before joining a team.");

        require(!memberTeam(playerId).has_value(),
                "This player already belongs to a team.");

        checkRecordLimit(selectedTeam.playerIds_);

        checkJerseyNumber(
            teamId,
            playerId,
            found->second.jerseyNumber
        );

        selectedTeam.playerIds_.insert(playerId);
    }

    void removePlayer(Id teamId, Id playerId) {
        FootballTeam& selectedTeam = mutableTeam(teamId);

        require(
            selectedTeam.playerIds_.erase(playerId) == 1,
            "This player does not belong to the selected team."
        );

        // The player's league registration is preserved.
    }

    void addCoach(Id teamId, Id coachId) {
        FootballTeam& selectedTeam = mutableTeam(teamId);

        require(
            registeredCoaches_.count(coachId) != 0,
            "Coach must be registered before joining a team."
        );

        require(!memberTeam(coachId).has_value(),
                "This coach already belongs to a team.");

        checkRecordLimit(selectedTeam.coachIds_);
        selectedTeam.coachIds_.insert(coachId);
    }

    void removeCoach(Id teamId, Id coachId) {
        FootballTeam& selectedTeam = mutableTeam(teamId);

        require(
            selectedTeam.coachIds_.erase(coachId) == 1,
            "This coach does not belong to the selected team."
        );
    }

    std::vector<Id> searchTeams(const std::string& query) const {
        validateText(query, "Search query");

        std::string searchText = lowercase(query);
        std::vector<Id> matches;

        for (const auto& entry : teams_) {
            const FootballTeam& currentTeam = entry.second;

            bool matched =
                lowercase(currentTeam.name()).find(searchText)
                    != std::string::npos ||
                lowercase(currentTeam.parish()).find(searchText)
                    != std::string::npos ||
                lowercase(currentTeam.registrationNumber()).find(searchText)
                    != std::string::npos ||
                std::to_string(currentTeam.id()) == query;

            if (matched) {
                matches.push_back(currentTeam.id());
            }
        }

        return matches;
    }

    // Stadium management 

    void registerStadium(
        Id stadiumId,
        const std::string& name,
        const std::string& location,
        int seatingCapacity,
        FieldCondition condition,
        Money ticketPrice
    ) {
        validateId(stadiumId);

        require(stadiums_.count(stadiumId) == 0,
                "This stadium ID is already registered.");

        checkRecordLimit(stadiums_);

        Stadium candidate(
            stadiumId,
            name,
            location,
            seatingCapacity,
            condition,
            ticketPrice
        );

        stadiums_.emplace(stadiumId, std::move(candidate));
    }

    void updateStadium(
        Id stadiumId,
        const std::string& name,
        const std::string& location,
        int seatingCapacity,
        FieldCondition condition,
        Money ticketPrice
    ) {
        const Stadium& existing = stadium(stadiumId);

        Stadium candidate(
            stadiumId,
            name,
            location,
            seatingCapacity,
            condition,
            ticketPrice
        );

        // Preserve permissions unless the field is being closed.
        if (candidate.isOpen()) {
            candidate.practiceTeamIds_ = existing.practiceTeamIds_;
        }

        mutableStadium(stadiumId) = std::move(candidate);
    }

    void grantPracticePermission(Id stadiumId, Id teamId) {
        const FootballTeam& selectedTeam = team(teamId);
        Stadium& selectedStadium = mutableStadium(stadiumId);

        require(selectedTeam.isActive(),
                "Only an approved, unsuspended team can practise.");

        require(selectedStadium.isOpen(),
                "Practice is not allowed on a closed field.");

        require(!selectedStadium.hasPracticePermission(teamId),
                "This team already has practice permission.");

        checkRecordLimit(selectedStadium.practiceTeamIds_);
        selectedStadium.practiceTeamIds_.insert(teamId);
    }

    void revokePracticePermission(Id stadiumId, Id teamId) {
        (void)team(teamId);

        Stadium& selectedStadium = mutableStadium(stadiumId);

        require(
            selectedStadium.practiceTeamIds_.erase(teamId) == 1,
            "This team does not have practice permission."
        );
    }

    // Fixture management 

    void createFixture(
        Id fixtureId,
        Id homeTeamId,
        Id awayTeamId,
        Id stadiumId,
        const std::string& date,
        const std::string& time,
        int durationMinutes = 120
    ) {
        validateId(fixtureId);

        require(fixtures_.count(fixtureId) == 0,
                "This fixture ID already exists.");

        checkRecordLimit(fixtures_);

        require(homeTeamId != awayTeamId,
                "A team cannot play against itself.");

        require(
            team(homeTeamId).isActive() &&
            team(awayTeamId).isActive(),
            "Both teams must be approved and unsuspended."
        );

        require(stadium(stadiumId).isOpen(),
                "A fixture cannot be scheduled at a closed stadium.");

        require(
            durationMinutes >= 1 && durationMinutes <= 1440,
            "Fixture duration must be between 1 and 1440 minutes."
        );

        (void)absoluteMinute(date, time);

        Fixture candidate;
        candidate.id = fixtureId;
        candidate.homeTeamId = homeTeamId;
        candidate.awayTeamId = awayTeamId;
        candidate.stadiumId = stadiumId;
        candidate.date = date;
        candidate.time = time;
        candidate.durationMinutes = durationMinutes;

        // This also detects fixtures that overlap across midnight.
        checkScheduling(candidate);

        fixtures_.emplace(fixtureId, std::move(candidate));
    }

    void recordResult(
        Id fixtureId,
        int homeGoals,
        int awayGoals
    ) {
        require(
            homeGoals >= 0 && homeGoals <= MAX_SCORE &&
            awayGoals >= 0 && awayGoals <= MAX_SCORE,
            "Scores must be between 0 and 1000000."
        );

        Fixture& selectedFixture = mutableFixture(fixtureId);

        require(!selectedFixture.isCompleted(),
                "A result has already been recorded.");

        // A fixture is completed only when a valid result exists.
        selectedFixture.result = MatchResult{
            homeGoals,
            awayGoals
        };
    }

    // ------------------------- League standings -------------------------

    std::vector<Standing> standings() const {
        std::map<Id, Standing> rows;

        for (const auto& entry : teams_) {
            const FootballTeam& currentTeam = entry.second;

            if (currentTeam.approvedBy() == 0) {
                continue;
            }

            Standing row;
            row.teamId = currentTeam.id();
            row.teamName = currentTeam.name();

            rows.emplace(row.teamId, std::move(row));
        }

        for (const auto& entry : fixtures_) {
            const Fixture& currentFixture = entry.second;

            if (!currentFixture.result) {
                continue;
            }

            Standing& home = rows.at(currentFixture.homeTeamId);
            Standing& away = rows.at(currentFixture.awayTeamId);

            const MatchResult& result = *currentFixture.result;

            ++home.played;
            ++away.played;

            home.goalsFor += result.homeGoals;
            home.goalsAgainst += result.awayGoals;

            away.goalsFor += result.awayGoals;
            away.goalsAgainst += result.homeGoals;

            if (result.homeGoals > result.awayGoals) {
                ++home.won;
                ++away.lost;
                home.points += 3;
            } else if (result.homeGoals < result.awayGoals) {
                ++away.won;
                ++home.lost;
                away.points += 3;
            } else {
                ++home.drawn;
                ++away.drawn;

                ++home.points;
                ++away.points;
            }
        }

        std::vector<Standing> table;
        table.reserve(rows.size());

        for (const auto& entry : rows) {
            table.push_back(entry.second);
        }

        std::sort(
            table.begin(),
            table.end(),
            [](const Standing& first, const Standing& second) {
                if (first.points != second.points) {
                    return first.points > second.points;
                }

                if (first.goalDifference() != second.goalDifference()) {
                    return first.goalDifference() > second.goalDifference();
                }

                if (first.goalsFor != second.goalsFor) {
                    return first.goalsFor > second.goalsFor;
                }

                if (first.teamName != second.teamName) {
                    return first.teamName < second.teamName;
                }

                return first.teamId < second.teamId;
            }
        );

        return table;
    }

    // Management reports 

    void printTeamReport(std::ostream& output) const {
        output << "\nTEAM REPORT\n";

        if (teams_.empty()) {
            output << "No teams registered.\n";
            return;
        }

        for (const auto& entry : teams_) {
            const FootballTeam& currentTeam = entry.second;

            currentTeam.display(output);

            output << "  Player IDs:";
            for (Id playerId : currentTeam.players()) {
                output << ' ' << playerId;
            }
            output << '\n';

            output << "  Coach IDs:";
            for (Id coachId : currentTeam.coaches()) {
                output << ' ' << coachId;
            }
            output << '\n';
        }
    }

    void printStadiumReport(std::ostream& output) const {
        output << "\nSTADIUM REPORT\n";

        if (stadiums_.empty()) {
            output << "No stadiums registered.\n";
            return;
        }

        for (const auto& entry : stadiums_) {
            const Stadium& currentStadium = entry.second;

            currentStadium.display(output);

            output << "  Teams permitted to practise:\n";

            if (currentStadium.practiceTeams().empty()) {
                output << "    None\n";
            }

            for (Id teamId : currentStadium.practiceTeams()) {
                output << "    " << team(teamId).name() << '\n';
            }
        }
    }

    void printStandingsReport(std::ostream& output) const {
        output << "\nLEAGUE STANDINGS\n"
               << "Team | P W D L | GF GA GD | Points\n";

        std::vector<Standing> table = standings();

        if (table.empty()) {
            output << "No approved teams.\n";
            return;
        }

        for (const Standing& row : table) {
            output << row.teamName << " | "
                   << row.played << ' '
                   << row.won << ' '
                   << row.drawn << ' '
                   << row.lost << " | "
                   << row.goalsFor << ' '
                   << row.goalsAgainst << ' '
                   << row.goalDifference() << " | "
                   << row.points << '\n';
        }

        output << "Ranking: points, goal difference, goals scored, "
               << "team name, then team ID.\n";
    }
};

} // namespace kpfl

#endif // KPFL_LEAGUE_CORE_HPP
