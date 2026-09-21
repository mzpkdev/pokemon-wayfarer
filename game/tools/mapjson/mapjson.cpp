// mapjson.cpp

#include <iostream>
using std::cout; using std::endl;

#include <string>
using std::string;

#include <vector>
using std::vector;

#include <algorithm>
using std::sort; using std::find;

#include <map>
using std::map;

#include <set>
using std::set;

#include <fstream>
using std::ofstream; using std::ifstream;

#include <sstream>
using std::ostringstream;

#include <limits>
using std::numeric_limits;

#include "json11.h"
using json11::Json;

#include <regex>

#include "mapjson.h"

#include <filesystem>

string version;
// System directory separator
string sep;
string wayfarer_sevii_manifest_path;
string wayfarer_sinnoh_manifest_path;
string wayfarer_sinnoh_asset_manifest_path;
bool wayfarer_sevii_release_link_enabled = false;
bool wayfarer_sinnoh_release_link_enabled = false;
set<string> wayfarer_sevii_map_names;
set<string> wayfarer_sevii_map_ids;
set<string> wayfarer_sevii_layout_ids;
set<string> wayfarer_sevii_enabled_map_names;
set<string> wayfarer_sevii_enabled_map_ids;
set<string> wayfarer_sevii_enabled_layout_ids;
map<string, Json> wayfarer_sevii_records;
map<string, bool> wayfarer_sevii_content_domains;
map<string, string> wayfarer_sevii_content_inventory;
map<string, Json> wayfarer_sinnoh_records;
set<string> wayfarer_sinnoh_enabled_map_names;
set<string> wayfarer_sinnoh_enabled_layout_ids;

string read_text_file(string filepath) {
    ifstream in_file(filepath);

    if (!in_file.is_open())
        FATAL_ERROR("Cannot open file %s for reading.\n", filepath.c_str());

    string text;

    in_file.seekg(0, std::ios::end);
    text.resize(in_file.tellg());

    in_file.seekg(0, std::ios::beg);
    in_file.read(&text[0], text.size());

    in_file.close();

    return text;
}

void write_text_file(string filepath, string text) {
    // `generated` runs before dependency scanning.  Rewriting an identical
    // include here makes every scaninc dependency older than its generated
    // map input and forces a full rescan on every build invocation.
    if (std::filesystem::exists(filepath)) {
        ifstream in_file(filepath, std::ifstream::binary);
        std::ostringstream existing;
        existing << in_file.rdbuf();
        if (in_file.good() || in_file.eof()) {
            if (existing.str() == text)
                return;
        }
    }

    ofstream out_file(filepath, std::ofstream::binary);

    if (!out_file.is_open())
        FATAL_ERROR("Cannot open file %s for writing.\n", filepath.c_str());

    out_file << text;

    out_file.close();
}


string json_to_string(const Json &data, const string &field = "", bool silent = false) {
    const Json value = !field.empty() ? data[field] : data;
    string output = "";
    switch (value.type()) {
        case Json::Type::STRING:
            output = value.string_value();
            break;
        case Json::Type::NUMBER:
            output = std::to_string(value.int_value());
            break;
        case Json::Type::BOOL:
            output = value.bool_value() ? "TRUE" : "FALSE";
            break;
        case Json::Type::NUL:
            output = "";
            break;
        default:{
            if (!silent) {
                string s = !field.empty() ? ("Value for '" + field + "'") : "JSON field";
                FATAL_ERROR("%s is unexpected type; expected string, number, or bool.\n", s.c_str());
            }
        }
    }

    if (!silent && output.empty()) {
        string s = !field.empty() ? ("Value for '" + field + "'") : "JSON field";
        FATAL_ERROR("%s cannot be empty.\n", s.c_str());
    }

    return output;
}

string get_source_version(const Json &data) {
    string source_version = json_to_string(data, "game_version", true);
    source_version = source_version.empty() ? "emerald" : source_version;
    if (source_version != "emerald" && source_version != "frlg"
     && source_version != "hns" && source_version != "sinnoh")
        FATAL_ERROR("Unsupported map source version %s.\n", source_version.c_str());
    return source_version;
}

bool source_version_is_selected(const string &source_version) {
    if (version == "wayfarer")
        return source_version == "hns" || source_version == "emerald" || source_version == "sinnoh";

    string selected_version = version == "firered" ? "frlg" : version;
    return source_version == selected_version;
}

const vector<std::pair<string, int>> wayfarer_sinnoh_source_groups = {
    {"gMapGroup_SinnohTownsRoutes", 55}, {"gMapGroup_SpecialAreasSinnoh", 5},
    {"gMapGroup_DungeonsSinnoh", 8}, {"gMapGroup_IndoorSinnoh", 14},
    {"gMapGroup_IndoorTwinleaf", 6}, {"gMapGroup_IndoorSandgem", 6},
    {"gMapGroup_IndoorJubilife", 20}, {"gMapGroup_IndoorOreburgh", 13},
    {"gMapGroup_IndoorFloaroma", 6},
};

const map<string, int> wayfarer_sinnoh_expected_counts = {
    {"maps", 133}, {"layouts", 133}, {"warps", 233}, {"connections", 114},
    {"object_events", 0}, {"coord_events", 0}, {"bg_events", 0},
    {"nonempty_map_scripts", 0}, {"wild_encounter_profiles", 0},
};

bool wayfarer_sinnoh_record_is_enabled(const Json &record) {
    const string state = json_to_string(record["inclusion"], "state");
    return (wayfarer_sinnoh_release_link_enabled && state == "INCLUDED")
        || state == "FROZEN_NOT_SELECTED";
}

bool wayfarer_sinnoh_map_is_selected(const Json &data) {
    const string map_name = json_to_string(data, "name", true);
    auto record = wayfarer_sinnoh_records.find(map_name);
    if (record == wayfarer_sinnoh_records.end())
        FATAL_ERROR("Sinnoh map %s is not registered in the Wayfarer Sinnoh manifest.\n", map_name.c_str());
    if (json_to_string(data, "id") != json_to_string(record->second, "target_map_id")
     || json_to_string(data, "layout") != json_to_string(record->second, "target_layout"))
        FATAL_ERROR("Sinnoh map %s does not match its reviewed target IDs.\n", map_name.c_str());
    return wayfarer_sinnoh_record_is_enabled(record->second);
}

bool wayfarer_sinnoh_layout_is_selected(const Json &data) {
    const string layout_id = json_to_string(data, "id");
    return wayfarer_sinnoh_enabled_layout_ids.find(layout_id) != wayfarer_sinnoh_enabled_layout_ids.end();
}

// The mainland coast is selected as one closed FRLG set.  Keep this separate
// from the Sevii manifest: its event filter deliberately excludes ordinary
// Trainers, while the coast must retain all of its source events.
const set<string> wayfarer_coast_map_names = {
    "Route19_Frlg", "Route20_Frlg", "Route21_North_Frlg", "Route21_South_Frlg",
    "CinnabarIsland_Frlg", "CinnabarIsland_Gym_Frlg",
    "CinnabarIsland_PokemonLab_Entrance_Frlg", "CinnabarIsland_PokemonLab_Lounge_Frlg",
    "CinnabarIsland_PokemonLab_ResearchRoom_Frlg", "CinnabarIsland_PokemonLab_ExperimentRoom_Frlg",
    "CinnabarIsland_PokemonCenter_1F_Frlg", "CinnabarIsland_PokemonCenter_2F_Frlg",
    "CinnabarIsland_Mart_Frlg", "PokemonMansion_1F_Frlg", "PokemonMansion_2F_Frlg",
    "PokemonMansion_3F_Frlg", "PokemonMansion_B1F_Frlg",
    "SeafoamIslands_1F_Frlg", "SeafoamIslands_B1F_Frlg", "SeafoamIslands_B2F_Frlg",
    "SeafoamIslands_B3F_Frlg", "SeafoamIslands_B4F_Frlg",
};

const set<string> wayfarer_coast_layout_ids = {
    "LAYOUT_ROUTE19", "LAYOUT_ROUTE20", "LAYOUT_ROUTE21_NORTH", "LAYOUT_ROUTE21_SOUTH",
    "LAYOUT_CINNABAR_ISLAND", "LAYOUT_CINNABAR_ISLAND_GYM",
    "LAYOUT_CINNABAR_ISLAND_POKEMON_LAB_ENTRANCE", "LAYOUT_CINNABAR_ISLAND_POKEMON_LAB_LOUNGE",
    "LAYOUT_CINNABAR_ISLAND_POKEMON_LAB_RESEARCH_ROOM", "LAYOUT_CINNABAR_ISLAND_POKEMON_LAB_EXPERIMENT_ROOM",
    "LAYOUT_POKEMON_CENTER_1F_FRLG", "LAYOUT_POKEMON_CENTER_2F_FRLG", "LAYOUT_MART_FRLG",
    "LAYOUT_POKEMON_MANSION_1F", "LAYOUT_POKEMON_MANSION_2F", "LAYOUT_POKEMON_MANSION_3F",
    "LAYOUT_POKEMON_MANSION_B1F", "LAYOUT_SEAFOAM_ISLANDS_1F", "LAYOUT_SEAFOAM_ISLANDS_B1F",
    "LAYOUT_SEAFOAM_ISLANDS_B2F", "LAYOUT_SEAFOAM_ISLANDS_B3F", "LAYOUT_SEAFOAM_ISLANDS_B4F",
    "LAYOUT_SEAFOAM_ISLANDS_B3F_CURRENT_STOPPED", "LAYOUT_SEAFOAM_ISLANDS_B4F_CURRENT_STOPPED",
};

// S.S. Anne is deliberately a closed interior-only adventure.  Its exterior
// remains a source-only map: the two corridor exits are resolved directly to
// the existing Vermilion dock instead.
const set<string> wayfarer_anne_map_names = {
    "SSAnne_1F_Corridor_Frlg", "SSAnne_1F_Room1_Frlg", "SSAnne_1F_Room2_Frlg",
    "SSAnne_1F_Room3_Frlg", "SSAnne_1F_Room4_Frlg", "SSAnne_1F_Room5_Frlg",
    "SSAnne_1F_Room6_Frlg", "SSAnne_1F_Room7_Frlg", "SSAnne_2F_Corridor_Frlg",
    "SSAnne_2F_Room1_Frlg", "SSAnne_2F_Room2_Frlg", "SSAnne_2F_Room3_Frlg",
    "SSAnne_2F_Room4_Frlg", "SSAnne_2F_Room5_Frlg", "SSAnne_2F_Room6_Frlg",
    "SSAnne_3F_Corridor_Frlg", "SSAnne_B1F_Corridor_Frlg", "SSAnne_B1F_Room1_Frlg",
    "SSAnne_B1F_Room2_Frlg", "SSAnne_B1F_Room3_Frlg", "SSAnne_B1F_Room4_Frlg",
    "SSAnne_B1F_Room5_Frlg", "SSAnne_CaptainsOffice_Frlg", "SSAnne_Deck_Frlg",
    "SSAnne_Kitchen_Frlg",
};

const set<string> wayfarer_anne_layout_ids = {
    "LAYOUT_SSANNE_1F_CORRIDOR", "LAYOUT_SSANNE_2F_CORRIDOR", "LAYOUT_SSANNE_3F_CORRIDOR",
    "LAYOUT_SSANNE_B1F_CORRIDOR", "LAYOUT_SSANNE_CAPTAINS_OFFICE", "LAYOUT_SSANNE_DECK",
    "LAYOUT_SSANNE_KITCHEN", "LAYOUT_SSANNE_ROOM1", "LAYOUT_SSANNE_ROOM2",
};

