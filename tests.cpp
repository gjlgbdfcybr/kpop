#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

#include "model.hpp"

using namespace std;

void check(bool condition, const string& message) {
    if (!condition) throw runtime_error(message);
}

void test_empty_park() {
    ServiceCenter service;
    Container container(service);
    auto park = container.create_park();
    check(park->get_vehicle_count() == 0, "Empty vehicle count");
    check(park->get_equipment_count() == 0, "Empty equipment count");
    check(park->get_total_energy() == 0, "Empty energy total");
    check(park->get_inventory().empty(), "Empty inventory");
    check(park->get_beginner_vehicles().empty(), "Empty beginners");
}

void test_inspection() {
    ServiceCenter service;
    Container container(service);
    auto park = container.create_park();
    check(park->add_vehicle(make_unique<Bicycle>("B", "Bike", 6, true)),
          "Healthy vehicle accepted");
    check(
        !park->add_vehicle(make_unique<Scooter>("S", "Broken", 9, 100, false)),
        "Broken vehicle rejected");
    check(park->get_vehicle_count() == 1, "Rejected vehicle not counted");
    check(park->get_total_energy() == 0, "Rejected energy not counted");
    check(park->get_beginner_vehicles().size() == 1,
          "Rejected vehicle not listed");
    check(park->add_vehicle(make_unique<Scooter>("S", "Repaired", 9, 2, true)),
          "Rejection does not reserve number");
}

void test_duplicate_numbers() {
    ServiceCenter service;
    Container container(service);
    auto park = container.create_park();
    check(park->add_vehicle(make_unique<Bicycle>("B", "Bike", 6, true)),
          "Add bike");
    check(
        !park->add_vehicle(make_unique<Scooter>("B", "Duplicate", 7, 2, true)),
        "Duplicate vehicle number rejected");
    check(!park->add_equipment(make_unique<Helmet>("B", "Duplicate")),
          "Equipment cannot reuse vehicle number");
    check(park->add_equipment(make_unique<Helmet>("H", "Helmet")),
          "Add helmet");
    check(!park->add_equipment(make_unique<Helmet>("H", "Duplicate")),
          "Duplicate equipment number rejected");
    check(!park->add_vehicle(make_unique<Bicycle>("H", "Duplicate", 6, true)),
          "Vehicle cannot reuse equipment number");
    check(park->get_inventory().size() == 2,
          "Duplicates do not change inventory");
}

void test_beginner_boundary() {
    ServiceCenter service;
    Container container(service);
    auto park = container.create_park();
    park->add_vehicle(make_unique<Bicycle>("B5", "Five", 5, true));
    park->add_vehicle(make_unique<Bicycle>("B6", "Six", 6, true));
    park->add_vehicle(make_unique<Scooter>("S10", "Ten", 10, 1, true));
    park->add_equipment(make_unique<Helmet>("H", "Helmet"));
    const auto beginners = park->get_beginner_vehicles();
    check(beginners.size() == 2, "Exactly two beginner vehicles");
    check(beginners[0]->get_inventory_number() == "B6", "Ease 6 included");
    check(beginners[1]->get_inventory_number() == "S10", "Ease 10 included");
}

void test_energy_and_inventory() {
    ServiceCenter service;
    Container container(service);
    auto park = container.create_park();
    park->add_vehicle(make_unique<Scooter>("S", "Scooter", 7, 2.5, true));
    park->add_vehicle(make_unique<Bicycle>("B", "Bike", 6, true));
    park->add_equipment(make_unique<Helmet>("H", "Helmet"));
    park->add_equipment(make_unique<ChargingStation>("C", "Charger", 3));
    check(abs(park->get_total_energy() - 5.5) < 1e-9,
          "Vehicle and station energy summed");
    check(park->get_vehicle_count() == 2 && park->get_equipment_count() == 2,
          "Mixed counts");
    check(
        park->get_vehicles().size() == 2 && park->get_equipments().size() == 2,
        "Separate lists");
    const auto items = park->get_inventory();
    check(items.size() == 4, "Full inventory size");
    check(items[0]->get_inventory_number() == "S" &&
              items[0]->get_name() == "Scooter",
          "Vehicle in inventory");
    check(items[3]->get_inventory_number() == "C" &&
              items[3]->get_name() == "Charger",
          "Equipment in inventory");
}

class AcceptingService : public InspectionService {
   public:
    bool inspect(const Vehicle&) const override { return true; }
};

