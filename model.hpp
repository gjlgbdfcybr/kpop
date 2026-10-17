#pragma once
#include <memory>
#include <string>
#include <vector>

class Inventoriable {
   public:
    virtual std::string get_inventory_number() const = 0;
    virtual std::string get_name() const = 0;
    virtual ~Inventoriable() = default;
};

class InventoryItem : public Inventoriable {
   private:
    std::string inventory_number;
    std::string name;

   public:
    InventoryItem(std::string number, std::string title);

    virtual ~InventoryItem() = default;

    bool set_name(std::string value);

    std::string get_name() const override;

    bool set_inventory_number(std::string value);

    std::string get_inventory_number() const override;
};

class Equipment : public InventoryItem {
   public:
    Equipment(std::string number, std::string title);
};

class Helmet : public Equipment {
   public:
    Helmet(std::string number, std::string title);
};

class EnergyConsumer {
   public:
    virtual double get_daily_energy() const = 0;
    virtual ~EnergyConsumer() = default;
};

class ChargingStation : public Equipment, public EnergyConsumer {
    double daily_energy;

   public:
    ChargingStation(std::string number, std::string title, double energy);

    double get_daily_energy() const override;
};

class Vehicle : public InventoryItem {
    int ease = 1;
    bool brakes_ok;

   public:
    Vehicle(std::string number, std::string title, int ease_value, bool brakes);
    bool set_ease(int value);

    int get_ease() const;

    bool has_working_brakes() const;

    bool is_beginner_friendly() const;
};

class Bicycle : public Vehicle {
   public:
    Bicycle(std::string number, std::string title, int ease_value, bool brakes);
};

class Scooter : public Vehicle, public EnergyConsumer {
   private:
    double daily_energy = 0;

   public:
    Scooter(std::string number, std::string title, int ease_value,
            double energy, bool brakes);

    bool set_daily_energy(double value);

    double get_daily_energy() const override;
};

class InspectionService {
   public:
    virtual bool inspect(const Vehicle& vehicle) const = 0;
    virtual ~InspectionService() = default;
};

class RejectingService : public InspectionService {
   public:
    bool inspect(const Vehicle&) const override;
};

class ServiceCenter : public InspectionService {
   public:
    bool inspect(const Vehicle& vehicle) const override;
};

class Park {
   private:
    std::vector<std::unique_ptr<Vehicle>> vehicles;
    std::vector<std::unique_ptr<Equipment>> equipments;
    const InspectionService& service_center;
    std::string last_error;

    bool has_number(const std::string& number) const;

   public:
    Park(const InspectionService& service);

    bool add_vehicle(std::unique_ptr<Vehicle> vehicle);

    bool add_equipment(std::unique_ptr<Equipment> equipment);

    std::string get_last_error() const;

    int get_vehicle_count() const;
    int get_equipment_count() const;

    double get_total_energy() const;

    std::vector<const Inventoriable*> get_inventory() const;

    std::vector<const Equipment*> get_equipments() const;

    std::vector<const Vehicle*> get_vehicles() const;

    std::vector<const Vehicle*> get_beginner_vehicles() const;
};

class Container {
    const InspectionService& service_center;

   public:
    Container(const InspectionService& service);

    std::unique_ptr<Park> create_park() const;
};