bool data_matches_version(const Json &data) {
    if (get_source_version(data) == "sinnoh") {
        // Sinnoh is not an Emerald binary-layout format. Its maps deliberately
        // retain Emerald layout_version while this source provenance is gated
        // through the reviewed Wayfarer manifest.
        if (version != "wayfarer")
            return false;
        if (wayfarer_sinnoh_manifest_path.empty())
            return false;
        if (data["layout"].type() == Json::Type::STRING)
            return wayfarer_sinnoh_map_is_selected(data);
        return wayfarer_sinnoh_layout_is_selected(data);
    }

    // Navigation-only coast previews are retired from every product catalog.
    // Standalone HNS keeps its authored coast maps; standalone FRLG keeps its
    // source coast maps. Neither product needs the former Wayfarer prototypes.
    if (version == "wayfarer" && get_source_version(data) == "hns") {
        static const set<string> retired_preview_ids = {
            "MAP_CINNABAR_SEAM_POC", "MAP_ROUTE19_COAST_POC", "MAP_ROUTE20_COAST_POC",
            "MAP_ROUTE21_NORTH_COAST_POC", "MAP_ROUTE21_SOUTH_COAST_POC",
            "MAP_SEAFOAM_ISLANDS_1F_COAST_POC", "MAP_SEAFOAM_ISLANDS_B1F_COAST_POC",
            "MAP_SEAFOAM_ISLANDS_B2F_COAST_POC", "MAP_SEAFOAM_ISLANDS_B3F_COAST_POC",
            "MAP_SEAFOAM_ISLANDS_B4F_COAST_POC",
            "LAYOUT_CINNABAR_SEAM_POC", "LAYOUT_ROUTE19_COAST_POC", "LAYOUT_ROUTE20_COAST_POC",
            "LAYOUT_ROUTE21_NORTH_COAST_POC", "LAYOUT_ROUTE21_SOUTH_COAST_POC",
            "LAYOUT_SEAFOAM_ISLANDS_1F_COAST_POC", "LAYOUT_SEAFOAM_ISLANDS_B1F_COAST_POC",
            "LAYOUT_SEAFOAM_ISLANDS_B2F_COAST_POC", "LAYOUT_SEAFOAM_ISLANDS_B3F_COAST_POC",
            "LAYOUT_SEAFOAM_ISLANDS_B4F_COAST_POC",
        };
        const string id = json_to_string(data, "id", true);
        if (retired_preview_ids.count(id))
            return false;
        if (id.size() >= 5 && id.substr(id.size() - 5) == "_PORT"
         && (id.rfind("MAP_CINNABAR_ISLAND_", 0) == 0
          || id.rfind("MAP_POKEMON_MANSION_", 0) == 0
          || id.rfind("LAYOUT_CINNABAR_ISLAND_", 0) == 0
          || id.rfind("LAYOUT_POKEMON_MANSION_", 0) == 0))
            return false;
    }

    // The FRLG coast replaces this complete HNS map cluster in Wayfarer.
    // Retain the source maps and stable map/layout IDs for standalone HNS.
    if (version == "wayfarer" && get_source_version(data) == "hns") {
        static const set<string> replaced_hns_ids = {
            "MAP_CINNABAR_ISLAND_HNS", "MAP_CINNABAR_ISLAND_POKEMON_CENTER_HNS",
            "MAP_SEAFOAM_ISLANDS_1F_HNS", "MAP_SEAFOAM_ISLANDS_B1F_HNS",
            "MAP_SEAFOAM_ISLANDS_GYM_HNS", "MAP_SEAFOAM_ISLANDS_SECRET_CAVE_HNS",
            "MAP_ROUTE21_HNS",
            "MAP_ROUTE19_HNS", "MAP_ROUTE20_HNS", "MAP_ROUTE19_CAVE_HNS",
            "MAP_FUCHSIA_ROUTE19GATE_HNS",
            "MAP_CINNABAR_SEAM_POC", "MAP_ROUTE19_COAST_POC", "MAP_ROUTE20_COAST_POC",
            "MAP_ROUTE21_NORTH_COAST_POC", "MAP_ROUTE21_SOUTH_COAST_POC",
            "MAP_SEAFOAM_ISLANDS_1F_COAST_POC", "MAP_SEAFOAM_ISLANDS_B1F_COAST_POC",
            "MAP_SEAFOAM_ISLANDS_B2F_COAST_POC", "MAP_SEAFOAM_ISLANDS_B3F_COAST_POC",
            "MAP_SEAFOAM_ISLANDS_B4F_COAST_POC",
            "LAYOUT_CINNABAR_ISLAND_HNS", "LAYOUT_CINNABAR_ISLAND_POKEMON_CENTER_HNS",
            "LAYOUT_SEAFOAM_ISLANDS_1F_HNS", "LAYOUT_SEAFOAM_ISLANDS_B1F_HNS",
            "LAYOUT_SEAFOAM_ISLANDS_GYM_HNS", "LAYOUT_SEAFOAM_ISLANDS_SECRET_CAVE_HNS",
            "LAYOUT_ROUTE21_HNS",
            "LAYOUT_ROUTE19_HNS", "LAYOUT_ROUTE20_HNS", "LAYOUT_ROUTE19_CAVE_HNS",
            "LAYOUT_FUCHSIA_ROUTE19GATE_HNS",
            "LAYOUT_CINNABAR_SEAM_POC", "LAYOUT_ROUTE19_COAST_POC", "LAYOUT_ROUTE20_COAST_POC",
            "LAYOUT_ROUTE21_NORTH_COAST_POC", "LAYOUT_ROUTE21_SOUTH_COAST_POC",
            "LAYOUT_SEAFOAM_ISLANDS_1F_COAST_POC", "LAYOUT_SEAFOAM_ISLANDS_B1F_COAST_POC",
            "LAYOUT_SEAFOAM_ISLANDS_B2F_COAST_POC", "LAYOUT_SEAFOAM_ISLANDS_B3F_COAST_POC",
            "LAYOUT_SEAFOAM_ISLANDS_B4F_COAST_POC",
        };
        if (replaced_hns_ids.find(json_to_string(data, "id", true)) != replaced_hns_ids.end())
            return false;
        string id = json_to_string(data, "id", true);
        if (id.size() >= 5 && id.substr(id.size() - 5) == "_PORT"
         && (id.rfind("MAP_CINNABAR_ISLAND_", 0) == 0
          || id.rfind("MAP_POKEMON_MANSION_", 0) == 0
          || id.rfind("LAYOUT_CINNABAR_ISLAND_", 0) == 0
          || id.rfind("LAYOUT_POKEMON_MANSION_", 0) == 0))
            return false;
        string name = json_to_string(data, "name", true);
        if (name.rfind("CinnabarIsland_", 0) == 0 && name.size() >= 5
         && name.substr(name.size() - 5) == "_Port")
            return false;
        if (name.rfind("PokemonMansion_", 0) == 0 && name.size() >= 5
         && name.substr(name.size() - 5) == "_Port")
            return false;
    }
    if (version == "wayfarer" && get_source_version(data) == "frlg") {
        if (wayfarer_coast_map_names.count(json_to_string(data, "name", true))
         || wayfarer_coast_layout_ids.count(json_to_string(data, "id", true))
         || wayfarer_anne_map_names.count(json_to_string(data, "name", true))
         || wayfarer_anne_layout_ids.count(json_to_string(data, "id", true)))
            return true;
        if (!wayfarer_sevii_release_link_enabled)
            return false;
        string name = json_to_string(data, "name", true);
        string id = json_to_string(data, "id", true);
        return wayfarer_sevii_enabled_map_names.find(name) != wayfarer_sevii_enabled_map_names.end()
            || wayfarer_sevii_enabled_layout_ids.find(id) != wayfarer_sevii_enabled_layout_ids.end();
    }
    return source_version_is_selected(get_source_version(data));
}

void validate_wayfarer_sinnoh_donor(const Json &manifest, const string &kind) {
    if (manifest["schema_version"].int_value() != 1)
        FATAL_ERROR("Wayfarer Sinnoh %s manifest has unsupported schema version.\n", kind.c_str());
    const Json donor = manifest["donor"];
    if (donor.type() != Json::Type::OBJECT
     || json_to_string(donor, "url", true) != "https://github.com/LiderMorti00/Sinnoh-pokeemerald-expansion"
     || json_to_string(donor, "commit", true) != "4eed17cc63c4ec8c24fbb20fe49e8d65cb4870d8")
        FATAL_ERROR("Wayfarer Sinnoh %s manifest donor identity does not match the frozen baseline.\n", kind.c_str());
}

void validate_wayfarer_sinnoh_selection(const Json &selection, const string &kind) {
    if (selection.type() != Json::Type::OBJECT
     || selection["release_link_enabled"].type() != Json::Type::BOOL
     || selection["asset_manifest_ready"].type() != Json::Type::BOOL
     || selection["blockers"].type() != Json::Type::ARRAY)
        FATAL_ERROR("Wayfarer Sinnoh %s manifest must declare its complete selection gate.\n", kind.c_str());
}

void load_wayfarer_sinnoh_manifest() {
    if (version != "wayfarer" || wayfarer_sinnoh_manifest_path.empty())
        return;
    if (wayfarer_sinnoh_asset_manifest_path.empty())
        FATAL_ERROR("Wayfarer Sinnoh selection requires the checked-in asset manifest.\n");

    string err;
    Json manifest = Json::parse(read_text_file(wayfarer_sinnoh_manifest_path), err);
    if (manifest == Json())
        FATAL_ERROR("Failed to read Wayfarer Sinnoh manifest: %s\n", err.c_str());
    validate_wayfarer_sinnoh_donor(manifest, "map");
    const Json selection = manifest["selection"];
    validate_wayfarer_sinnoh_selection(selection, "map");
    if (manifest["source_groups"].type() != Json::Type::ARRAY
     || manifest["expected_counts"].type() != Json::Type::OBJECT
     || manifest["maps"].type() != Json::Type::ARRAY)
        FATAL_ERROR("Wayfarer Sinnoh map manifest is missing its frozen catalog metadata.\n");
    if (manifest["source_groups"].array_items().size() != wayfarer_sinnoh_source_groups.size()
     || manifest["expected_counts"].object_items().size() != wayfarer_sinnoh_expected_counts.size()
     || manifest["maps"].array_items().size() != static_cast<size_t>(wayfarer_sinnoh_expected_counts.at("maps")))
        FATAL_ERROR("Wayfarer Sinnoh map manifest does not contain the complete frozen catalog.\n");
    for (const auto &expected : wayfarer_sinnoh_expected_counts)
        if (manifest["expected_counts"][expected.first].int_value() != expected.second)
            FATAL_ERROR("Wayfarer Sinnoh map manifest has an unexpected frozen %s count.\n", expected.first.c_str());
    for (size_t index = 0; index < wayfarer_sinnoh_source_groups.size(); index++) {
        const Json group = manifest["source_groups"].array_items()[index];
        if (json_to_string(group, "source_group") != wayfarer_sinnoh_source_groups[index].first
         || json_to_string(group, "target_group") != wayfarer_sinnoh_source_groups[index].first
         || group["order"].int_value() != static_cast<int>(index)
         || group["map_count"].int_value() != wayfarer_sinnoh_source_groups[index].second)
            FATAL_ERROR("Wayfarer Sinnoh map manifest source group %zu is not frozen.\n", index);
    }

    wayfarer_sinnoh_release_link_enabled = selection["release_link_enabled"].bool_value();
    if (wayfarer_sinnoh_release_link_enabled
     && (!selection["asset_manifest_ready"].bool_value()
      || !selection["blockers"].array_items().empty()))
        FATAL_ERROR("Wayfarer Sinnoh content cannot link until the asset manifest is ready and blockers are clear.\n");
    set<string> source_maps;
    set<string> target_map_ids;
    set<string> target_layout_ids;
    set<string> asset_record_ids;
    vector<int> maps_by_group(wayfarer_sinnoh_source_groups.size(), 0);
    int warp_count = 0;
    int connection_count = 0;
    for (size_t index = 0; index < manifest["maps"].array_items().size(); index++) {
        const Json record = manifest["maps"].array_items()[index];
        const string source_map = json_to_string(record, "source_map");
        const string target_map_id = json_to_string(record, "target_map_id");
        const string target_layout = json_to_string(record, "target_layout");
        const string source_group = json_to_string(record, "source_group");
        int group_index = -1;
        for (size_t group = 0; group < wayfarer_sinnoh_source_groups.size(); group++)
            if (source_group == wayfarer_sinnoh_source_groups[group].first)
                group_index = group;
        const Json inclusion = record["inclusion"];
        const string inclusion_state = json_to_string(inclusion, "state");
        if (record["order"].int_value() != static_cast<int>(index)
         || group_index < 0
         || record["source_group_order"].int_value() != maps_by_group[group_index]
         || json_to_string(record, "source_map_id").empty()
         || json_to_string(record, "source_layout").empty()
         || json_to_string(record, "target_map").empty()
         || json_to_string(record, "layout_format") != "emerald"
         || inclusion.type() != Json::Type::OBJECT
         || (inclusion_state != "INCLUDED" && inclusion_state != "FROZEN_NOT_SELECTED"
          && inclusion_state != "EXCLUDED"))
            FATAL_ERROR("Wayfarer Sinnoh map manifest record %zu is not a frozen Emerald-layout map.\n", index);
        if (!source_maps.insert(source_map).second
         || !target_map_ids.insert(target_map_id).second
         || !target_layout_ids.insert(target_layout).second)
            FATAL_ERROR("Wayfarer Sinnoh manifest contains duplicate selected map or layout IDs.\n");
        maps_by_group[group_index]++;
        const Json warps = record["warps"];
        const Json connections = record["connections"];
        const Json empty_content = record["empty_content"];
        if (warps.type() != Json::Type::ARRAY || connections.type() != Json::Type::ARRAY
         || empty_content.type() != Json::Type::OBJECT)
            FATAL_ERROR("Wayfarer Sinnoh map manifest record %zu has incomplete topology data.\n", index);
        warp_count += warps.array_items().size();
        connection_count += connections.array_items().size();
        for (const string key : {"object_events", "coord_events", "bg_events", "map_scripts", "wild_encounter_profiles"})
            if (empty_content[key].int_value() != 0)
                FATAL_ERROR("Wayfarer Sinnoh map manifest record %zu has unexpected authored %s.\n", index, key.c_str());
        const Json assets = record["asset_records"];
        if (assets.type() != Json::Type::OBJECT || assets["tilesets"].type() != Json::Type::ARRAY)
            FATAL_ERROR("Wayfarer Sinnoh map manifest record %zu has no asset authority references.\n", index);
        asset_record_ids.insert(json_to_string(assets, "blockdata"));
        asset_record_ids.insert(json_to_string(assets, "border"));
        for (const Json &asset_id : assets["tilesets"].array_items())
            asset_record_ids.insert(json_to_string(asset_id));
        wayfarer_sinnoh_records.emplace(source_map, record);
        if (wayfarer_sinnoh_record_is_enabled(record)) {
            wayfarer_sinnoh_enabled_map_names.insert(source_map);
            wayfarer_sinnoh_enabled_layout_ids.insert(target_layout);
        }
    }
    for (size_t index = 0; index < maps_by_group.size(); index++)
        if (maps_by_group[index] != wayfarer_sinnoh_source_groups[index].second)
            FATAL_ERROR("Wayfarer Sinnoh map manifest source group %zu has an unexpected map count.\n", index);
    if (warp_count != wayfarer_sinnoh_expected_counts.at("warps")
     || connection_count != wayfarer_sinnoh_expected_counts.at("connections"))
        FATAL_ERROR("Wayfarer Sinnoh map manifest topology counts are not frozen.\n");

    Json assets = Json::parse(read_text_file(wayfarer_sinnoh_asset_manifest_path), err);
    if (assets == Json())
        FATAL_ERROR("Failed to read Wayfarer Sinnoh asset manifest: %s\n", err.c_str());
    validate_wayfarer_sinnoh_donor(assets, "asset");
    const Json asset_selection = assets["selection"];
    validate_wayfarer_sinnoh_selection(asset_selection, "asset");
    if (asset_selection["release_link_enabled"].bool_value() != selection["release_link_enabled"].bool_value()
     || asset_selection["asset_manifest_ready"].bool_value() != selection["asset_manifest_ready"].bool_value()
     || asset_selection["blockers"].dump() != selection["blockers"].dump())
        FATAL_ERROR("Wayfarer Sinnoh map and asset selection gates disagree.\n");
    if (assets["records"].type() != Json::Type::ARRAY)
        FATAL_ERROR("Wayfarer Sinnoh asset manifest has no records.\n");
    set<string> known_asset_records;
    for (const Json &asset : assets["records"].array_items()) {
        const string asset_id = json_to_string(asset, "record_id");
        const string reuse_class = json_to_string(asset, "reuse_class");
        if (!known_asset_records.insert(asset_id).second || asset["selection_blocker"].type() != Json::Type::BOOL)
            FATAL_ERROR("Wayfarer Sinnoh asset manifest has an invalid asset record.\n");
        if (wayfarer_sinnoh_release_link_enabled
         && (reuse_class == "REVIEW_REQUIRED" || asset["selection_blocker"].bool_value()))
            FATAL_ERROR("Wayfarer Sinnoh asset manifest contains unresolved release asset %s.\n", asset_id.c_str());
    }
    for (const string &asset_id : asset_record_ids)
        if (known_asset_records.find(asset_id) == known_asset_records.end())
            FATAL_ERROR("Wayfarer Sinnoh map manifest references unknown asset record %s.\n", asset_id.c_str());
}

