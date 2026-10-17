#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include "model.hpp"

using namespace std;

bool read_text(const string& prompt, string& value) {
    cout << prompt;
    return static_cast<bool>(getline(cin, value));
}

template <class T>
bool read_number(const string& prompt, T& value, T minimum, T maximum) {
    string line;
    while (read_text(prompt, line)) {
        istringstream input(line);
        T parsed;
        if (input >> parsed) {
            input >> ws;
            if (input.eof() && isfinite(parsed)) {
                if (parsed < minimum || parsed > maximum) {
                    if (maximum == numeric_limits<T>::max()) {
                        cout << "Value must be at least " << minimum << ". Try again.\n";
                    } else {
                        cout << "Value must be between " << minimum << " and "
                             << maximum << ". Try again.\n";
                    }
                    continue;
                }
                value = parsed;
                return true;
            }
        }
        cout << "Invalid number. Try again.\n";
    }
    return false;
}

void add_vehicle_from_input(Park& park) {
    int type;
    if (!read_number("1. Scooter, 2. Bicycle: ", type, 1, 2)) return;

    string number, name;
    int ease, brakes;
    double energy = 0;
    if (!read_text("Inventory number: ", number)) return;
    if (!read_text("Name: ", name)) return;
    if (!read_number("Ease (1-10): ", ease, 1, 10)) return;
    if (!read_number("Working brakes (1=yes, 0=no): ", brakes, 0, 1)) return;
    if (type == 1 && !read_number("Daily energy (kWh/day): ", energy, 0.0,
                                  numeric_limits<double>::max())) return;

    try {
        unique_ptr<Vehicle> vehicle;
        if (type == 1) {
            vehicle =
                make_unique<Scooter>(number, name, ease, energy, brakes == 1);
        }
        else {
            vehicle = make_unique<Bicycle>(number, name, ease, brakes == 1);
        }
        if (park.add_vehicle(move(vehicle))) {
            cout << "Added\n";
        }
        else {
            cout << "Rejected: " << park.get_last_error() << '\n';
        }
    } catch (const invalid_argument& error) {
        cout << "Error: " << error.what() << '\n';
    }
}

void add_equipment_from_input(Park& park) {
    int type;
    if (!read_number("1. Helmet, 2. Charging station: ", type, 1, 2)) return;

    string number, name;
    double energy = 0;
    if (!read_text("Inventory number: ", number)) return;
    if (!read_text("Name: ", name)) return;
    if (type == 2 && !read_number("Daily energy (kWh/day): ", energy, 0.0,
                                  numeric_limits<double>::max())) return;

    try {
        unique_ptr<Equipment> equipment;
        if (type == 1) {
            equipment = make_unique<Helmet>(number, name);
        }
        else {
            equipment = make_unique<ChargingStation>(number, name, energy);
        }
        if (park.add_equipment(move(equipment))) {
            cout << "Added\n";
        }
        else {
            cout << "Rejected: " << park.get_last_error() << '\n';
        }
    } catch (const invalid_argument& error) {
        cout << "Error: " << error.what() << '\n';
    }
}

int main() {
    ServiceCenter service;
    Container container(service);
    auto park = container.create_park();

    park->add_vehicle(
        make_unique<Scooter>("S-001", "City scooter", 7, 2.5, true));
    park->add_vehicle(make_unique<Bicycle>("B-001", "City bike", 5, true));
    park->add_equipment(make_unique<Helmet>("H-001", "City helmet"));
    park->add_equipment(
        make_unique<ChargingStation>("C-001", "Charging station", 3.0));

    while (cin) {
        cout << "\n1. Show report\n"
             << "2. Show beginner vehicles\n"
             << "3. Show inventory\n"
             << "4. Add vehicle\n"
             << "5. Add equipment\n"
             << "0. Exit\n";

        int choice;
        if (!read_number("> ", choice, 0, 5)) return 0;

        switch (choice) {
            case 1:
                cout << "Vehicles: " << park->get_vehicle_count() << '\n';
                cout << "Equipment: " << park->get_equipment_count() << '\n';
                cout << "Total items: "
                     << park->get_vehicle_count() + park->get_equipment_count() << '\n';
                cout << "Daily energy: " << park->get_total_energy()
                     << " kWh/day\n";
                break;
            case 2: {
                const auto beginners = park->get_beginner_vehicles();
                cout << "For beginners:\n";
                if (beginners.empty()) {
                    cout << "No suitable vehicles\n";
                }
                for (const auto* vehicle : beginners) {
                    cout << vehicle->get_inventory_number() << ' '
                         << vehicle->get_name() << " | Ease: "
                         << vehicle->get_ease() << "/10\n";
                }
                break;
            }
            case 3: {
                const auto inventory = park->get_inventory();
                cout << "Inventory:\n";
                if (inventory.empty()) {
                    cout << "Inventory is empty\n";
                }
                for (const auto* item : inventory) {
                    cout << item->get_inventory_number() << ' '
                         << item->get_name() << '\n';
                }
                break;
            }
            case 4:
                add_vehicle_from_input(*park);
                break;
            case 5:
                add_equipment_from_input(*park);
                break;
            case 0:
                return 0;
            default:
                cout << "Unknown command\n";
                break;
        }
    }
}
