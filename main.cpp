#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <limits>
#include <algorithm>
#include <thread>
#include <chrono>
#include <cctype>

using ID = std::string;

enum class Season { Spring, Summer, Fall, Winter };

struct Item {
    ID name;
    explicit Item(ID  name) : name(std::move(name)) {} // Constructor
};

struct NPC {
    ID name;
    std::vector<ID> likes;
    std::vector<ID> loves;
    int weekly_count{0};
    bool gift_received{false};
};

std::vector<Season> parse_seasons (const std::string& season_str);

std::vector<ID> parse_npcs (const std::string& npc_str);

void wait_for_user();

void load_data (const std::string& filename, std::map<ID, std::vector<Item>>& season_items, std::map<ID, NPC>& npc_map);

void add_item (std::map<ID, std::vector<Item>>& season_items, const ID& name, const std::vector<Season>& seasons);

void update_or_add_npc (std::map<ID, NPC>& npc_map, const ID& npc_id, const ID& item_id, bool is_love);

void clear_screen();

void display_loading_screen();

void display_menu(const std::string* weekdays, const int& current_weekday, const Season& current_season);

char get_user_choice();

void gift_given(std::map<ID, NPC>& npc_map);

void edit_count(std::map<ID, NPC>& npc_map);

Season change_season(int& current_weekday, std::map <ID, NPC>& npc_map);

void remaining_npcs(const std::map<ID, NPC>& npc_map, const std::map<ID, std::vector<Item>>& season_items, Season current_season);

void reset_week(int& current_weekday, std::map<ID, NPC>& npc_map);

void increment_day(const std::string* weekdays, int& current_weekday, std::map<ID, NPC>& npc_map);

std::string to_lowercase(const std::string& str);

std::string to_uppercase(const std::string& str);

std::string to_lowercase(const char& c);

std::string season_to_string(Season season);

Season to_season(const std::string& season);


int main() {
    std::map<ID, NPC> npc_map;
    std::map<ID, std::vector<Item>> all_season_items;

    std::string week[7] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
    int current_weekday{0};

    // Assuming "data.txt" is in the current directory
    load_data("data.txt", all_season_items, npc_map);

    // Season variable to keep track of the current season
    Season current_season{Season::Spring};

    clear_screen();
    display_loading_screen();
    display_menu(week, current_weekday, current_season);
    char choice{get_user_choice()};
    while (choice != '0') {
        switch (choice) {
            case '1': gift_given(npc_map); break;
            case '2': edit_count(npc_map); break;
            case '3': {
                current_season = change_season(current_weekday, npc_map);
                std::cout << "It is now a " << week[current_weekday] << " of " << season_to_string(current_season) << ".\n";
                break;
            }
            case '4': remaining_npcs(npc_map, all_season_items, current_season); break;
            case '5': increment_day(week, current_weekday, npc_map); break;
            case '6': reset_week(current_weekday, npc_map); break;
            default: {
                std::cout << "Invalid option. Please try again.\n";
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
        }
        display_menu(week, current_weekday, current_season);
        choice = get_user_choice();
    }

    return 0;
}


std::vector<Season> parse_seasons (const std::string& season_str) {
    std::vector<Season> seasons;
    if (season_str[0] == 'a') seasons.emplace_back(Season::Spring);
    if (season_str[1] == 'a') seasons.emplace_back(Season::Summer);
    if (season_str[2] == 'a') seasons.emplace_back(Season::Fall);
    if (season_str[3] == 'a') seasons.emplace_back(Season::Winter);
    return seasons;
}

std::vector<ID> parse_npcs(const std::string& npc_str) {
    std::vector<ID> npcs;
    if (npc_str.size() <= 3 || npc_str[2] == ')') return npcs; // Handle empty cases like V() or K()

    std::string npc_list = npc_str.substr(2, npc_str.size() - 3); // Extracting the list

    std::stringstream ss(npc_list);
    std::string npc;
    while (std::getline(ss, npc, ',')) {
        npc.erase(std::remove_if(npc.begin(), npc.end(), ::isspace), npc.end()); // Remove whitespace
        if (!npc.empty()) {  // Avoid adding empty strings from trailing commas
            npcs.push_back(npc);
        }
    }
    return npcs;
}

void wait_for_user() {
    std::cout << "Press enter to continue.";
    std::cin.get();
}

void load_data(const std::string& filename, std::map<ID, std::vector<Item>>& season_items, std::map<ID, NPC>& npc_map) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << filename << std::endl; // Error message
        return;
    }
    std::string line;
    while (std::getline(file, line)) {

        std::stringstream ss(line);
        std::string season_str, item_name, loves_str, likes_str;
        std::getline(ss, season_str, ';');
        std::getline(ss, item_name, ';');
        std::getline(ss, loves_str, ';');
        std::getline(ss, likes_str, ';');

        std::vector<Season> seasons = parse_seasons(season_str);
        std::vector<ID> loves = parse_npcs(loves_str);
        std::vector<ID> likes = parse_npcs(likes_str);

        add_item(season_items, item_name, seasons);

        for (const auto& npc_id : loves) {
            update_or_add_npc(npc_map, npc_id, item_name, true);
        }
        for (const auto& npc_id : likes) {
            update_or_add_npc(npc_map, npc_id, item_name, false);
        }
    }
    file.close();
}