void load_wayfarer_sevii_manifest() {
    if (version != "wayfarer" || wayfarer_sevii_manifest_path.empty())
        return;

    string err;
    Json manifest = Json::parse(read_text_file(wayfarer_sevii_manifest_path), err);
    if (manifest == Json())
        FATAL_ERROR("Failed to read Wayfarer Sevii manifest: %s\n", err.c_str());
    if (manifest["schema_version"].int_value() != 2)
        FATAL_ERROR("Wayfarer Sevii manifest has unsupported schema version.\n");
    if (manifest["release_link_enabled"].type() != Json::Type::BOOL)
        FATAL_ERROR("Wayfarer Sevii manifest must declare release_link_enabled.\n");

    wayfarer_sevii_release_link_enabled = manifest["release_link_enabled"].bool_value();
    const Json domains = manifest["content_domains"];
    const map<string, string> domain_owners = {
        {"exploration", "exploration"},
        {"ordinary_trainers", "ordinary_trainer"},
        {"story", "story"},
        {"trainer_tower", "trainer_tower"},
    };
    if (domains.type() != Json::Type::OBJECT || domains.object_items().size() != domain_owners.size())
        FATAL_ERROR("Wayfarer Sevii manifest must declare every content domain.\n");
    for (const auto &domain : domain_owners) {
        const Json entry = domains[domain.first];
        if (entry.type() != Json::Type::OBJECT
         || json_to_string(entry, "owner", true) != domain.second
         || entry["enabled"].type() != Json::Type::BOOL
         || entry["inventory"].type() != Json::Type::ARRAY)
            FATAL_ERROR("Wayfarer Sevii manifest domain %s is invalid.\n", domain.first.c_str());
        wayfarer_sevii_content_domains.emplace(domain.second, entry["enabled"].bool_value());
        for (const Json &content_id : entry["inventory"].array_items()) {
            string value = json_to_string(content_id, "", true);
            if (value.empty() || !wayfarer_sevii_content_inventory.emplace(value, domain.second).second)
                FATAL_ERROR("Wayfarer Sevii manifest has an invalid or duplicate content inventory ID.\n");
        }
    }
    if (!wayfarer_sevii_content_domains["exploration"])
        FATAL_ERROR("Wayfarer Sevii exploration content domain must remain enabled.\n");
    map<string, unsigned int> content_uses;
    for (const Json &entry : manifest["maps"].array_items()) {
        string source_map = json_to_string(entry, "source_map");
        string map_id = json_to_string(entry, "map_id");
        string layout = json_to_string(entry, "layout");
        if (source_map.rfind("BirthIsland_", 0) == 0 || source_map.rfind("NavelRock_", 0) == 0)
            FATAL_ERROR("Wayfarer Sevii manifest cannot select event-island map %s.\n", source_map.c_str());
        if (!wayfarer_sevii_map_names.insert(source_map).second
         || !wayfarer_sevii_map_ids.insert(map_id).second)
            FATAL_ERROR("Wayfarer Sevii manifest contains duplicate map %s.\n", source_map.c_str());
        wayfarer_sevii_layout_ids.insert(layout);
        wayfarer_sevii_records.emplace(source_map, entry);
        const Json retained = entry["retained_events"];
        for (const string event_kind : {"object_events", "coord_events", "bg_events"}) {
            const Json rows = retained[event_kind];
            if (rows.type() != Json::Type::ARRAY)
                FATAL_ERROR("Wayfarer Sevii manifest %s %s must be a list.\n", source_map.c_str(), event_kind.c_str());
            for (const Json &row : rows.array_items()) {
                string content_id = json_to_string(row, "content_id", true);
                if (content_id.empty())
                    FATAL_ERROR("Wayfarer Sevii manifest event has no content_id.\n");
                content_uses[content_id]++;
            }
        }
        for (const Json &row : entry["retained_map_scripts"].array_items()) {
            string content_id = json_to_string(row, "content_id", true);
            if (content_id.empty())
                FATAL_ERROR("Wayfarer Sevii manifest handler has no content_id.\n");
            content_uses[content_id]++;
        }

        // An explicit per-map value narrows a release. Otherwise the frozen
        // manifest follows the reviewed release-wide switch, keeping the
        // initial all-off artifact inert without duplicating 135 booleans.
        bool enabled = entry["enabled"].type() == Json::Type::BOOL
            ? entry["enabled"].bool_value()
            : wayfarer_sevii_release_link_enabled;
        if (enabled) {
            wayfarer_sevii_enabled_map_names.insert(source_map);
            wayfarer_sevii_enabled_map_ids.insert(map_id);
            wayfarer_sevii_enabled_layout_ids.insert(layout);
        }
    }
    for (const auto &item : wayfarer_sevii_content_inventory) {
        if (content_uses[item.first] != 1)
            FATAL_ERROR("Wayfarer Sevii content inventory ID %s must resolve exactly once.\n", item.first.c_str());
    }
}

bool wayfarer_sevii_owner_is_enabled(const Json &rule) {
    string owner = json_to_string(rule, "owner", true);
    auto it = wayfarer_sevii_content_domains.find(owner);
    if (it == wayfarer_sevii_content_domains.end())
        FATAL_ERROR("Wayfarer Sevii event has an untyped owner.\n");
    string content_id = json_to_string(rule, "content_id", true);
    if (content_id.empty() || json_to_string(rule, "reason", true).empty())
        FATAL_ERROR("Wayfarer Sevii event must declare stable content_id and adaptation reason.\n");
    auto inventory = wayfarer_sevii_content_inventory.find(content_id);
    if (inventory == wayfarer_sevii_content_inventory.end() || inventory->second != owner)
        FATAL_ERROR("Wayfarer Sevii event content_id is not owned by its domain inventory.\n");
    return it->second;
}

bool wayfarer_sevii_valid_state_name(const string &state) {
    return state.rfind("SEVII_", 0) == 0;
}

void validate_wayfarer_sevii_event_rule(const Json &event, const Json &rule,
                                        const string &map_name, const string &event_kind,
                                        unsigned int index) {
    if (rule.type() != Json::Type::OBJECT)
        FATAL_ERROR("Wayfarer Sevii %s %s[%u] must record a source event identity.\n",
                    map_name.c_str(), event_kind.c_str(), index);
    if (rule["source"] == Json() || rule["source"].dump() != event.dump())
        FATAL_ERROR("Wayfarer Sevii %s %s[%u] no longer matches its reviewed source event.\n",
                    map_name.c_str(), event_kind.c_str(), index);
    wayfarer_sevii_owner_is_enabled(rule);
    const string owner = json_to_string(rule, "owner", true);
    const Json overrides = rule["overrides"];
    if (overrides != Json() && overrides.type() != Json::Type::OBJECT)
        FATAL_ERROR("Wayfarer Sevii %s %s[%u] overrides must be an object.\n",
                    map_name.c_str(), event_kind.c_str(), index);
    const set<string> allowed = event_kind == "object_events"
        ? set<string>{"script", "flag"}
        : event_kind == "coord_events"
            ? set<string>{"script", "var", "var_value"}
            : set<string>{"script", "flag"};
    for (const auto &override : overrides.object_items()) {
        if (allowed.find(override.first) == allowed.end())
            FATAL_ERROR("Wayfarer Sevii %s %s[%u] has forbidden override %s.\n",
                        map_name.c_str(), event_kind.c_str(), index, override.first.c_str());
        if (override.first == "flag" || override.first == "var") {
            const string value = json_to_string(override.second, "", true);
            const bool always_visible_object = override.first == "flag"
                && event_kind == "object_events" && value == "0";
            const bool tower_transient = owner == "trainer_tower"
                && ((override.first == "flag" && value.rfind("FLAG_TEMP_", 0) == 0)
                 || (override.first == "var" && value.rfind("VAR_TEMP_", 0) == 0));
            const string prefix = override.first == "flag"
                ? "FLAG_WAYFARER_SEVII_" : "VAR_WAYFARER_SEVII_";
            if (!always_visible_object && !tower_transient && value.rfind(prefix, 0) != 0)
                FATAL_ERROR("Wayfarer Sevii %s %s[%u] override %s must use the Sevii namespace.\n",
                            map_name.c_str(), event_kind.c_str(), index, override.first.c_str());
        }
        if (override.first == "flag" && event_kind == "bg_events"
         && json_to_string(event, "type", true) != "hidden_item")
            FATAL_ERROR("Wayfarer Sevii %s bg_events[%u] may override a flag only for a hidden item.\n",
                        map_name.c_str(), index);
    }
    const string replacement_script = json_to_string(rule, "wayfarer_script", true);
    if (!replacement_script.empty() && replacement_script.rfind("WayfarerSevii_", 0) != 0
     && !(json_to_string(rule, "owner", true) == "exploration" && replacement_script == "EventScript_StrengthBoulder"))
        FATAL_ERROR("Wayfarer Sevii %s %s[%u] must use a Wayfarer-owned replacement script.\n",
                    map_name.c_str(), event_kind.c_str(), index);
    if (owner != "exploration") {
        const string field = event_kind == "coord_events" ? "var" : "flag";
        string effective = overrides[field] == Json()
            ? json_to_string(event, field, true) : json_to_string(overrides, field, true);
        const string prefix = field == "var" ? "VAR_WAYFARER_SEVII_" : "FLAG_WAYFARER_SEVII_";
        const bool tower_transient = owner == "trainer_tower"
            && ((field == "flag" && effective.rfind("FLAG_TEMP_", 0) == 0)
             || (field == "var" && effective.rfind("VAR_TEMP_", 0) == 0));
        if (!effective.empty() && effective != "0" && effective != "0x0"
         && effective.rfind(prefix, 0) != 0 && !tower_transient)
            FATAL_ERROR("Wayfarer Sevii %s %s[%u] retains raw FRLG persistent state.\n",
                        map_name.c_str(), event_kind.c_str(), index);
    }
    for (const string state_field : {"state_reads", "state_writes"}) {
        const Json states = rule[state_field];
        if (states == Json())
            continue;
        if (states.type() != Json::Type::ARRAY)
            FATAL_ERROR("Wayfarer Sevii %s %s[%u] %s must be a list.\n",
                        map_name.c_str(), event_kind.c_str(), index, state_field.c_str());
        for (const Json &state : states.array_items()) {
            if (!wayfarer_sevii_valid_state_name(json_to_string(state, "", true)))
                FATAL_ERROR("Wayfarer Sevii %s %s[%u] writes or reads raw FRLG state.\n",
                            map_name.c_str(), event_kind.c_str(), index);
        }
    }
}

