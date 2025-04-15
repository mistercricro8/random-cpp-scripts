#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include "json.hpp"

using json = nlohmann::json;

const std::string PROGRAM_PATH = "/home/cricro/nixos-config/programs/flake-init/";
const std::string GROUPS_PATH = PROGRAM_PATH + "groups/";

json read_json(const std::string &filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("no file: " + filepath);
    }
    json j;
    file >> j;
    return j;
}

void write_to_file(const std::string &filepath, const std::string &content) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("no file: " + filepath);
    }
    file << content;
}

void process_base_nix(const std::string &base_filepath, const std::vector<std::string> &vspkgs, const std::vector<std::string> &vsextpkgs, const std::vector<std::string> &pkgs, const std::vector<std::string> &inithooks, const std::string &output_filepath) {
    std::ifstream base_file(base_filepath);
    if (!base_file.is_open()) {
        throw std::runtime_error("no base.nix file: " + base_filepath);
    }

    std::ofstream output_file(output_filepath);
    if (!output_file.is_open()) {
        throw std::runtime_error("no output file: " + output_filepath);
    }

    std::string line;
    while (std::getline(base_file, line)) {
        bool replaced = false;
        if (line.find("vspkgs") != std::string::npos) {
            replaced = true;
            for (const auto &pkg : vspkgs) {
                output_file << "            " << pkg << "\n";
            }
        } else if (line.find("vsextpkgs") != std::string::npos) {
            replaced = true;
            for (const auto &pkg : vsextpkgs) {
                output_file << "            " << pkg << "\n";
            }
        } else if (line.find("basepkgs") != std::string::npos) {
            replaced = true;
            for (const auto &pkg : pkgs) {
                output_file << "        " << pkg << "\n";
            }
        } else if (line.find("inithooks") != std::string::npos) {
            replaced = true;
            for (const auto &hook : inithooks) {
                output_file << "        " << hook << "\n";
            }
        }
        if (!replaced) {
            output_file << line << "\n";
        }
    }
}

int main(int argc, char *argv[]) {
    std::vector<std::string> pkgs;
    std::vector<std::string> vspkgs;
    std::vector<std::string> vsextpkgs;
    std::vector<std::string> inithooks;

    try {
        for (int i = 1; i < argc; ++i) {
            std::string input = argv[i];
            std::string filepath = GROUPS_PATH + input + ".json";

            json j = read_json(filepath);

            for (const auto &pkg : j["pkgs"]) {
                pkgs.push_back(pkg);
            }

            for (const auto &vspkg : j["vspkgs"]) {
                vspkgs.push_back(vspkg);
            }

            for (const auto &vsextpkg : j["vsextpkgs"]) {
                vsextpkgs.push_back(vsextpkg);
            }

            for (const auto &hook : j["inithooks"]) {
                inithooks.push_back(hook);
            }
        }

        const std::string base_nix_filepath = PROGRAM_PATH + "base.nix";
        const std::string flake_nix_filepath = "flake.nix";
        process_base_nix(base_nix_filepath, vspkgs, vsextpkgs, pkgs, inithooks, flake_nix_filepath);

        std::cout << "flake done :+1:" << std::endl;

    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