void test_service_replacement_through_container() {
    RejectingService rejecting_service;
    Container rejecting_container(rejecting_service);
    auto rejecting_park = rejecting_container.create_park();
    check(!rejecting_park->add_vehicle(
              make_unique<Bicycle>("B", "Healthy", 6, true)),
          "Injected service rejects even healthy vehicle");
    check(rejecting_park->get_inventory().empty(), "Rejected park stays empty");
    check(rejecting_park->add_equipment(make_unique<Helmet>("H", "Helmet")),
          "Equipment does not need vehicle inspection");

    AcceptingService accepting_service;
    Container accepting_container(accepting_service);
    auto accepting_park = accepting_container.create_park();
    check(accepting_park->add_vehicle(
              make_unique<Bicycle>("B", "Broken", 6, false)),
          "Park obeys injected decision rather than checking brakes itself");
    auto other_park = accepting_container.create_park();
    check(other_park->get_inventory().empty(),
          "Created parks have independent inventories");
}

void test_invalid_data() {
    bool caught = false;
    try {
        Helmet item("", "Helmet");
    } catch (const invalid_argument&) {
        caught = true;
    }
    check(caught, "Empty number rejected");

    caught = false;
    try {
        Helmet item("H", "");
    } catch (const invalid_argument&) {
        caught = true;
    }
    check(caught, "Empty name rejected");

    for (int ease : {0, 11}) {
        caught = false;
        try {
            Bicycle bike("B", "Bike", ease, true);
        } catch (const invalid_argument&) {
            caught = true;
        }
        check(caught, "Out-of-range ease rejected");
    }
    for (double energy : {-1.0, numeric_limits<double>::infinity(),
                          numeric_limits<double>::quiet_NaN()}) {
        caught = false;
        try {
            ChargingStation station("C", "Station", energy);
        } catch (const invalid_argument&) {
            caught = true;
        }
        check(caught, "Invalid station energy rejected");
        caught = false;
        try {
            Scooter scooter("S", "Scooter", 7, energy, true);
        } catch (const invalid_argument&) {
            caught = true;
        }
        check(caught, "Invalid scooter energy rejected");
    }
    Scooter scooter("S", "Scooter", 7, 2.5, true);
    check(!scooter.set_ease(11) && scooter.get_ease() == 7,
          "Failed ease update preserves value");
    check(!scooter.set_daily_energy(-1) && scooter.get_daily_energy() == 2.5,
          "Failed energy update preserves value");
    check(!scooter.set_name("") && scooter.get_name() == "Scooter",
          "Failed name update preserves value");
    check(!scooter.set_inventory_number("") &&
              scooter.get_inventory_number() == "S",
          "Failed number update preserves value");
    check(scooter.set_daily_energy(0), "Zero energy allowed");
}

void test_null_objects() {
    ServiceCenter service;
    Container container(service);
    auto park = container.create_park();
    check(!park->add_vehicle(nullptr), "Null vehicle rejected");
    check(!park->add_equipment(nullptr), "Null equipment rejected");
    check(park->get_inventory().empty(), "Null input leaves inventory empty");
}

void test_rejection_messages() {
    RejectingService service;
    Container container(service);
    auto park = container.create_park();
    check(!park->add_vehicle(make_unique<Bicycle>("B", "Healthy", 6, true)),
          "Injected rejection");
    check(park->get_last_error() == "Vehicle failed technical inspection",
          "Inspection reason comes from failed service decision");
    check(park->add_equipment(make_unique<Helmet>("H", "Helmet")), "Add equipment");
    check(park->get_last_error().empty(), "Success clears old error");
    check(!park->add_vehicle(make_unique<Bicycle>("H", "Duplicate", 6, true)), "Duplicate");
    check(park->get_last_error() == "Inventory number already exists", "Duplicate checked first");
    check(!park->add_equipment(make_unique<Helmet>("H", "Duplicate")), "Duplicate equipment");
    check(park->get_last_error() == "Inventory number already exists", "Equipment duplicate reason");
    check(!park->add_vehicle(nullptr), "Null vehicle");
    check(park->get_last_error() == "No vehicle supplied", "Null vehicle reason");
    check(!park->add_equipment(nullptr), "Null equipment");
    check(park->get_last_error() == "No equipment supplied", "Null equipment reason");
}

int run_test(const string& name, void (*test)()) {
    try {
        test();
        cout << "PASS " << name << '\n';
        return 0;
    } catch (const exception& error) {
        cout << "FAIL " << name << ": " << error.what() << '\n';
        return 1;
    }
}

int main() {
    int failures = 0;
    failures += run_test("empty park", test_empty_park);
    failures += run_test("inspection", test_inspection);
    failures += run_test("duplicate numbers", test_duplicate_numbers);
    failures += run_test("beginner boundary", test_beginner_boundary);
    failures += run_test("energy and inventory", test_energy_and_inventory);
    failures += run_test("DI service replacement",
                         test_service_replacement_through_container);
    failures += run_test("invalid data", test_invalid_data);
    failures += run_test("null objects", test_null_objects);
    failures += run_test("rejection messages", test_rejection_messages);
    cout << 9 - failures << "/9 tests passed\n";
    return failures == 0 ? 0 : 1;
}