void validate_wayfarer_sevii_map_event_rules(const Json &map_data, const Json &record) {
    const string map_name = json_to_string(map_data, "name");
    const Json retained = record["retained_events"];
    set<string> local_ids, coordinates;
    for (const string event_kind : {"object_events", "coord_events", "bg_events"}) {
        const Json rules = retained[event_kind];
        if (rules.type() != Json::Type::ARRAY)
            FATAL_ERROR("Wayfarer Sevii %s %s must be a list.\n", map_name.c_str(), event_kind.c_str());
        set<unsigned int> indices;
        const Json events = map_data[event_kind];
        for (const Json &rule : rules.array_items()) {
            int index = rule["index"].int_value();
            if (rule.type() != Json::Type::OBJECT || index < 0
             || (unsigned int)index >= events.array_items().size())
                FATAL_ERROR("Wayfarer Sevii %s %s has an invalid source index.\n",
                            map_name.c_str(), event_kind.c_str());
            if (!indices.insert((unsigned int)index).second)
                FATAL_ERROR("Wayfarer Sevii %s %s has duplicate source ownership.\n",
                            map_name.c_str(), event_kind.c_str());
            validate_wayfarer_sevii_event_rule(events.array_items()[index], rule, map_name, event_kind, index);
            const Json event = events.array_items()[index];
            if (event_kind == "object_events") {
                string local_id = json_to_string(event, "local_id", true);
                if (!local_id.empty() && !local_ids.insert(local_id).second)
                    FATAL_ERROR("Wayfarer Sevii %s has duplicate local_id ownership %s.\n",
                                map_name.c_str(), local_id.c_str());
            } else {
                string coordinate = event_kind + ":" + json_to_string(event, "x", true) + ":"
                    + json_to_string(event, "y", true) + ":" + json_to_string(event, "elevation", true);
                if (!coordinates.insert(coordinate).second)
                    FATAL_ERROR("Wayfarer Sevii %s has duplicate %s ownership.\n",
                                map_name.c_str(), event_kind.c_str());
            }
        }
    }
}

bool is_registered_wayfarer_sevii_map(const Json &map_data) {
    return version == "wayfarer"
        && get_source_version(map_data) == "frlg"
        && wayfarer_sevii_records.find(json_to_string(map_data, "name", true))
            != wayfarer_sevii_records.end();
}

Json wayfarer_sevii_record(const Json &map_data) {
    auto it = wayfarer_sevii_records.find(json_to_string(map_data, "name", true));
    return it == wayfarer_sevii_records.end() ? Json() : it->second;
}

bool is_wayfarer_sevii_enabled_destination(const string &map_id) {
    return map_id == "MAP_DYNAMIC" || map_id == "MAP_UNDEFINED"
        || wayfarer_sevii_enabled_map_ids.find(map_id) != wayfarer_sevii_enabled_map_ids.end();
}

bool retained_event_matches_index(const Json &entries, unsigned int index, Json *rule) {
    for (const Json &entry : entries.array_items()) {
        if (entry.type() == Json::Type::OBJECT && entry["index"].int_value() == (int)index) {
            if (!wayfarer_sevii_owner_is_enabled(entry))
                return false;
            if (rule != nullptr)
                *rule = entry;
            return true;
        }
    }
    return false;
}

Json apply_wayfarer_sevii_event_rule(const Json &event, const Json &rule,
                                     const string &map_name, const string &event_kind,
                                     unsigned int index) {
    Json::object output = event.object_items();
    string source_script = json_to_string(event, "script", true);
    string replacement_script = json_to_string(rule, "wayfarer_script", true);
    if (source_script != "" && source_script != "0" && source_script != "0x0" && source_script != "NULL") {
        if (replacement_script.empty())
            FATAL_ERROR("Wayfarer Sevii %s %s[%u] must name a Wayfarer-owned replacement script.\n",
                        map_name.c_str(), event_kind.c_str(), index);
        output["script"] = replacement_script;
    }
    const Json overrides = rule["overrides"];
    for (const auto &override : overrides.object_items()) {
        if (override.first == "script") {
            if (!replacement_script.empty() && json_to_string(override.second, "", true) != replacement_script)
                FATAL_ERROR("Wayfarer Sevii %s %s[%u] override script must match wayfarer_script.\n",
                            map_name.c_str(), event_kind.c_str(), index);
            output["script"] = override.second;
        } else {
            output[override.first] = override.second;
        }
    }
    return output;
}

Json sanitize_wayfarer_sevii_map_events(const Json &map_data) {
    if (!is_registered_wayfarer_sevii_map(map_data))
        return map_data;

    const string map_name = json_to_string(map_data, "name");
    const Json record = wayfarer_sevii_record(map_data);
    const Json retained = record["retained_events"];
    validate_wayfarer_sevii_map_event_rules(map_data, record);
    Json::object output = map_data.object_items();
    Json::array objects, warps, coords, bgs;

    // Objects, coordinate triggers, and background events are opt-in only.
    // This keeps story actors, Trainers, items, and signs out unless a later
    // milestone records the exact source event and supplies a replacement.
    for (unsigned int i = 0; i < map_data["object_events"].array_items().size(); i++) {
        const Json event = map_data["object_events"].array_items()[i];
        Json rule;
        if (!retained_event_matches_index(retained["object_events"], i, &rule))
            continue;
        string trainer_type = json_to_string(event, "trainer_type", true);
        if (!trainer_type.empty() && trainer_type != "TRAINER_TYPE_NONE"
         && json_to_string(rule, "owner", true) == "exploration")
            FATAL_ERROR("Wayfarer Sevii %s object_events[%u] is a Trainer and cannot be retained.\n",
                        map_name.c_str(), i);
        objects.push_back(apply_wayfarer_sevii_event_rule(event, rule, map_name, "object_events", i));
    }

    // Internal map warps are geometry, not story content. Keep only targets
    // that are enabled in this release; the FRLG Union Room and Trade Center
    // are intentionally absent, so the Pokemon Center 2F doors disappear.
    for (const Json &event : map_data["warp_events"].array_items()) {
        if (is_wayfarer_sevii_enabled_destination(json_to_string(event, "dest_map")))
            warps.push_back(event);
    }

    for (unsigned int i = 0; i < map_data["coord_events"].array_items().size(); i++) {
        const Json event = map_data["coord_events"].array_items()[i];
        Json rule;
        if (retained_event_matches_index(retained["coord_events"], i, &rule))
            coords.push_back(apply_wayfarer_sevii_event_rule(event, rule, map_name, "coord_events", i));
    }
    for (unsigned int i = 0; i < map_data["bg_events"].array_items().size(); i++) {
        const Json event = map_data["bg_events"].array_items()[i];
        Json rule;
        if (retained_event_matches_index(retained["bg_events"], i, &rule))
            bgs.push_back(apply_wayfarer_sevii_event_rule(event, rule, map_name, "bg_events", i));
    }

    output["object_events"] = objects;
    output["warp_events"] = warps;
    output["coord_events"] = coords;
    output["bg_events"] = bgs;
    return output;
}

Json resolve_wayfarer_coast_map_events(Json map_data) {
    map_data = sanitize_wayfarer_sevii_map_events(map_data);
    if (version != "wayfarer")
        return map_data;

    const string name = json_to_string(map_data, "name");
    if (wayfarer_anne_map_names.count(name)) {
        static const map<string, string> flags = {
            {"FLAG_HIDE_SSANNE_1F_ROOM2_TM31", "FLAG_WAYFARER_SS_ANNE_ITEM_TM31"},
            {"FLAG_HIDE_SSANNE_2F_ROOM2_STARDUST", "FLAG_WAYFARER_SS_ANNE_ITEM_STARDUST"},
            {"FLAG_HIDE_SSANNE_2F_ROOM4_X_ATTACK", "FLAG_WAYFARER_SS_ANNE_ITEM_X_ATTACK"},
            {"FLAG_HIDE_SSANNE_B1F_ROOM2_TM44", "FLAG_WAYFARER_SS_ANNE_ITEM_TM44"},
            {"FLAG_HIDE_SSANNE_B1F_ROOM3_ETHER", "FLAG_WAYFARER_SS_ANNE_ITEM_ETHER"},
            {"FLAG_HIDE_SSANNE_B1F_ROOM5_SUPER_POTION", "FLAG_WAYFARER_SS_ANNE_ITEM_SUPER_POTION"},
            {"FLAG_HIDE_SSANNE_KITCHEN_GREAT_BALL", "FLAG_WAYFARER_SS_ANNE_ITEM_GREAT_BALL"},
            {"FLAG_HIDDEN_ITEM_SSANNE_B1F_CORRIDOR_HYPER_POTION", "FLAG_WAYFARER_SS_ANNE_ITEM_HYPER_POTION"},
            {"FLAG_HIDDEN_ITEM_SSANNE_KITCHEN_CHESTO_BERRY", "FLAG_WAYFARER_SS_ANNE_ITEM_CHESTO_BERRY"},
            {"FLAG_HIDDEN_ITEM_SSANNE_KITCHEN_PECHA_BERRY", "FLAG_WAYFARER_SS_ANNE_ITEM_PECHA_BERRY"},
            {"FLAG_HIDDEN_ITEM_SSANNE_KITCHEN_CHERI_BERRY", "FLAG_WAYFARER_SS_ANNE_ITEM_CHERI_BERRY"},
            {"FLAG_HIDE_SS_ANNE_RIVAL", "FLAG_WAYFARER_SS_ANNE_HIDE_BLUE"},
        };
        Json::object output = map_data.object_items();
        auto remap = [](Json::object event) {
            auto flag = event.find("flag");
            if (flag != event.end() && flag->second.type() == Json::Type::STRING) {
                auto found = flags.find(flag->second.string_value());
                if (found != flags.end()) flag->second = found->second;
            }
            return event;
        };
        Json::array objects, bgs;
        Json::array coords;
        for (const Json &event : map_data["object_events"].array_items())
            objects.push_back(remap(event.object_items()));
        for (const Json &event : map_data["bg_events"].array_items())
            bgs.push_back(remap(event.object_items()));
        for (const Json &event : map_data["coord_events"].array_items()) {
            Json::object coord = event.object_items();
            if (json_to_string(event, "var", true) == "VAR_MAP_SCENE_S_S_ANNE_2F_CORRIDOR")
                coord["var"] = "VAR_WAYFARER_SS_ANNE_BLUE_SCENE";
            coords.push_back(coord);
        }
        output["object_events"] = objects;
        output["bg_events"] = bgs;
        output["coord_events"] = coords;
        return output;
    }
    if (name == "ViridianCity_hns") {
        Json::object output = map_data.object_items();
        Json::array objects;
        for (const Json &source : map_data["object_events"].array_items()) {
            Json::object object = source.object_items();
            if (json_to_string(source, "flag", true) == "FLAG_HIDE_VIRIDIAN_BLUE_INTRO")
                object["flag"] = "FLAG_WAYFARER_HIDE_VIRIDIAN_BLUE_INTRO_COAST";
            objects.push_back(object);
        }
        output["object_events"] = objects;
        return output;
    }
    if (name == "Route25_BillsHouse_hns") {
        // Keep the HNS grandfather and regional-form service intact; Bill is
        // an additional Wayfarer-only actor on a walkable interior tile. Use
        // a fixed sprite because arrange/warp fixtures enter after transition
        // scripts have run and must still instantiate the actor consistently.
        Json::object output = map_data.object_items();
        Json::array objects = map_data["object_events"].array_items();
        objects.push_back(Json::object{
            {"type", "object"}, {"local_id", 5}, {"graphics_id", "OBJ_EVENT_GFX_BILL_HNS"},
            {"x", 4}, {"y", 6}, {"elevation", 0},
            {"movement_type", "MOVEMENT_TYPE_FACE_RIGHT"}, {"movement_range_x", 0},
            {"movement_range_y", 0}, {"trainer_type", "TRAINER_TYPE_NONE"},
            {"trainer_sight_or_berry_tree_id", "0"},
            {"script", "Route25_BillsHouse_EventScript_Bill"}, {"flag", "0"},
        });
        output["object_events"] = objects;
        return output;
    }
    if (!wayfarer_coast_map_names.count(name))
        return map_data;

    auto coast_flag = [](const string &flag) -> string {
        if (flag == "FLAG_HIDE_ARTICUNO") return "FLAG_WAYFARER_HIDE_ARTICUNO";
        if (flag == "FLAG_HIDE_POKEMON_MANSION_B1F_SECRET_KEY") return "FLAG_WAYFARER_CINNABAR_SECRET_KEY";
        const vector<std::pair<string, string>> prefixes = {
            {"FLAG_HIDE_SEAFOAM_ISLANDS_", "FLAG_WAYFARER_HIDE_SEAFOAM_"},
            {"FLAG_HIDE_SEAFOAM_", "FLAG_WAYFARER_HIDE_SEAFOAM_"},
            {"FLAG_HIDDEN_ITEM_SEAFOAM_ISLANDS_", "FLAG_WAYFARER_HIDDEN_ITEM_SEAFOAM_"},
            {"FLAG_HIDE_POKEMON_MANSION_", "FLAG_WAYFARER_HIDE_POKEMON_MANSION_"},
            {"FLAG_HIDDEN_ITEM_POKEMON_MANSION_", "FLAG_WAYFARER_HIDDEN_ITEM_POKEMON_MANSION_"},
            {"FLAG_HIDDEN_ITEM_ROUTE20_", "FLAG_WAYFARER_HIDDEN_ITEM_ROUTE20_"},
            {"FLAG_HIDDEN_ITEM_ROUTE21_NORTH_", "FLAG_WAYFARER_HIDDEN_ITEM_ROUTE21_NORTH_"},
        };
        for (const auto &prefix : prefixes) {
            if (flag.rfind(prefix.first, 0) == 0)
                return prefix.second + flag.substr(prefix.first.size());
        }
        return flag;
    };

    auto update_event = [&coast_flag](Json::object event) -> Json::object {
        for (const string field : {"flag", "trainer_type"}) {
            auto it = event.find(field);
            if (it != event.end() && it->second.type() == Json::Type::STRING)
                it->second = coast_flag(it->second.string_value());
        }
        return event;
    };

    Json::array objects;
    for (const Json &source : map_data["object_events"].array_items()) {
        const string local_id = json_to_string(source, "local_id", true);
        if (name == "CinnabarIsland_Frlg"
         && (local_id == "LOCALID_CINNABAR_BILL" || local_id == "LOCALID_CINNABAR_SEAGALLOP"))
            continue;
        if (name == "CinnabarIsland_PokemonCenter_1F_Frlg"
         && local_id == "LOCALID_CINNABAR_POKEMON_CENTER_BILL")
            continue;
        if (name == "Route21_North_Frlg" && json_to_string(source, "type", true) == "clone") {
            // HNS Pallet's boundary actor is a different local ID from FRLG's.
            // Render the boundary actor without cloning the wrong NPC.
            objects.push_back(Json::object{
                {"type", "object"}, {"graphics_id", "OBJ_EVENT_GFX_FAT_MAN_HNS"},
                {"x", 19}, {"y", -3}, {"elevation", 0},
                {"movement_type", "MOVEMENT_TYPE_LOOK_AROUND"},
                {"movement_range_x", 0}, {"movement_range_y", 0},
                {"trainer_type", "TRAINER_TYPE_NONE"},
                {"trainer_sight_or_berry_tree_id", "0"},
                {"script", "PalletTown_EventScript_Fatman"}, {"flag", "0"},
            });
            continue;
        }
        Json::object object = update_event(source.object_items());
        if (name == "CinnabarIsland_PokemonCenter_2F_Frlg") {
            const string script = json_to_string(source, "script", true);
            if (script == "Common_EventScript_UnionRoomAttendant")
                object["script"] = "CinnabarIsland_PokemonCenter_2F_EventScript_UnionRoomAttendant";
            else if (script == "Common_EventScript_WirelessClubAttendant")
                object["script"] = "CinnabarIsland_PokemonCenter_2F_EventScript_WirelessClubAttendant";
            else if (script == "Common_EventScript_DirectCornerAttendant")
                object["script"] = "CinnabarIsland_PokemonCenter_2F_EventScript_DirectCornerAttendant";
            else if (script == "CableClub_EventScript_MysteryGiftMan_Frlg")
                object["script"] = "CinnabarIsland_PokemonCenter_2F_EventScript_MysteryGiftMan";
        }
        objects.push_back(object);
    }
    Json::object output = map_data.object_items();
    output["object_events"] = objects;
    Json::array coords;
    for (const Json &source : map_data["coord_events"].array_items()) {
        Json::object event = update_event(source.object_items());
        if (name == "SeafoamIslands_B4F_Frlg"
         && json_to_string(source, "var", true) == "VAR_MAP_SCENE_SEAFOAM_ISLANDS_B4F")
            event["var"] = "VAR_WAYFARER_SEAFOAM_B4F_CURRENT_ENTRY";
        coords.push_back(event);
    }
    // The FRLG Lab tileset does not mark its interior doorway tiles as warp
    // behaviors. Coordinate triggers make the six authored door positions
    // traversable without changing the source layouts or standalone FRLG.
    auto add_lab_door = [&coords](int x, int y, const string &script) {
        coords.push_back(Json::object{
            {"type", "trigger"}, {"x", x}, {"y", y}, {"elevation", 0},
            {"var", "VAR_TEMP_0"}, {"var_value", "0"}, {"script", script},
        });
    };
    if (name == "CinnabarIsland_PokemonLab_Entrance_Frlg") {
        add_lab_door(13, 5, "WayfarerCinnabar_Lab_WarpLounge");
        add_lab_door(19, 5, "WayfarerCinnabar_Lab_WarpResearch");
        add_lab_door(25, 5, "WayfarerCinnabar_Lab_WarpExperiment");
    } else if (name == "CinnabarIsland_PokemonLab_Lounge_Frlg") {
        add_lab_door(7, 9, "WayfarerCinnabar_Lab_WarpFromLounge");
    } else if (name == "CinnabarIsland_PokemonLab_ResearchRoom_Frlg") {
        add_lab_door(7, 9, "WayfarerCinnabar_Lab_WarpFromResearch");
    } else if (name == "CinnabarIsland_PokemonLab_ExperimentRoom_Frlg") {
        add_lab_door(7, 9, "WayfarerCinnabar_Lab_WarpFromExperiment");
    }
    output["coord_events"] = coords;
    Json::array backgrounds;
    for (const Json &source : map_data["bg_events"].array_items())
        backgrounds.push_back(update_event(source.object_items()));
    output["bg_events"] = backgrounds;
    if (name == "CinnabarIsland_PokemonCenter_2F_Frlg") {
        // The FRLG cable rooms are outside Wayfarer's selected catalog.
        // Attendants on this floor give an explicit service response instead.
        Json::array exits;
        for (const Json &warp : map_data["warp_events"].array_items()) {
            if (json_to_string(warp, "dest_map") == "MAP_CINNABAR_ISLAND_POKEMON_CENTER_1F")
                exits.push_back(warp);
        }
        output["warp_events"] = exits;
    }
    return output;
}

