#include "HighScores.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <utility>

// Current release target: Linux.
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>

namespace fs = std::filesystem;

namespace {

constexpr std::size_t MAX_RECORDS = 4096;
constexpr std::uintmax_t MAX_FILE_SIZE = 1024 * 1024;

struct Profile {
    std::string name;
    int solo = 0;
    int coop = 0;
    int legacy = 0;
};

fs::path configuredPath(const char* variable) {
    const char* value = std::getenv(variable);

    if (value && *value) {
        return fs::absolute(value);
    }

    return {};
}

fs::path dataDirectory() {
    if (auto custom = configuredPath("ASTEROIDS_DATA_DIR"); !custom.empty()) {
        return custom;
    }

    if (const char* xdg = std::getenv("XDG_DATA_HOME");
        xdg && *xdg && fs::path(xdg).is_absolute()) {
        return fs::path(xdg) / "asteroids";
    }

    if (const char* home = std::getenv("HOME"); home && *home) {
        return fs::path(home) / ".local" / "share" / "asteroids";
    }

    throw std::runtime_error("Cannot determine user data directory");
}

std::vector<Profile> loadFile(const fs::path& path) {
    if (fs::file_size(path) > MAX_FILE_SIZE) {
        throw std::runtime_error("Score file exceeds size limit");
    }

    std::ifstream input(path, std::ios::binary);

    if (!input) {
        throw std::runtime_error("Cannot open score file");
    }

    std::string magic;
    int version = 0;
    std::size_t count = 0;

    if (!(input >> magic >> version >> count) || magic != "ASTEROIDS_SCORES" ||
        version != 1 || count > MAX_RECORDS) {
        throw std::runtime_error("Invalid score file header");
    }

    std::vector<Profile> profiles;
    profiles.reserve(count);

    for (std::size_t i = 0; i < count; ++i) {
        Profile profile;

        input >> std::ws;

        if (input.peek() != '"' ||
            !(input >> std::quoted(profile.name) >> profile.solo >>
              profile.coop >> profile.legacy) ||
            !validPlayerName(profile.name) || profile.solo < 0 ||
            profile.coop < 0 || profile.legacy < 0) {
            throw std::runtime_error("Invalid score record");
        }

        const bool duplicate = std::any_of(
            profiles.begin(), profiles.end(), [&](const Profile& existing) {
                return existing.name == profile.name;
            });

        if (duplicate) {
            throw std::runtime_error("Duplicate player name");
        }

        profiles.push_back(std::move(profile));
    }

    std::string end;

    if (!(input >> end) || end != "END") {
        throw std::runtime_error("Incomplete score file");
    }

    input >> std::ws;

    if (!input.eof()) {
        throw std::runtime_error("Unexpected data after score records");
    }

    return profiles;
}

std::optional<unsigned long> legacyKey(const fs::path& directory) {
    std::ifstream input(directory / ".env");
    std::string line;

    while (std::getline(input, line)) {
        constexpr const char* prefix = "ENCRYPTION_KEY=";

        if (!line.starts_with(prefix)) {
            continue;
        }

        try {
            const std::string value = line.substr(15);
            std::size_t consumed = 0;
            const unsigned long key = std::stoul(value, &consumed);

            if (consumed == value.size()) {
                return key;
            }
        } catch (const std::exception&) {
            return std::nullopt;
        }
    }

    return std::nullopt;
}

std::string decodeLegacy(const std::string& data, unsigned long key) {
    // Only used once during migration, before runGame seeds gameplay RNG.
    std::srand(static_cast<unsigned>(key));

    std::string decoded = data;

    for (char& character : decoded) {
        const auto byte = static_cast<unsigned char>(std::rand() % 256);

        character =
            static_cast<char>(static_cast<unsigned char>(character) ^ byte);
    }

    return decoded;
}

bool parseLegacy(const std::string& line, Profile& profile) {
    const auto comma = line.rfind(',');

    if (comma == std::string::npos) {
        return false;
    }

    profile = {};
    profile.name = line.substr(0, comma);

    if (!validPlayerName(profile.name)) {
        return false;
    }

    std::istringstream input(line.substr(comma + 1));

    if (!(input >> profile.legacy) || profile.legacy < 0) {
        return false;
    }

    input >> std::ws;
    return input.eof();
}

struct Store {
    fs::path file;
    fs::path backup;
    std::vector<Profile> profiles;

    std::string notice;
    std::string error;

    bool writable = false;
    bool primaryValid = false;
    int lockFd = -1;

    Store() {
        try {
            const auto directory = dataDirectory();
            fs::create_directories(directory);

            file = directory / "scores-v1.txt";
            backup = directory / "scores-v1.txt.bak";

            const auto lockPath = directory / "scores.lock";

            lockFd =
                ::open(lockPath.c_str(), O_CREAT | O_RDWR | O_CLOEXEC, 0600);

            writable = lockFd >= 0 && ::flock(lockFd, LOCK_EX | LOCK_NB) == 0;

            if (!writable) {
                error = "Scores are session-only: another game may be running\n"
                        "or the data directory is not writable.";
            }

            if (fs::exists(file)) {
                try {
                    profiles = loadFile(file);
                    primaryValid = true;
                    return;
                } catch (const std::exception& exception) {
                    std::cerr << "Cannot read " << file << ": "
                              << exception.what() << '\n';
                }

                recoverBackup();
                return;
            }

            if (fs::exists(backup)) {
                recoverBackup();
                return;
            }

            auto legacyDirectory = configuredPath("ASTEROIDS_LEGACY_DIR");

            if (legacyDirectory.empty()) {
                legacyDirectory = fs::current_path();
            }

            importLegacy(legacyDirectory);

        } catch (const std::exception& exception) {
            writable = false;
            error = "Scores are session-only: storage initialization failed.\n"
                    "See terminal output for details.";

            std::cerr << "Highscores: " << exception.what() << '\n';
        }
    }

