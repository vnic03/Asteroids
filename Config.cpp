#include <unordered_map>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <random>
#include <filesystem>

/*
 * The Config class manages configuration settings,
 * loading them from an environment file
 */
class Config {
private:
    // Stores key-value pairs from the env file
    std::unordered_map<std::string, std::string> env;
    // Holds the encryption key value
    unsigned long Key;

public:
    explicit Config(const std::string& envPath) {
        readEnv(envPath);
        if (env.find("ENCRYPTION_KEY") != env.end()) {
            Key = std::stoul(env["ENCRYPTION_KEY"]);
        }
    }

    // Creates the .env for the first time with a random generated key
    static void createEnv(const std::string& envPath) {
        if (!std::filesystem::exists(envPath)) {

            std::random_device rd;
            std::default_random_engine eng(rd());
            std::uniform_int_distribution<unsigned long> distr;

            unsigned long randomKey = distr(eng);

            // Create the .env-file and writes key.
            std::ofstream file(envPath);
            file << "ENCRYPTION_KEY=" << randomKey;
            file.close();
        }
    }

    unsigned long getKey() const {
        return Key;
    }

private:
    // Reads key-value pairs from the specified environment file and stores them
    std::unordered_map<std::string, std::string> readEnv(const std::string& path) {
        std::ifstream file(path);
        std::string line;

        try  {
            while(std::getline(file, line)) {
                std::istringstream is_line(line);
                std::string key;
                // Split line into key and value at '='
                if (std::getline(is_line, key, '=')) {
                    std::string value;
                    if (std::getline(is_line, value)) {
                        env[key] = value;
                    }
                }
            }
            return env;

        } catch (const std::exception& e) {
            std::cerr << "Error while reading env: " << e.what() << std::endl;
            return {};
        }
    }
};