Json sanitize_wayfarer_sevii_map_connections(const Json &map_data) {
    if (!is_registered_wayfarer_sevii_map(map_data))
        return map_data;
    Json::object output = map_data.object_items();
    Json::array connections;
    for (const Json &connection : map_data["connections"].array_items()) {
        if (is_wayfarer_sevii_enabled_destination(json_to_string(connection, "map")))
            connections.push_back(connection);
    }
    output["connections"] = connections;
    return output;
}

Json resolve_wayfarer_coast_connections(Json map_data) {
    map_data = sanitize_wayfarer_sevii_map_connections(map_data);
    if (version != "wayfarer")
        return map_data;

    const string name = json_to_string(map_data, "name");
    Json::array connections;
    for (const Json &source : map_data["connections"].array_items()) {
        Json::object connection = source.object_items();
        const string destination = json_to_string(source, "map");
        if (name == "FuchsiaCity_hns" && destination == "MAP_ROUTE19_COAST_POC")
            connection["map"] = "MAP_ROUTE19";
        else if (name == "PalletTown_hns" && destination == "MAP_ROUTE21_NORTH_COAST_POC")
            connection["map"] = "MAP_ROUTE21_NORTH";
        else if (name == "Route19_Frlg" && destination == "MAP_FUCHSIA_CITY")
            connection["map"] = "MAP_FUCHSIA_CITY_HNS";
        else if (name == "Route21_North_Frlg" && destination == "MAP_PALLET_TOWN")
            connection["map"] = "MAP_PALLET_TOWN_HNS";
        connections.push_back(connection);
    }
    Json::object output = map_data.object_items();
    if (map_data["connections"].type() == Json::Type::ARRAY)
        output["connections"] = connections;
    return output;
}

string get_generated_warning(const string &filename, bool isAsm) {
    string comment = isAsm ? "@" : "//";

    ostringstream warning;
    warning << comment << "\n"
            << comment << " DO NOT MODIFY THIS FILE! It is auto-generated from " << filename << "\n"
            << comment << "\n\n";
    return warning.str();
}

string get_include_guard_start(const string &name) {
    ostringstream guard;
    guard << "#ifndef GUARD_" << name << "_H\n"
          << "#define GUARD_" << name << "_H\n\n";
    return guard.str();
}

string get_include_guard_end(const string &name) {
    ostringstream guard;
    guard << "#endif // GUARD_" << name << "_H\n";
    return guard.str();
}

string generate_map_header_text(Json map_data, Json layouts_data) {
    Json effective_map_data = resolve_wayfarer_coast_connections(map_data);
    string map_layout_id = json_to_string(map_data, "layout");

    vector<Json> matched;

    for (auto &layout : layouts_data["layouts"].array_items()) {
        if (map_layout_id == json_to_string(layout, "id", true))
            matched.push_back(layout);
    }

    if (matched.size() != 1)
        FATAL_ERROR("Failed to find matching layout for %s.\n", map_layout_id.c_str());

    Json layout = matched[0];

    if (data_matches_version(map_data) && !data_matches_version(layout))
        FATAL_ERROR("Layout %s is not available in the %s map catalog.\n", map_layout_id.c_str(), version.c_str());

    if (data_matches_version(map_data)
     && (!std::filesystem::exists(json_to_string(layout, "border_filepath"))
      || !std::filesystem::exists(json_to_string(layout, "blockdata_filepath"))))
        FATAL_ERROR("Layout %s references missing blockdata or border data.\n", map_layout_id.c_str());

    ostringstream text;

    string mapName = json_to_string(map_data, "name");
    text << get_generated_warning("data/maps/" + mapName + "/map.json", true);

    text << mapName << ":\n"
         << "\t.4byte " << json_to_string(layout, "name") << "\n";

    if (map_data.object_items().find("shared_events_map") != map_data.object_items().end())
        text << "\t.4byte " << json_to_string(map_data, "shared_events_map") << "_MapEvents\n";
    else
        text << "\t.4byte " << mapName << "_MapEvents\n";

    if (version == "wayfarer" && mapName == "Route19_Frlg")
        text << "\t.4byte WayfarerKantoCoast_Route19_MapScripts\n";
    else if (version == "wayfarer" && mapName == "Route20_Frlg")
        text << "\t.4byte WayfarerKantoCoast_Route20_MapScripts\n";
    else if (version == "wayfarer" && mapName == "Route21_North_Frlg")
        text << "\t.4byte WayfarerKantoCoast_Route21_North_MapScripts\n";
    else if (version == "wayfarer" && mapName == "Route21_South_Frlg")
        text << "\t.4byte WayfarerKantoCoast_Route21_South_MapScripts\n";
    else if (get_source_version(map_data) == "sinnoh")
        // The frozen donor catalog has no map-script tables.  A null pointer
        // is the complete representation; do not manufacture 133 empty labels
        // merely to satisfy the ordinary per-map script convention.
        text << "\t.4byte NULL\n";
    else if (map_data.object_items().find("shared_scripts_map") != map_data.object_items().end())
        text << "\t.4byte " << json_to_string(map_data, "shared_scripts_map") << "_MapScripts\n";
    else
        text << "\t.4byte " << mapName << "_MapScripts\n";

    if (effective_map_data.object_items().find("connections") != effective_map_data.object_items().end()
     && effective_map_data["connections"].array_items().size() > 0 && json_to_string(map_data, "connections_no_include", true) != "TRUE")
        text << "\t.4byte " << mapName << "_MapConnections\n";
    else
        text << "\t.4byte NULL\n";

    text << "\t.2byte " << json_to_string(map_data, "music") << "\n"
         << "\t.2byte " << json_to_string(layout, "id") << "\n"
         << "\t.byte "  << json_to_string(map_data, "region_map_section") << "\n"
         << "\t.byte "  << json_to_string(map_data, "requires_flash") << "\n"
         << "\t.byte "  << json_to_string(map_data, "weather") << "\n"
         << "\t.byte "  << json_to_string(map_data, "map_type") << "\n";

    if (version != "firered")
        text << "\t.2byte 0\n";

    if (version == "ruby")
        text << "\t.byte " << json_to_string(map_data, "show_map_name") << "\n";
    else if (version == "emerald" || version == "firered" || version == "hns" || version == "wayfarer")
        text << "\tmap_header_flags "
             << "allow_cycling=" << json_to_string(map_data, "allow_cycling") << ", "
             << "allow_escaping=" << json_to_string(map_data, "allow_escaping") << ", "
             << "allow_running=" << json_to_string(map_data, "allow_running") << ", "
             << "show_map_name=" << json_to_string(map_data, "show_map_name") << "\n";

    if (version == "firered")
        text << "\t.byte " << json_to_string(map_data, "floor_number") << "\n";

     text << "\t.byte " << json_to_string(map_data, "battle_scene") << "\n\n";

    return text.str();
}

vector<string> get_existing_maps() {
    vector<string> v = {};
    string map_constants = read_text_file("include/constants/map_groups.h");

    std::regex map_regex("(MAP_\\w+)\\s+=\\s+\\(\\d+");

    for (std::smatch sm; regex_search(map_constants, sm, map_regex);)
    {
        v.push_back(sm[1]);
        map_constants = sm.suffix();
    }
    return v;
}

string generate_map_connections_text(Json map_data) {
    map_data = resolve_wayfarer_coast_connections(map_data);
    if (map_data["connections"] == Json())
        return string("\n");

    string mapName = json_to_string(map_data, "name");

    vector<string> existing_maps = get_existing_maps();
    ostringstream text;
    text << get_generated_warning("data/maps/" + mapName + "/map.json", true);
    text << mapName << "_MapConnectionsList:\n";

    for (auto &connection : map_data["connections"].array_items()) {
        auto it = find(existing_maps.begin(), existing_maps.end(), json_to_string(connection, "map"));
        if (it == existing_maps.end())
            continue;
        text << "\tconnection "
             << json_to_string(connection, "direction") << ", "
             << json_to_string(connection, "offset") << ", "
             << json_to_string(connection, "map") << "\n";
    }

    text << "\n" << mapName << "_MapConnections:\n"
         << "\t.4byte " << map_data["connections"].array_items().size() << "\n"
         << "\t.4byte " << mapName << "_MapConnectionsList\n\n";

    return text.str();
}