    ~Store() {
        if (lockFd >= 0) {
            ::close(lockFd);
        }
    }

    Store(const Store&) = delete;
    Store& operator=(const Store&) = delete;

    void recoverBackup() {
        writable = false;

        try {
            profiles = loadFile(backup);

            error = "Backup loaded for this session. Saving is disabled.\n"
                    "Repair the score files before the next session.";

        } catch (const std::exception& exception) {
            profiles.clear();

            error = "Score files could not be read. Saving is disabled.\n"
                    "Existing files have been preserved.";

            std::cerr << "Highscore recovery: " << exception.what() << '\n';
        }
    }

    void importLegacy(const fs::path& directory) {
        const auto legacyFile = directory / ".highscores.txt";

        if (!fs::exists(legacyFile)) {
            return;
        }

        if (fs::file_size(legacyFile) > MAX_FILE_SIZE) {
            throw std::runtime_error("Legacy file exceeds size limit");
        }

        std::ifstream input(legacyFile, std::ios::binary);

        if (!input) {
            throw std::runtime_error("Cannot read legacy score file");
        }

        const auto key = legacyKey(directory);
        std::size_t skipped = 0;
        std::string line;

        while (std::getline(input, line)) {
            Profile profile;
            bool parsed = false;

            if (key) {
                parsed = parseLegacy(decodeLegacy(line, *key), profile);
            }

            if (!parsed) {
                parsed = parseLegacy(line, profile);
            }

            if (!parsed) {
                ++skipped;
                continue;
            }

            auto existing = std::find_if(profiles.begin(), profiles.end(),
                                         [&](const Profile& entry) {
                                             return entry.name == profile.name;
                                         });

            if (existing != profiles.end()) {
                existing->legacy = std::max(existing->legacy, profile.legacy);
            } else if (profiles.size() < MAX_RECORDS) {
                profiles.push_back(std::move(profile));
            } else {
                ++skipped;
            }
        }

        if (input.bad()) {
            throw std::runtime_error("Legacy file read failed");
        }

        notice = "Legacy import: " + std::to_string(profiles.size()) +
                 " profiles; " + std::to_string(skipped) +
                 " unreadable fragments skipped.";

        std::cerr << notice << "\nOriginal files preserved: " << directory
                  << '\n';

        save();
    }

    bool save() {
        if (!writable) {
            return false;
        }

        const fs::path temporary = file.string() + ".tmp";
        const fs::path backupTemporary = backup.string() + ".tmp";

        try {
            {
                std::ofstream output;
                output.exceptions(std::ios::failbit | std::ios::badbit);

                output.open(temporary, std::ios::binary | std::ios::trunc);

                output << "ASTEROIDS_SCORES 1 " << profiles.size() << '\n';

                for (const auto& profile : profiles) {
                    output << std::quoted(profile.name) << ' ' << profile.solo
                           << ' ' << profile.coop << ' ' << profile.legacy
                           << '\n';
                }

                output << "END\n";
                output.close();
            }

            if (primaryValid) {
                fs::copy_file(file, backupTemporary,
                              fs::copy_options::overwrite_existing);

                fs::rename(backupTemporary, backup);
            }

            // Same-directory replacement is atomic on Linux.
            fs::rename(temporary, file);

            primaryValid = true;
            error.clear();
            return true;

        } catch (const std::exception& exception) {
            error = "Changes are session-only: saving failed.\n"
                    "See terminal output for details.";

            std::cerr << "Saving highscores: " << exception.what() << '\n';

            return false;
        }
    }
};

Store& store() {
    static Store instance;
    return instance;
}

} // namespace

bool validPlayerName(const std::string& name) {
    if (name.empty() || name.size() > 32) {
        return false;
    }

    bool hasNonSpace = false;

    for (unsigned char character : name) {
        if (character < 32 || character > 126) {
            return false;
        }

        hasNonSpace |= character != ' ';
    }

    return hasNonSpace;
}

std::vector<HighScore> readHighScores(ScoreMode mode) {
    std::vector<HighScore> result;

    for (const auto& profile : store().profiles) {
        result.push_back({profile.name,
                          mode == ScoreMode::Solo ? profile.solo : profile.coop,
                          profile.legacy});
    }

    std::sort(result.begin(), result.end(),
              [](const HighScore& left, const HighScore& right) {
                  if (left.score != right.score) {
                      return left.score > right.score;
                  }

                  return left.name < right.name;
              });

    return result;
}

bool writeHighScore(const std::string& name, int score, ScoreMode mode) {
    auto& data = store();

    if (!validPlayerName(name) || score < 0) {
        data.error = "Invalid player name or score.";
        return false;
    }

    auto profile =
        std::find_if(data.profiles.begin(), data.profiles.end(),
                     [&](const Profile& entry) { return entry.name == name; });

    if (profile == data.profiles.end()) {
        if (data.profiles.size() >= MAX_RECORDS) {
            data.error = "Player limit reached.";
            return false;
        }

        data.profiles.push_back({name, 0, 0, 0});
        profile = std::prev(data.profiles.end());
    }

    int& best = mode == ScoreMode::Solo ? profile->solo : profile->coop;

    best = std::max(best, score);
    return data.save();
}

bool removePlayer(const std::string& name) {
    auto& data = store();

    std::erase_if(data.profiles,
                  [&](const Profile& profile) { return profile.name == name; });

    return data.save();
}

std::string highScoreStatus() {
    const auto& data = store();

    if (data.notice.empty())
        return data.error;

    if (data.error.empty())
        return data.notice;

    return data.notice + '\n' + data.error;
}
