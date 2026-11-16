#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "adapters.hpp"

std::string read_text(const std::string& prompt) {
    std::cout << prompt;
    std::string text;
    if (!std::getline(std::cin, text))
        throw std::runtime_error("Input closed.");
    return text;
}
template <typename T>
T read_number(const std::string& prompt) {
    std::istringstream input(read_text(prompt));
    T value;
    char extra;
    if (!(input >> value) || (input >> extra))
        throw std::invalid_argument("Enter a valid number.");
    return value;
}
void show_lists(const Application& app) {
    const auto& book = app.get_book();
    std::cout << "Students:\n";
    for (const auto& student : book.get_students())
        std::cout << student.get_id() << " | " << student.get_name() << '\n';
    std::cout << "Criteria:\n";
    for (const auto& criterion : book.get_criteria())
        std::cout << criterion.get_id() << " | " << criterion.get_name()
                  << " | Maximum: " << criterion.get_maximum()
                  << " | Weight: " << criterion.get_weight() << '\n';
}
int main() {
    Container container;
    Application app = container.create_application();
    std::cout << std::fixed << std::setprecision(2);
    std::cout
        << "POS: teacher evaluation of students\nData is kept until exit.\n";
    while (true) {
        std::cout << "\n1 Add student\n2 Add criterion\n3 Show students and "
                     "criteria\n"
                     "4 Set or correct score\n5 Remove score\n6 Show scores\n"
                     "7 Show results\n8 Export results to CSV\n9 Select "
                     "calculation mode\n10 Build rubric\n0 Exit\n";
        try {
            int choice = read_number<int>("Choice: ");
            if (choice == 0) break;
            if (choice == 1) {
                app.add_student(read_text("Student name: "));
                std::cout << "Student added.\n";
            }
            else if (choice == 2) {
                std::string name = read_text("Criterion name: ");
                double maximum = read_number<double>("Maximum score: ");
                double weight = read_number<double>("Weight (0, 1]: ");
                app.add_criterion(name, maximum, weight);
                std::cout << "Criterion added.\n";
            }
            else if (choice == 3) {
                show_lists(app);
            }
            else if (choice == 4 || choice == 5) {
                int student = read_number<int>("Student ID: ");
                int criterion = read_number<int>("Criterion ID: ");
                if (choice == 4)
                    app.set_score(student, criterion,
                                  read_number<double>("Score: "));
                else
                    app.remove_score(student, criterion);
                std::cout << "Done.\n";
            }
            else if (choice == 6) {
                const auto& book = app.get_book();
                if (book.get_evaluations().empty())
                    std::cout << "No scores yet.\n";
                for (const auto& evaluation : book.get_evaluations())
                    std::cout
                        << book.find_student(evaluation.get_student_id())
                               .get_name()
                        << " | "
                        << book.find_criterion(evaluation.get_criterion_id())
                               .get_name()
                        << " | " << evaluation.get_score() << '\n';
            }
            else if (choice == 7) {
                std::cout << "Mode: " << app.mode_name() << '\n';
                if (app.get_book().get_students().empty())
                    std::cout << "No students yet.\n";
                for (const auto& student : app.get_book().get_students()) {
                    double result = app.final_score(student.get_id());
                    std::cout << student.get_id() << " | " << student.get_name()
                              << " | " << result << " / 10\n";
                }
            }
            else if (choice == 8) {
                app.export_results(read_text(
                    "CSV path (existing file will be overwritten): "));
                std::cout << "Exported.\n";
            }
            else if (choice == 9) {
                int mode = read_number<int>("1 Final, 2 Preliminary: ");
                if (mode == 1)
                    app.set_strategy(container.get_final_strategy());
                else if (mode == 2)
                    app.set_strategy(container.get_preliminary_strategy());
                else
                    throw std::invalid_argument("Choose mode 1 or 2.");
                std::cout << "Mode: " << app.mode_name() << '\n';
            }
            else if (choice == 10) {
                if (!app.get_book().get_criteria().empty())
                    throw std::invalid_argument(
                        "Rubric must be empty. Use option 2 to add criteria.");
                int count = read_number<int>("Number of criteria: ");
                if (count <= 0)
                    throw std::invalid_argument("Count must be positive.");
                RubricBuilder builder;
                for (int i = 0; i < count; ++i) {
                    std::string name = read_text("Criterion name: ");
                    double maximum = read_number<double>("Maximum score: ");
                    double weight = read_number<double>("Weight (0, 1]: ");
                    builder.add(name, maximum, weight);
                }
                app.set_rubric(builder);
                std::cout << "Rubric installed.\n";
            }
            else {
                std::cout << "Choose a number from 0 to 10.\n";
            }
        } catch (const std::exception& error) {
            if (!std::cin) break;
            std::cout << "Error: " << error.what() << '\n';
        }
    }
}