Json resolve_wayfarer_coast_warp(const Json &map_data, const Json &warp, size_t index) {
    if (version != "wayfarer")
        return warp;

    string map_name = json_to_string(map_data, "name");
    if (map_name == "SSAnne_1F_Corridor_Frlg" && (index == 2 || index == 3)) {
        Json::object resolved = warp.object_items();
        // The load script sets this dynamic return to the dock's walkable
        // (8, 9) tile. Keeping both exits dynamic avoids an exterior map.
        resolved["dest_map"] = "MAP_DYNAMIC";
        return resolved;
    }
    if (map_name == "Route20_hns" && index < 2) {
        const string expected_x = index == 0 ? "60" : "72";
        const string expected_y = index == 0 ? "8" : "14";
        const string expected_warp = index == 0 ? "0" : "3";
        if (json_to_string(warp, "x") != expected_x
         || json_to_string(warp, "y") != expected_y
         || json_to_string(warp, "dest_map") != "MAP_SEAFOAM_ISLANDS_1F_HNS"
         || json_to_string(warp, "dest_warp_id") != expected_warp)
            FATAL_ERROR("Route20_hns Seafoam entrance %zu no longer matches its HNS source warp.\n", index);

        Json::object resolved = warp.object_items();
        resolved["dest_map"] = "MAP_SEAFOAM_ISLANDS_1F_COAST_POC";
        resolved["dest_warp_id"] = index == 0 ? "3" : "4";
        return resolved;
    }
    if (map_name == "SeafoamIslands_1F_CoastPoc" && (index == 3 || index == 4)) {
        const string expected_x = index == 3 ? "6" : "32";
        const string expected_warp = index == 3 ? "0" : "1";
        if (json_to_string(warp, "x") != expected_x
         || json_to_string(warp, "y") != "21"
         || json_to_string(warp, "dest_map") != "MAP_ROUTE20_COAST_POC"
         || json_to_string(warp, "dest_warp_id") != expected_warp)
            FATAL_ERROR("SeafoamIslands_1F_CoastPoc exit %zu no longer matches its source warp.\n", index);

        Json::object resolved = warp.object_items();
        resolved["dest_map"] = "MAP_ROUTE20_HNS";
        // FRLG's elevation 15 wildcard is a literal height in HNS. These
        // exit tiles are at elevation 3 in the copied 1F layout.
        resolved["elevation"] = 3;
        return resolved;
    }
    return warp;
}

string generate_map_events_text(Json map_data) {
    map_data = resolve_wayfarer_coast_map_events(map_data);
    if (map_data.object_items().find("shared_events_map") != map_data.object_items().end())
        return string("\n");

    string mapName = json_to_string(map_data, "name");

    ostringstream text;
    text << get_generated_warning("data/maps/" + mapName + "/map.json", true);
    text << "\t.align 2\n\n";

    string objects_label, warps_label, coords_label, bgs_label;

    if (map_data["object_events"].array_items().size() > 0) {
        objects_label = mapName + "_ObjectEvents";
        text << objects_label << ":\n";
        for (unsigned int i = 0; i < map_data["object_events"].array_items().size(); i++) {
            auto obj_event = map_data["object_events"].array_items()[i];
            string type = json_to_string(obj_event, "type", true);

            // If no type field is present, assume it's a regular object event.
            if (type == "" || type == "object") {
                text << "\tobject_event " << i + 1 << ", "
                     << json_to_string(obj_event, "graphics_id") << ", "
                     << json_to_string(obj_event, "x") << ", "
                     << json_to_string(obj_event, "y") << ", "
                     << json_to_string(obj_event, "elevation") << ", "
                     << json_to_string(obj_event, "movement_type") << ", "
                     << json_to_string(obj_event, "movement_range_x") << ", "
                     << json_to_string(obj_event, "movement_range_y") << ", "
                     << json_to_string(obj_event, "trainer_type") << ", "
                     << json_to_string(obj_event, "trainer_sight_or_berry_tree_id") << ", "
                     << json_to_string(obj_event, "script") << ", "
                     << json_to_string(obj_event, "flag") << "\n";
            } else if (type == "clone") {
                text << "\tclone_event " << i + 1 << ", "
                     << json_to_string(obj_event, "graphics_id") << ", "
                     << json_to_string(obj_event, "x") << ", "
                     << json_to_string(obj_event, "y") << ", "
                     << json_to_string(obj_event, "target_local_id") << ", "
                     << json_to_string(obj_event, "target_map") << "\n";
            } else {
                FATAL_ERROR("Unknown object event type '%s'. Expected 'object' or 'clone'.\n", type.c_str());
            }
        }
        text << "\n";
    } else {
        objects_label = "NULL";
    }

    if (map_data["warp_events"].array_items().size() > 0) {
        warps_label = mapName + "_MapWarps";
        text << warps_label << ":\n";
        size_t warp_index = 0;
        for (auto &source_warp : map_data["warp_events"].array_items()) {
            Json warp_event = resolve_wayfarer_coast_warp(map_data, source_warp, warp_index++);
            text << "\twarp_def "
                 << json_to_string(warp_event, "x") << ", "
                 << json_to_string(warp_event, "y") << ", "
                 << json_to_string(warp_event, "elevation") << ", "
                 << json_to_string(warp_event, "dest_warp_id") << ", "
                 << json_to_string(warp_event, "dest_map") << "\n";
        }
        text << "\n";
    } else {
        warps_label = "NULL";
    }

    if (map_data["coord_events"].array_items().size() > 0) {
        coords_label = mapName + "_MapCoordEvents";
        text << coords_label << ":\n";
        for (auto &coord_event : map_data["coord_events"].array_items()) {
            string type = json_to_string(coord_event, "type");
            if (type == "trigger") {
                text << "\tcoord_event "
                     << json_to_string(coord_event, "x") << ", "
                     << json_to_string(coord_event, "y") << ", "
                     << json_to_string(coord_event, "elevation") << ", "
                     << json_to_string(coord_event, "var") << ", "
                     << json_to_string(coord_event, "var_value") << ", "
                     << json_to_string(coord_event, "script") << "\n";
            }
            else if (type == "weather") {
                text << "\tcoord_weather_event "
                     << json_to_string(coord_event, "x") << ", "
                     << json_to_string(coord_event, "y") << ", "
                     << json_to_string(coord_event, "elevation") << ", "
                     << json_to_string(coord_event, "weather") << "\n";
            } else {
                FATAL_ERROR("Unknown coord event type '%s'. Expected 'trigger' or 'weather'.\n", type.c_str());
            }
        }
        text << "\n";
    } else {
        coords_label = "NULL";
    }

    if (map_data["bg_events"].array_items().size() > 0) {
        bgs_label = mapName + "_MapBGEvents";
        text << bgs_label << ":\n";
        for (auto &bg_event : map_data["bg_events"].array_items()) {
            string type = json_to_string(bg_event, "type");
            if (type == "sign") {
                text << "\tbg_sign_event "
                     << json_to_string(bg_event, "x") << ", "
                     << json_to_string(bg_event, "y") << ", "
                     << json_to_string(bg_event, "elevation") << ", "
                     << json_to_string(bg_event, "player_facing_dir") << ", "
                     << json_to_string(bg_event, "script") << "\n";
            }
            else if (type == "hidden_item") {
                string quantity = json_to_string(bg_event, "quantity", true);
                if (quantity.empty()) {
                    quantity = "1";
                }
                string underfoot = json_to_string(bg_event, "underfoot", true);
                if (underfoot.empty()) {
                    underfoot = "FALSE";
                }
                // Wayfarer only changes registered FRLG Sevii maps.  Existing
                // Emerald/HNS/event-island maps retain their legacy event
                // encoding and flag namespaces byte-for-byte.
                bool use_hoenn_namespace = false;
                text << (use_hoenn_namespace ? "\tbg_hidden_item_event_hoenn " : "\tbg_hidden_item_event ")
                     << json_to_string(bg_event, "x") << ", "
                     << json_to_string(bg_event, "y") << ", "
                     << json_to_string(bg_event, "elevation") << ", "
                     << json_to_string(bg_event, "item") << ", "
                     << json_to_string(bg_event, "flag") << ", "
                     << quantity << ", "
                     << underfoot << "\n";
            }
            else if (type == "secret_base") {
                text << "\tbg_secret_base_event "
                     << json_to_string(bg_event, "x") << ", "
                     << json_to_string(bg_event, "y") << ", "
                     << json_to_string(bg_event, "elevation") << ", "
                     << json_to_string(bg_event, "secret_base_id") << "\n";
            } else {
                FATAL_ERROR("Unknown bg event type '%s'. Expected 'sign', 'hidden_item', or 'secret_base'.\n", type.c_str());
            }
        }
        text << "\n";
    } else {
        bgs_label = "NULL";
    }

    text << mapName << "_MapEvents::\n"
         << "\tmap_events " << objects_label << ", " << warps_label << ", "
         << coords_label << ", " << bgs_label << "\n\n";

    return text.str();
}

string strip_trailing_separator(string filename) {
    if(filename.back() == '/' || filename.back() == '\\')
        filename.pop_back();

    return filename;
}
void infer_separator(string filename) {
    size_t dir_pos = filename.find_last_of("/\\");
    sep = filename[dir_pos];
}
string file_parent(string filename){
    size_t dir_pos = filename.find_last_of("/\\");
    return filename.substr(0, dir_pos + 1);
}

void process_map(string map_filepath, string layouts_filepath, string output_dir) {
    string mapdata_err, layouts_err;

    string mapdata_json_text = read_text_file(map_filepath);
    string layouts_json_text = read_text_file(layouts_filepath);

    Json map_data = Json::parse(mapdata_json_text, mapdata_err);
    if (map_data == Json())
        FATAL_ERROR("%s\n", mapdata_err.c_str());

    Json layouts_data = Json::parse(layouts_json_text, layouts_err);
    if (layouts_data == Json())
        FATAL_ERROR("%s\n", layouts_err.c_str());

    string header_text = generate_map_header_text(map_data, layouts_data);
    string events_text = generate_map_events_text(map_data);
    string connections_text = generate_map_connections_text(map_data);

    string out_dir = strip_trailing_separator(output_dir).append(sep);
    write_text_file(out_dir + "header.inc", header_text);
    write_text_file(out_dir + "events.inc", events_text);
    write_text_file(out_dir + "connections.inc", connections_text);
}

void process_event_constants(const vector<string> &map_filepaths, string output_ids_file) {
    string warning = get_generated_warning("data/maps/*/map.json", false);

    string guard_name = "CONSTANTS_MAP_EVENT_IDS";
    ostringstream ids_file_text;
    ids_file_text << get_include_guard_start(guard_name) << warning;

    for (const string &filepath : map_filepaths) {
        string err;
        string map_json_text = read_text_file(filepath);
        Json map_data = Json::parse(map_json_text, err);
        if (map_data == Json())
            FATAL_ERROR("Failed to read '%s' while generating map event constants: %s\n", filepath.c_str(), err.c_str());

        string map_id = json_to_string(map_data, "id");

        // Get IDs from the object/clone events.
        ostringstream map_ids_text;
        auto obj_events = map_data["object_events"].array_items();
        for (unsigned int i = 0; i < obj_events.size(); i++) {
            auto obj_event = obj_events[i];
            if (obj_event.object_items().find("local_id") != obj_event.object_items().end())
                map_ids_text << "#define " << json_to_string(obj_event, "local_id") << " " << i + 1 << "\n";
        }
        // Get IDs from the warp events.
        auto warp_events = map_data["warp_events"].array_items();
        for (unsigned int i = 0; i < warp_events.size(); i++) {
            auto warp_event = warp_events[i];
            if (warp_event.object_items().find("warp_id") != warp_event.object_items().end())
                map_ids_text << "#define " << json_to_string(warp_event, "warp_id") << " " << i << "\n";
        }
        // Only output if we found any IDs
        string temp = map_ids_text.str();
        if (!temp.empty()) {
            ids_file_text << "// " << map_id << "\n" << temp << "\n";
        }
    }

    ids_file_text << get_include_guard_end(guard_name);
    write_text_file(output_ids_file, ids_file_text.str());
}

string generate_groups_text(Json groups_data, vector<string> &invalid_maps, const map<string, Json> &maps_by_name) {
    ostringstream text;

    text << get_generated_warning("data/maps/map_groups.json", true);

    for (auto &key : groups_data["group_order"].array_items()) {
        string group = json_to_string(key);
        auto maps = groups_data[group].array_items();

        text << group << "::\n";
        for (Json &map_name : maps) {
            string map_name_str = json_to_string(map_name);
            auto it = find(invalid_maps.begin(), invalid_maps.end(), map_name_str);
            if (it == invalid_maps.end()) {
                if (version == "wayfarer" && !wayfarer_sinnoh_release_link_enabled
                 && get_source_version(maps_by_name.at(map_name_str)) == "sinnoh")
                    text << "\t.if HAS_SINNOH_CONTENT_ASM\n\t.4byte " << map_name_str << "\n\t.else\n\t.4byte NULL\n\t.endif\n";
                else
                    text << "\t.4byte " << map_name_str << "\n";
            } else {
                text << "\t.4byte NULL\n";
            }
        }
        text << "\n";
    }

    text << "\t.align 2\n" << "gMapGroups::\n";
    for (auto &group : groups_data["group_order"].array_items()) {
        text << "\t.4byte " << json_to_string(group) << "\n";
    }
    text << "\n";

    return text.str();
}

