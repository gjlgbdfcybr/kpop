#include "model.hpp"

#include <cmath>
#include <stdexcept>
#include <utility>

InventoryItem::InventoryItem(std::string number, std::string title) {
    if (!set_inventory_number(number)) {
        throw std::invalid_argument("Inventory number must not be empty");
    }
    if (!set_name(title)) {
        throw std::invalid_argument("Name must not be empty");
    }
}

bool InventoryItem::set_name(std::string value) {
    if (value.empty()) return false;
    name = value;
    return true;
}

std::string InventoryItem::get_name() const { return name; }

bool InventoryItem::set_inventory_number(std::string value) {
    if (value.empty()) return false;
    inventory_number = value;
    return true;
}

std::string InventoryItem::get_inventory_number() const {
    return inventory_number;
}

Equipment::Equipment(std::string number, std::string title)
    : InventoryItem(number, title) {}

Helmet::Helmet(std::string number, std::string title)
    : Equipment(number, title) {}

ChargingStation::ChargingStation(std::string number, std::string title,
                                 double energy)
    : Equipment(number, title) {
    if (!std::isfinite(energy) || energy < 0) {
        throw std::invalid_argument(
            "Daily energy must be finite and non-negative");
    }

    daily_energy = energy;
}

double ChargingStation::get_daily_energy() const { return daily_energy; }

Vehicle::Vehicle(std::string number, std::string title, int ease_value,
                 bool brakes)
    : InventoryItem(number, title) {
    if (!set_ease(ease_value)) {
        throw std::invalid_argument("Ease must be between 1 and 10");
    }
    brakes_ok = brakes;
}

bool Vehicle::set_ease(int value) {
    if (value < 1 || value > 10) {
        return false;
    }

    ease = value;
    return true;
}

int Vehicle::get_ease() const { return ease; }

bool Vehicle::has_working_brakes() const { return brakes_ok; }

bool Vehicle::is_beginner_friendly() const { return ease >= 6; }

Bicycle::Bicycle(std::string number, std::string title, int ease_value,
                 bool brakes)
    : Vehicle(number, title, ease_value, brakes) {}

Scooter::Scooter(std::string number, std::string title, int ease_value,
                 double energy, bool brakes)
    : Vehicle(number, title, ease_value, brakes) {
    if (!set_daily_energy(energy)) {
        throw std::invalid_argument(
            "Daily energy must be finite and non-negative");
    }
}

bool Scooter::set_daily_energy(double value) {
    if (!std::isfinite(value) || value < 0) return false;

    daily_energy = value;
    return true;
}

double Scooter::get_daily_energy() const { return daily_energy; }

bool RejectingService::inspect(const Vehicle&) const { return false; }

bool ServiceCenter::inspect(const Vehicle& vehicle) const {
    return vehicle.has_working_brakes();
}

bool Park::has_number(const std::string& number) const {
    for (auto& vehicle : vehicles) {
        if (vehicle->get_inventory_number() == number) {
            return true;
        }
    }
    for (auto& equipment : equipments) {
        if (equipment->get_inventory_number() == number) {
            return true;
        }
    }

    return false;
}

Park::Park(const InspectionService& service) : service_center(service) {}

bool Park::add_vehicle(std::unique_ptr<Vehicle> vehicle) {
    last_error.clear();
    if (vehicle == nullptr) {
        last_error = "No vehicle supplied";
        return false;
    }

    if (has_number(vehicle->get_inventory_number())) {
        last_error = "Inventory number already exists";
        return false;
    }

    if (!service_center.inspect(*vehicle)) {
        last_error = "Vehicle failed technical inspection";
        return false;
    }

    vehicles.push_back(std::move(vehicle));
    return true;
}

bool Park::add_equipment(std::unique_ptr<Equipment> equipment) {
    last_error.clear();
    if (equipment == nullptr) {
        last_error = "No equipment supplied";
        return false;
    }

    if (has_number(equipment->get_inventory_number())) {
        last_error = "Inventory number already exists";
        return false;
    }

    equipments.push_back(std::move(equipment));
    return true;
}

std::string Park::get_last_error() const { return last_error; }

int Park::get_vehicle_count() const { return vehicles.size(); }

int Park::get_equipment_count() const { return equipments.size(); }

double Park::get_total_energy() const {
    double res = 0;
    for (auto& vehicle : vehicles) {
        const auto* consumer =
            dynamic_cast<const EnergyConsumer*>(vehicle.get());
        if (consumer != nullptr) {
            res += consumer->get_daily_energy();
        }
    }

    for (auto& equipment : equipments) {
        const auto* consumer =
            dynamic_cast<const EnergyConsumer*>(equipment.get());
        if (consumer != nullptr) {
            res += consumer->get_daily_energy();
        }
    }
    return res;
}

std::vector<const Inventoriable*> Park::get_inventory() const {
    std::vector<const Inventoriable*> result;

    for (const auto& vehicle : vehicles) {
        result.push_back(vehicle.get());
    }
    for (const auto& equipment : equipments) {
        result.push_back(equipment.get());
    }

    return result;
}

std::vector<const Equipment*> Park::get_equipments() const {
    std::vector<const Equipment*> result;

    for (auto& equipment : equipments) {
        result.push_back(equipment.get());
    }

    return result;
}

std::vector<const Vehicle*> Park::get_vehicles() const {
    std::vector<const Vehicle*> result;

    for (auto& vehicle : vehicles) {
        result.push_back(vehicle.get());
    }

    return result;
}

std::vector<const Vehicle*> Park::get_beginner_vehicles() const {
    std::vector<const Vehicle*> result;

    for (auto& vehicle : vehicles) {
        if (vehicle->is_beginner_friendly()) {
            result.push_back(vehicle.get());
        }
    }

    return result;
}

Container::Container(const InspectionService& service)
    : service_center(service) {}

std::unique_ptr<Park> Container::create_park() const {
    return std::make_unique<Park>(service_center);
}