void add_item (std::map<ID, std::vector<Item>>& season_items, const ID& name, const std::vector<Season>& seasons) {
    for (Season season : seasons) {
        ID season_name = season_to_string(season);
        season_items[season_name].emplace_back(name); // Explicitly construct Item
    }
}

void update_or_add_npc (std::map<ID, NPC>& npc_map, const ID& npc_id, const ID& item_id, bool is_love) {
    auto& npc = npc_map[npc_id];
    npc.name = npc_id;  // Set the name if it's a new NPC
    if (is_love) {
        npc.loves.push_back(item_id);
    } else {
        npc.likes.push_back(item_id);
    }
}

void clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void display_loading_screen() {
    std::string ascii_art = R"(
  ___  __  ____  ____  ____     __  ____    ____   __    ___  _  _   __
 / __)(  )(  __)(_  _)/ ___)   /  \(  __)  (  _ \ / _\  / __)/ )( \ / _\
( (_ \ )(  ) _)   )(  \___ \  (  O )) _)    ) __//    \( (__ ) __ (/    \
 \___/(__)(__)   (__) (____/   \__/(__)    (__)  \_/\_/ \___)\_)(_/\_/\_/
    )";

    std::cout << ascii_art << std::endl;
    std::cout << "Roots of Pacha Gift Tracker (c) Lurulai 2023/2024\n\n";
    std::cout << "Loading ";

    // Loading animation
    const std::string cursor = "-\\|/";
    int pos{0};
    for (int i{0}; i < 25; ++i) {
        // Adjust 10 to run for however long.
        std::cout << cursor[pos] << std::flush;
        // Speed of rotation: 100
        std::this_thread::sleep_for(std::chrono::milliseconds(450));
        std::cout << "\b" << std::flush;
        pos = (pos + 1) % cursor.size();
    }
    clear_screen();
}

void display_menu(const std::string* weekdays, const int& current_weekday, const Season& current_season) {
    clear_screen();
    std::string ascii_art = R"(
  ___  __  ____  ____  ____     __  ____    ____   __    ___  _  _   __
 / __)(  )(  __)(_  _)/ ___)   /  \(  __)  (  _ \ / _\  / __)/ )( \ / _\
( (_ \ )(  ) _)   )(  \___ \  (  O )) _)    ) __//    \( (__ ) __ (/    \
 \___/(__)(__)   (__) (____/   \__/(__)    (__)  \_/\_/ \___)\_)(_/\_/\_/
    )";
    std::cout << ascii_art << std::endl;
    std::cout << "Roots of Pacha Gift Tracker (c) Lurulai 2023/2024\n\n";
    std::cout << "It is currently a " << weekdays[current_weekday] << ", of " << season_to_string(current_season) << ".\n\n";
    std::cout << "1. Gift Given\n";
    std::cout << "2. Edit Count\n";
    std::cout << "3. Change Season\n";
    std::cout << "4. Remaining NPCs\n";
    std::cout << "5. Increment Day\n";
    std::cout << "6. Reset Week\n";
    std::cout << "0. Exit\n\n";
    std::cout << "Enter your choice: ";
}

char get_user_choice() {
    char choice;
    std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return choice;
}

std::string get_string() {
    std::string str;
    std::getline(std::cin, str);
    return str;
}

void gift_given(std::map<ID, NPC>& npc_map) {
    std::cout << "Which NPC?: ";
    std::string npc_name{get_string()};
    npc_name = to_uppercase(npc_name); // Convert to uppercase.

    try {
        NPC& mentioned_npc = npc_map.at(npc_name);

        if (mentioned_npc.weekly_count == 2) {
            std::cout << npc_name << " has already received two gifts this week.\n";
        } else if (mentioned_npc.gift_received) {
            std::cout << npc_name << " has already received a gift today.\n";
        } else {
            mentioned_npc.weekly_count++;
            mentioned_npc.gift_received = true;
            std::cout << "Gift given to " << npc_name << ".\n";
            std::cout << npc_name << " has received " << mentioned_npc.weekly_count << " gifts this week.\n";
        }
    } catch (const std::out_of_range& e) {
        std::cout << "NPC named '" << npc_name << "' not found. Please try again!\n";
    }
    std::this_thread::sleep_for(std::chrono::seconds(2)); // Assuming you want to wait after handling the input
}