string generate_connections_text(Json groups_data, vector<string> &invalid_maps, string include_path, const map<string, Json> &maps_by_name) {
    vector<Json> map_names;

    for (auto &group : groups_data["group_order"].array_items()) {
        for (auto map_name : groups_data[json_to_string(group)].array_items()) {
            string map_name_str = json_to_string(map_name);
            auto it = find(invalid_maps.begin(), invalid_maps.end(), map_name_str);
            if (it == invalid_maps.end())
                map_names.push_back(map_name);
        }
    }

    vector<Json> connections_include_order = groups_data["connections_include_order"].array_items();

    if (connections_include_order.size() > 0)
        sort(map_names.begin(), map_names.end(), [connections_include_order](const Json &a, const Json &b) {
            auto iter_a = find(connections_include_order.begin(), connections_include_order.end(), a);
            if (iter_a == connections_include_order.end())
                iter_a = connections_include_order.begin() + numeric_limits<int>::max();
            auto iter_b = find(connections_include_order.begin(), connections_include_order.end(), b);
            if (iter_b == connections_include_order.end())
                iter_b = connections_include_order.begin() + numeric_limits<int>::max();
            return iter_a < iter_b;
        });

    ostringstream text;

    text << get_generated_warning("data/maps/map_groups.json", true);

    for (Json map_name : map_names) {
        const string name = json_to_string(map_name);
        const bool sinnoh = version == "wayfarer" && !wayfarer_sinnoh_release_link_enabled
                         && get_source_version(maps_by_name.at(name)) == "sinnoh";
        if (sinnoh) text << "\t.if HAS_SINNOH_CONTENT_ASM\n";
        text << "\t.include \"" << include_path << "/" << name << "/connections.inc\"\n";
        if (sinnoh) text << "\t.endif\n";
    }

    return text.str();
}

string generate_headers_text(Json groups_data, vector<string> &invalid_maps, string include_path, const map<string, Json> &maps_by_name) {
    vector<string> map_names;

    for (auto &group : groups_data["group_order"].array_items()) {
        for (auto map_name : groups_data[json_to_string(group)].array_items()) {
            string map_name_str = json_to_string(map_name);
            auto it = find(invalid_maps.begin(), invalid_maps.end(), map_name_str);
            if (it == invalid_maps.end())
                map_names.push_back(json_to_string(map_name));
        }
    }

    ostringstream text;

    text << get_generated_warning("data/maps/map_groups.json", true);

    string active_source = "hns";
    for (string map_name : map_names) {
        string map_source = get_source_version(maps_by_name.at(map_name));
        if (version == "wayfarer" && map_source != active_source) {
            if (map_source == "emerald")
                text << "\t.include \"data/wayfarer_hoenn_source_constants.inc\"\n";
            else
                text << "\t.include \"data/wayfarer_engine_source_constants.inc\"\n";
            active_source = map_source;
        }
        if (version == "wayfarer" && !wayfarer_sinnoh_release_link_enabled && map_source == "sinnoh") text << "\t.if HAS_SINNOH_CONTENT_ASM\n";
        text << "\t.include \"" << include_path << "/" << map_name << "/header.inc\"\n";
        if (version == "wayfarer" && !wayfarer_sinnoh_release_link_enabled && map_source == "sinnoh") text << "\t.endif\n";
    }

    if (version == "wayfarer" && active_source == "emerald")
        text << "\t.include \"data/wayfarer_engine_source_constants.inc\"\n";

    return text.str();
}

string generate_events_text(Json groups_data, vector<string> &invalid_maps, string include_path, const map<string, Json> &maps_by_name) {
    vector<string> map_names;

    for (auto &group : groups_data["group_order"].array_items()) {
        for (auto map_name : groups_data[json_to_string(group)].array_items()) {

            string map_name_str = json_to_string(map_name);
            auto it = find(invalid_maps.begin(), invalid_maps.end(), map_name_str);
            if (it == invalid_maps.end())
                map_names.push_back(json_to_string(map_name));
        }
    }

    ostringstream text;

    text << get_generated_warning(include_path + "/map_groups.json", true);

    string active_source = "hns";
    for (string map_name : map_names) {
        string map_source = get_source_version(maps_by_name.at(map_name));
        if (version == "wayfarer" && map_source != active_source) {
            if (map_source == "emerald")
                text << "\t.include \"data/wayfarer_hoenn_source_constants.inc\"\n";
            else
                text << "\t.include \"data/wayfarer_engine_source_constants.inc\"\n";
            active_source = map_source;
        }
        if (version == "wayfarer" && !wayfarer_sinnoh_release_link_enabled && map_source == "sinnoh") text << "\t.if HAS_SINNOH_CONTENT_ASM\n";
        text << "\t.include \"" << include_path << "/" << map_name << "/events.inc\"\n";
        if (version == "wayfarer" && !wayfarer_sinnoh_release_link_enabled && map_source == "sinnoh") text << "\t.endif\n";
    }

    if (version == "wayfarer" && active_source == "emerald")
        text << "\t.include \"data/wayfarer_engine_source_constants.inc\"\n";

    return text.str();
}

Json parse_required_map_defines(void) {
    string json_err;

    string json_text = read_text_file("tools/mapjson/required_map_defines.json");

    Json json_data = Json::parse(json_text, json_err);
    if (json_data == Json())
        FATAL_ERROR("%s\n", json_err.c_str());
    return json_data;
}

string generate_map_constants_text(string groups_filepath, Json groups_data, vector<string> &valid_map_ids) {
    string file_dir = file_parent(groups_filepath) + sep;

    string guard_name = "CONSTANTS_MAP_GROUPS";
    ostringstream text;
    ostringstream mapCountText;
    ostringstream mapSourceText;
    vector<int> mapSourceOffsets;
    vector<bool> hoennMapSources;
    vector<string> mapRegions;

    text << get_include_guard_start(guard_name) << get_generated_warning("data/maps/map_groups.json", false);

    text << "//\n// DO NOT MODIFY THIS FILE! It is auto-generated from data/maps/map_groups.json\n//\n\n";

    text << "enum\n{\n";

    int group_num = 0;
    vector<int> map_count_vec; //DEBUG
    for (auto &group : groups_data["group_order"].array_items()) {
        if (group_num > numeric_limits<signed char>::max())
            FATAL_ERROR("Map group %d exceeds the signed-byte warp limit (%d).\n", group_num, numeric_limits<signed char>::max());

        string groupName = json_to_string(group);
        text << "    // " << groupName << "\n";
        vector<string> map_ids;
        size_t max_length = 0;

        int map_count = 0; //DEBUG
        mapSourceOffsets.push_back(hoennMapSources.size());

        for (auto &map_name : groups_data[groupName].array_items()) {
            if (map_count > numeric_limits<signed char>::max())
                FATAL_ERROR("Map %d in group %s exceeds the signed-byte warp limit (%d).\n", map_count, groupName.c_str(), numeric_limits<signed char>::max());

            string map_filepath = file_dir + json_to_string(map_name) + sep + "map.json";
            string err_str;
            Json map_data = Json::parse(read_text_file(map_filepath), err_str);
            if (map_data == Json())
                FATAL_ERROR("%s: %s\n", map_filepath.c_str(), err_str.c_str());
            string id = json_to_string(map_data, "id", true);
            hoennMapSources.push_back(get_source_version(map_data) == "emerald");
            string mapRegion = json_to_string(map_data, "region", true);
            mapRegions.push_back(mapRegion.empty() ? "REGION_NONE" : mapRegion);
            map_ids.push_back(id);
            valid_map_ids.push_back(id);
            if (id.length() > max_length)
                max_length = id.length();
            map_count++; //DEBUG
        }

        int map_id_num = 0;
        for (string map_id : map_ids) {
            text << "    " << map_id << string(max_length - map_id.length(), ' ')
                 << " = (" << map_id_num++ << " | (" << group_num << " << 8)),\n";
        }

        text << "\n";

        group_num++;
        map_count_vec.push_back(map_count); //DEBUG
    }
    mapSourceOffsets.push_back(hoennMapSources.size());

    text << "};\n\n";

    text << "//Constants for unused maps\n";
    int map_id_num = 0;
    int old_map_group = -1;
    Json required_map_defines = parse_required_map_defines();
    map <int, string> filtered_map_defines;
    size_t max_length = 0;
    for (auto required_map_id : required_map_defines["required_maps"].array_items()) {
        string map_id = json_to_string(required_map_id[0]);
        auto it = find(valid_map_ids.begin(), valid_map_ids.end(), map_id);
        int current_map_group = required_map_id[1].int_value();
        if (old_map_group != current_map_group) {
            map_id_num = 0;
        } else {
            map_id_num++;
        }
        if (it == valid_map_ids.end()) {
            filtered_map_defines[(map_id_num + 256 * current_map_group)] = map_id;
            if (map_id.length() > max_length)
                max_length = map_id.length();
        }
        old_map_group = current_map_group;
    }

    for ( const auto &[map_value, map_id]: filtered_map_defines) {
        text << "#define " << map_id << string(max_length - map_id.length(), ' ')
             << "  " << map_value << "\n";
    }

    text << "\n#define MAP_GROUPS_COUNT " << group_num << "\n\n";
    text << get_include_guard_end(guard_name);

    char s = file_dir.back();
    mapCountText << "static const u8 MAP_GROUP_COUNT[] = {"; //DEBUG
    for(int i=0; i<group_num; i++){                          //DEBUG
        mapCountText << map_count_vec[i] << ", ";            //DEBUG
    }                                                        //DEBUG
    mapCountText << "0};\n";                                 //DEBUG
    write_text_file(file_dir + ".." + s + ".." + s + "src" + s + "data" + s + "map_group_count.h", mapCountText.str());

    mapSourceText << "// DO NOT MODIFY! Auto-generated from the selected map catalog.\n";
    mapSourceText << "static const u16 sWayfarerMapSourceOffsets[] = {";
    for (int offset : mapSourceOffsets)
        mapSourceText << offset << ", ";
    mapSourceText << "};\n";
    mapSourceText << "static const u8 sWayfarerHoennMapSourceBits[] = {";
    for (size_t byte = 0; byte * 8 < hoennMapSources.size(); byte++) {
        int value = 0;
        for (size_t bit = 0; bit < 8 && byte * 8 + bit < hoennMapSources.size(); bit++)
            if (hoennMapSources[byte * 8 + bit])
                value |= 1 << bit;
        mapSourceText << value << ", ";
    }
    mapSourceText << "};\n";
    mapSourceText << "static const u8 sWayfarerMapRegionNibbles[] = {";
    for (size_t byte = 0; byte * 2 < mapRegions.size(); byte++) {
        const string &lowRegion = mapRegions[byte * 2];
        const string &highRegion = byte * 2 + 1 < mapRegions.size()
            ? mapRegions[byte * 2 + 1]
            : "REGION_NONE";
        mapSourceText << "(" << lowRegion << " | (" << highRegion << " << 4)), ";
    }
    mapSourceText << "};\n";
    write_text_file(file_dir + ".." + s + ".." + s + "src" + s + "data" + s + "wayfarer_map_sources.h", mapSourceText.str());

    return text.str();
}

void validate_wayfarer_heal_locations(const set<string> &included_map_ids) {
    string err;
    Json heal_locations = Json::parse(read_text_file("src/data/heal_locations.json"), err);
    if (heal_locations == Json())
        FATAL_ERROR("Failed to read heal locations: %s\n", err.c_str());

    set<string> heal_location_ids;
    for (const Json &heal_location : heal_locations["heal_locations"].array_items()) {
        string id = json_to_string(heal_location, "id");
        if (!heal_location_ids.insert(id).second)
            FATAL_ERROR("Duplicate heal location %s in the Wayfarer catalog.\n", id.c_str());

        string source = json_to_string(heal_location, "source");
        string source_version;
        if (source == "EMERALD")
            source_version = "emerald";
        else if (source == "FRLG")
            source_version = "frlg";
        else if (source == "HNS")
            source_version = "hns";
        else
            FATAL_ERROR("Heal location %s has unknown content source %s.\n", id.c_str(), source.c_str());

        string map_id = json_to_string(heal_location, "map");
        if (version == "wayfarer" && id == "HEAL_LOCATION_CINNABAR_ISLAND_HNS")
            continue;
        if (version == "wayfarer" && source_version == "frlg") {
            if (map_id != "MAP_CINNABAR_ISLAND"
             && (!wayfarer_sevii_release_link_enabled
              || wayfarer_sevii_enabled_map_ids.find(map_id) == wayfarer_sevii_enabled_map_ids.end()))
                continue;
        } else if (!source_version_is_selected(source_version)) {
            continue;
        }

        if (included_map_ids.find(map_id) == included_map_ids.end())
            FATAL_ERROR("Heal location %s references unavailable map %s.\n", id.c_str(), map_id.c_str());

        string respawn_map = json_to_string(heal_location, "respawn_map", true);
        if (!respawn_map.empty() && included_map_ids.find(respawn_map) == included_map_ids.end())
            FATAL_ERROR("Heal location %s references unavailable respawn map %s.\n", id.c_str(), respawn_map.c_str());
    }
}

void validate_wayfarer_map_catalog(const Json &groups_data, const map<string, Json> &maps_by_name) {
    set<string> included_map_ids;
    vector<Json> included_maps;

    for (const Json &group : groups_data["group_order"].array_items()) {
        string group_name = json_to_string(group);
        for (const Json &map_name_json : groups_data[group_name].array_items()) {
            string map_name = json_to_string(map_name_json);
            auto map_it = maps_by_name.find(map_name);
            if (map_it == maps_by_name.end())
                FATAL_ERROR("Map group %s references missing map %s.\n", group_name.c_str(), map_name.c_str());
            if (!data_matches_version(map_it->second))
                continue;

            string map_id = json_to_string(map_it->second, "id");
            if (!included_map_ids.insert(map_id).second)
                FATAL_ERROR("Duplicate selected map id %s in the Wayfarer catalog.\n", map_id.c_str());
            included_maps.push_back(map_it->second);
        }
    }

    const set<string> dynamic_destinations = {"MAP_DYNAMIC", "MAP_UNDEFINED"};
    for (const Json &map_data : included_maps) {
        string map_name = json_to_string(map_data, "name");
        Json event_data = resolve_wayfarer_coast_map_events(map_data);
        Json connection_data = resolve_wayfarer_coast_connections(map_data);
        size_t warp_index = 0;
        for (const Json &source_warp : event_data["warp_events"].array_items()) {
            Json warp = resolve_wayfarer_coast_warp(map_data, source_warp, warp_index++);
            string destination = json_to_string(warp, "dest_map");
            if (included_map_ids.find(destination) == included_map_ids.end()
             && dynamic_destinations.find(destination) == dynamic_destinations.end())
                FATAL_ERROR("Map %s warp references unavailable map %s.\n", map_name.c_str(), destination.c_str());
        }
        for (const Json &connection : connection_data["connections"].array_items()) {
            string destination = json_to_string(connection, "map");
            if (included_map_ids.find(destination) == included_map_ids.end()
             && dynamic_destinations.find(destination) == dynamic_destinations.end())
                FATAL_ERROR("Map %s connection references unavailable map %s.\n", map_name.c_str(), destination.c_str());
        }
    }

    validate_wayfarer_heal_locations(included_map_ids);
}

// Output paths are directories with trailing path separators
void process_groups(string groups_filepath, vector<string> &map_filepaths, string output_asm, string output_c) {
    output_asm = strip_trailing_separator(output_asm); // Remove separator if existing.
    output_c = strip_trailing_separator(output_c);

    string err;
    Json groups_data = Json::parse(read_text_file(groups_filepath), err);
    vector<string> invalid_maps;
    vector<string> valid_map_ids;
    map<string, Json> maps_by_name;

    for (const string &filepath : map_filepaths) {
        string err;
        string map_json_text = read_text_file(filepath);
        Json map_data = Json::parse(map_json_text, err);
        if (map_data == Json())
            FATAL_ERROR("Failed to read '%s' while processing groups: %s\n", filepath.c_str(), err.c_str());

        string map_name = json_to_string(map_data, "name");
        if (!maps_by_name.emplace(map_name, map_data).second)
            FATAL_ERROR("Duplicate map name %s while processing groups.\n", map_name.c_str());

        if (!data_matches_version(map_data)) {
            invalid_maps.push_back(map_name);
        }
    }

    if (groups_data == Json())
        FATAL_ERROR("%s\n", err.c_str());

    if (version == "wayfarer")
        validate_wayfarer_map_catalog(groups_data, maps_by_name);

    string groups_text = generate_groups_text(groups_data, invalid_maps, maps_by_name);
    string connections_text = generate_connections_text(groups_data, invalid_maps, output_asm, maps_by_name);
    string headers_text = generate_headers_text(groups_data, invalid_maps, output_asm, maps_by_name);
    string events_text = generate_events_text(groups_data, invalid_maps, output_asm, maps_by_name);
    string map_header_text = generate_map_constants_text(groups_filepath, groups_data, valid_map_ids);

    write_text_file(output_asm + sep + "groups.inc", groups_text);
    write_text_file(output_asm + sep + "connections.inc", connections_text);
    write_text_file(output_asm + sep + "headers.inc", headers_text);
    write_text_file(output_asm + sep + "events.inc", events_text);
    write_text_file(output_c + sep + "map_groups.h", map_header_text);
}

bool layout_matches_version(const Json &layout) {
    return data_matches_version(layout);
}

string generate_layout_headers_text(Json layouts_data) {
    ostringstream text;

    text << get_generated_warning("data/layouts/layouts.json", true);

    for (auto &layout : layouts_data["layouts"].array_items()) {
        if (layout == Json::object()) continue;
        if (!std::filesystem::exists(json_to_string(layout, "border_filepath")))
            continue;
        if (!layout_matches_version(layout))
            continue;
        string layout_version = json_to_string(layout, "layout_version", true);
        if (layout_version.empty())
            layout_version = "emerald";
        string layoutName = json_to_string(layout, "name");
        string border_label = layoutName + "_Border";
        string blockdata_label = layoutName + "_Blockdata";
        const bool sinnoh = version == "wayfarer" && !wayfarer_sinnoh_release_link_enabled
                         && get_source_version(layout) == "sinnoh";
        if (sinnoh) text << "\t.if HAS_SINNOH_CONTENT_ASM\n";
        text << border_label << "::\n"
             << "\t.incbin \"" << json_to_string(layout, "border_filepath") << "\"\n\n"
             << blockdata_label << "::\n"
             << "\t.incbin \"" << json_to_string(layout, "blockdata_filepath") << "\"\n\n"
             << "\t.align 2\n"
             << layoutName << "::\n"
             << "\t.4byte " << json_to_string(layout, "width") << "\n"
             << "\t.4byte " << json_to_string(layout, "height") << "\n"
             << "\t.4byte " << border_label << "\n"
             << "\t.4byte " << blockdata_label << "\n"
             << "\t.4byte " << json_to_string(layout, "primary_tileset") << "\n"
             << "\t.4byte " << json_to_string(layout, "secondary_tileset") << "\n";
        if (layout_version == "frlg")
            text << "\t.byte 1\n"; // LAYOUT_VERSION_FRLG
        else if (layout_version == "hns")
            text << "\t.byte 2\n"; // LAYOUT_VERSION_HNS
        else
            text << "\t.byte 0\n"; // LAYOUT_VERSION_EMERALD

        if (layout_version == "frlg")
        {
            text << "\t.byte " << json_to_string(layout, "border_width") << "\n"
                 << "\t.byte " << json_to_string(layout, "border_height") << "\n"
                 << "\t.byte 0\n";
        }
        else
        {
            text << "\t.2byte 0\n"
                 << "\t.byte 0\n";
        }
        text << "\n";
        if (sinnoh) text << "\t.endif\n\n";
    }

    return text.str();
}

string generate_layouts_table_text(Json layouts_data) {
    ostringstream text;

    text << get_generated_warning("data/layouts/layouts.json", true);

    text << "\t.align 2\n"
         << json_to_string(layouts_data, "layouts_table_label") << "::\n";

    for (auto &layout : layouts_data["layouts"].array_items()) {
        if (!std::filesystem::exists(json_to_string(layout, "border_filepath")))
            continue;
        if (!layout_matches_version(layout)) {
            text << "\t.4byte NULL\n";
        } else {
            string layout_name = json_to_string(layout, "name", true);
            if (layout_name.empty()) layout_name = "NULL";
            if (version == "wayfarer" && !wayfarer_sinnoh_release_link_enabled
             && get_source_version(layout) == "sinnoh")
                text << "\t.if HAS_SINNOH_CONTENT_ASM\n\t.4byte " << layout_name << "\n\t.else\n\t.4byte NULL\n\t.endif\n";
            else
                text << "\t.4byte " << layout_name << "\n";
        }
    }

    return text.str();
}

vector<string> parse_required_layout_defines()
{
    vector<string> v;
    string json_err;

    string json_text = read_text_file("tools/mapjson/required_map_defines.json");

    Json json_data = Json::parse(json_text, json_err);
    if (json_data == Json())
        FATAL_ERROR("%s\n", json_err.c_str());

    for (auto required_layout : json_data["required_layouts"].array_items()) {
        v.push_back(json_to_string(required_layout));
    }

    return v;
}
string generate_layouts_constants_text(Json layouts_data) {
    string guard_name = "CONSTANTS_LAYOUTS";
    ostringstream text;
    vector<string> defined_layouts;
    text << get_include_guard_start(guard_name) << get_generated_warning("data/layouts/layouts.json", false);

    int i = 1;
    for (auto &layout : layouts_data["layouts"].array_items()) {
        if (!std::filesystem::exists(json_to_string(layout, "border_filepath")))
            continue;
        if (layout != Json::object())
        {
            text << "#define " << json_to_string(layout, "id") << " " << i << "\n";
            defined_layouts.push_back(json_to_string(layout, "id"));
        }
        i++;
    }

    text << "\n//Constants for unused layouts\n";
    vector<string> required_layout_defines = parse_required_layout_defines();
    vector<string> filtered_layout_defines;
    size_t max_length = 0;
    for (auto &layout : required_layout_defines) {
        auto it = find(defined_layouts.begin(), defined_layouts.end(), layout);
        if (it == defined_layouts.end()) {
            filtered_layout_defines.push_back(layout);
            if (layout.length() > max_length)
                max_length = layout.length();
        }
    }

    for (auto &layout : filtered_layout_defines) {
        text << "#define " << layout << string(max_length - layout.length(), ' ')
             << "  0xFFFF\n";
    }
    text << "\n" << get_include_guard_end(guard_name);

    return text.str();
}

void process_layouts(string layouts_filepath, string output_asm, string output_c) {
    output_asm = strip_trailing_separator(output_asm).append(sep);
    output_c = strip_trailing_separator(output_c).append(sep);

    string err;
    Json layouts_data = Json::parse(read_text_file(layouts_filepath), err);

    if (layouts_data == Json())
        FATAL_ERROR("%s\n", err.c_str());

    string layout_headers_text = generate_layout_headers_text(layouts_data);
    string layouts_table_text = generate_layouts_table_text(layouts_data);
    string layouts_constants_text = generate_layouts_constants_text(layouts_data);

    write_text_file(output_asm + "layouts.inc", layout_headers_text);
    write_text_file(output_asm + "layouts_table.inc", layouts_table_text);
    write_text_file(output_c + "layouts.h", layouts_constants_text);
}

int main(int argc, char *argv[]) {
    if (argc < 3)
        FATAL_ERROR("USAGE: mapjson <mode> <game-version> [options]\n");

    char *version_arg = argv[2];
    version = string(version_arg);
    if (version != "emerald" && version != "ruby" && version != "firered" && version != "hns" && version != "wayfarer")
        FATAL_ERROR("ERROR: <game-version> must be 'emerald', 'firered', 'hns', 'wayfarer', or 'ruby'.\n");

    while (argc >= 5) {
        const string option = argv[argc - 2];
        if (option == "--wayfarer-sevii-manifest")
            wayfarer_sevii_manifest_path = argv[argc - 1];
        else if (option == "--wayfarer-sinnoh-manifest")
            wayfarer_sinnoh_manifest_path = argv[argc - 1];
        else if (option == "--wayfarer-sinnoh-asset-manifest")
            wayfarer_sinnoh_asset_manifest_path = argv[argc - 1];
        else
            break;
        argc -= 2;
    }
    load_wayfarer_sevii_manifest();
    load_wayfarer_sinnoh_manifest();

    char *mode_arg = argv[1];
    string mode(mode_arg);
    if (mode == "map") {
        if (argc != 6)
            FATAL_ERROR("USAGE: mapjson map <game-version> <map_file> <layouts_file> <output_dir>\n");

        infer_separator(argv[3]);
        string filepath(argv[3]);
        string layouts_filepath(argv[4]);
        string output_dir(argv[5]);

        process_map(filepath, layouts_filepath, output_dir);
    }
    else if (mode == "groups") {
        if (argc < 6)
            FATAL_ERROR("USAGE: mapjson groups <game-version> <groups_file> <map_file> [additional_map_files] <output_asm_dir> <output_c_dir>\n");

        infer_separator(argv[3]);
        string filepath(argv[3]);

        vector<string> map_filepaths;
        const int firstMapFileArg = 4;
        const int lastMapFileArg = argc - 3;
        for (int i = firstMapFileArg; i <= lastMapFileArg; i++) {
            map_filepaths.push_back(argv[i]);
        }

        string output_asm(argv[argc - 2]);
        string output_c(argv[argc - 1]);

        process_groups(filepath, map_filepaths, output_asm, output_c);
    }
    else if (mode == "layouts") {
        if (argc != 6)
            FATAL_ERROR("USAGE: mapjson layouts <game-version> <layouts_file> <output_asm_dir> <output_c_dir>\n");

        infer_separator(argv[3]);
        string filepath(argv[3]);
        string output_asm(argv[4]);
        string output_c(argv[5]);

        process_layouts(filepath, output_asm, output_c);
    }
    else if (mode == "event_constants") {
        if (argc < 5)
            FATAL_ERROR("USAGE: mapjson event_constants <game-version> <map_file> [additional_map_files] <output_ids_file>");

        infer_separator(argv[3]);

        vector<string> filepaths;
        const int firstMapFileArg = 3;
        const int lastMapFileArg = argc - 2;
        for (int i = firstMapFileArg; i <= lastMapFileArg; i++) {
            filepaths.push_back(argv[i]);
        }
        string output_ids_file(argv[argc - 1]);

        process_event_constants(filepaths, output_ids_file);
    }
    else {
        FATAL_ERROR("ERROR: <mode> must be 'layouts', 'map', 'event_constants', or 'groups'.\n");
    }

    return 0;
}