void edit_count(std::map<ID, NPC>& npc_map) {
    std::cout << "Edit count for who?: ";
    std::string npc_name{get_string()};
    npc_name = to_uppercase(npc_name); // Assuming to_uppercase is a function you've defined elsewhere.

    try {
        NPC& mentioned_npc = npc_map.at(npc_name);
        std::cout << npc_name << " has received " << mentioned_npc.weekly_count << " gifts this week.\n";
        std::cout << "Enter new count: ";
        int new_count;
        std::cin >> new_count;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (new_count >= 0 && new_count <= 2) {
            mentioned_npc.weekly_count = new_count;
            std::cout << "Updated count for " << npc_name << " to " << new_count << ".\n";
        } else {
            std::cout << "Invalid count. Please enter a number between 0 and 2.\n";
        }
    } catch (const std::out_of_range& e) {
        std::cout << "NPC named '" << npc_name << "' not found. Please try again.\n";
    }
    std::this_thread::sleep_for(std::chrono::seconds(1)); // Assuming you want to wait after handling the input
}

Season change_season(int& current_weekday, std::map <ID, NPC>& npc_map) {
    std::cout << "Change to which season?: ";
    std::string season{get_string()};

    current_weekday = 0;
    for (auto& npc_pair : npc_map) {
        npc_pair.second.gift_received = false;
    }

    return to_season(season);
}

void remaining_npcs(const std::map<ID, NPC>& npc_map, const std::map<ID, std::vector<Item>>& season_items, Season current_season) {
    clear_screen();
    std::cout << "Remaining NPCs: \n";
    for (auto& npc_pair : npc_map) {
        if (npc_pair.second.weekly_count != 2 && !npc_pair.second.gift_received) {
            std::cout << npc_pair.first << std::endl;
            std::cout << "Loves: ";
            std::string season_str = season_to_string(current_season);
            for (const Item& item : season_items.at(season_str)) {
                if (std::find(npc_pair.second.loves.begin(), npc_pair.second.loves.end(), item.name) != npc_pair.second.loves.end()) {
                    std::cout << item.name << ", ";
                }
            }
            std::cout << "\nLikes: ";
            for (const Item& item : season_items.at(season_str)) {
                if (std::find(npc_pair.second.likes.begin(), npc_pair.second.likes.end(), item.name) != npc_pair.second.likes.end()) {
                    std::cout << item.name << ", ";
                }
            }
            std::cout << "\n\n";
        }
    }
    wait_for_user();
}

void reset_week(int& current_weekday, std::map<ID, NPC>& npc_map) {
    std::cout << "Are you sure you want to reset the week? [y/n]: ";
    char choice{get_user_choice()};
    if (std::tolower(choice) == 'y') {
        std::cout << "Week reset!\n";
    }

    for (auto& npc_pair : npc_map) {
        npc_pair.second.gift_received = false;
    }

    current_weekday = 0;
}

void increment_day(const std::string* weekdays, int& current_weekday, std::map<ID, NPC>& npc_map) {
    std::cout << "Are you sure you want to increment the day? [y/n]: ";
    char choice{get_user_choice()};
    if (std::tolower(choice) == 'y') {
        current_weekday = (current_weekday + 1) % 7;
        std::cout << "It is now " << weekdays[current_weekday] << ". :)\n";
        for (auto& npc_pair : npc_map) {
            npc_pair.second.gift_received = false;
        }
    }
    std::this_thread::sleep_for(std::chrono::seconds(2));
}

std::string season_to_string(Season season) {
    switch (season) {
        case Season::Spring: return "Spring";
        case Season::Summer: return "Summer";
        case Season::Fall: return "Fall";
        case Season::Winter: return "Winter";
        default: return "Unknown";
    }
}

// Helper function to convert a string to lowercase
std::string to_lowercase(const std::string& str) {
    std::string lower_str = str;
    std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return lower_str;
}

std::string to_uppercase(const std::string& str) {
    std::string upper_str = str;
    std::transform(upper_str.begin(), upper_str.end(), upper_str.begin(),
                   [](unsigned char c){ return std::toupper(c); });
    return upper_str;
}

// Overload for string input
Season to_season(const std::string& season) {
    std::string lower_season = to_lowercase(season);

    if (lower_season == "spring") return Season::Spring;
    if (lower_season == "summer") return Season::Summer;
    if (lower_season == "fall") return Season::Fall;
    if (lower_season == "winter") return Season::Winter;

    return Season::Spring; // default or error handling
}

// Overload for int input
Season to_season(int season) {
    switch (season) {
        case 0: return Season::Spring;
        case 1: return Season::Summer;
        case 2: return Season::Fall;
        case 3: return Season::Winter;
        default: return Season::Spring; // default or error handling
    }
}

// Overload for char input
Season to_season(char season) {
    switch (season) {
        case '0': return Season::Spring;
        case '1': return Season::Summer;
        case '2': return Season::Fall;
        case '3': return Season::Winter;
        default: return Season::Spring; // default or error handling
    }
}
