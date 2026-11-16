#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

#include "adapters.hpp"

void check(bool condition) {
    if (!condition) throw std::runtime_error("Check failed.");
}

template <typename Action>
void rejects(Action action) {
    try {
        action();
    } catch (const std::invalid_argument&) {
        return;
    }
    throw std::runtime_error("Expected invalid_argument.");
}

void invalid_values() {
    rejects([] { Student student(1, "  "); });
    rejects([] { Criterion criterion(1, "Exam", 0, 1); });
    rejects([] { Criterion criterion(1, "Exam", 10, 0); });
    rejects([] { Criterion criterion(1, "Exam", 10, 2); });
    rejects([] { Score score(-1); });
    rejects([] { Score score(std::numeric_limits<double>::quiet_NaN()); });
    rejects([] {
        Criterion criterion(1, "Exam", std::numeric_limits<double>::infinity(),
                            1);
    });
}

void evaluation_rules() {
    Gradebook book;
    book.add_student("Alice");
    book.add_criterion("Exam", 10, 1);
    book.set_score(1, 1, 8);
    rejects([&] { book.set_score(1, 1, 11); });
    rejects([&] { book.set_score(1, 2, 5); });
    rejects([&] { book.set_score(2, 1, 5); });
    check(book.get_evaluations().size() == 1);
    check(book.get_evaluations()[0].get_score() == 8);
    book.set_score(1, 1, 0);
    check(book.get_evaluations().size() == 1);
    book.set_score(1, 1, 10);
    check(book.get_evaluations()[0].get_score() == 10);
    book.remove_score(1, 1);
    check(book.get_evaluations().empty());
    rejects([&] { book.remove_score(1, 1); });
}

void weighted_results() {
    Gradebook book;
    FinalStrategy strategy;
    FinalScoreService calculator(strategy);
    book.add_student("Alice");
    book.add_student("Bob");
    rejects([&] { calculator.calculate(book, 1); });
    book.add_criterion("Exam", 20, 0.6);
    book.add_criterion("Homework", 10, 0.3);
    book.set_score(1, 1, 10);
    book.set_score(1, 2, 10);
    check(std::abs(calculator.calculate(book, 1) - 20.0 / 3) < 1e-9);
    check(calculator.calculate(book, 2) == 0);
    book.remove_score(1, 2);
    check(std::abs(calculator.calculate(book, 1) - 10.0 / 3) < 1e-9);
    book.set_score(1, 1, 20);
    book.set_score(1, 2, 10);
    check(std::abs(calculator.calculate(book, 1) - 10) < 1e-9);
    rejects([&] { calculator.calculate(book, 99); });
}

class TestExporter : public ResultsExporter {
   public:
    mutable double result = -1;
    void write(const Gradebook& book, const FinalScoreService& calculator,
               const std::string&) const override {
        result = calculator.calculate(book, 1);
    }
};

void dependency_injection() {
    MemoryRepository repository;
    FinalStrategy strategy;
    FinalScoreService calculator(strategy);
    TestExporter exporter;
    Application app(repository, calculator, exporter);
    app.add_student("Alice");
    app.add_criterion("Exam", 10, 1);
    app.set_score(1, 1, 7);
    app.export_results("unused");
    check(exporter.result == 7);
    Container first;
    Container second;
    auto one = first.create_application();
    auto two = second.create_application();
    one.add_student("Bob");
    check(two.get_book().get_students().empty());
}

void rubric_builder() {
    Gradebook book;
    RubricBuilder builder;
    rejects([&] { book.set_rubric(builder); });
    builder.add("Exam", 20, 0.6);
    rejects([&] { builder.add("Bad", 0, 0.4); });
    check(book.get_criteria().empty());
    builder.add("Homework", 10, 0.4);
    book.set_rubric(builder);
    check(book.get_criteria().size() == 2);
    check(book.find_criterion(2).get_name() == "Homework");
    book.add_student("Alice");
    book.set_score(1, 1, 10);
    rejects([&] { book.set_rubric(builder); });
    check(book.get_evaluations()[0].get_score() == 10);
    RubricBuilder excessive;
    excessive.add("A", 10, 0.8);
    excessive.add("B", 10, 0.8);
    Gradebook batch;
    batch.set_rubric(excessive);
    Gradebook individual;
    individual.add_criterion("A", 10, 0.8);
    individual.add_criterion("B", 10, 0.8);
    batch.add_student("Alice");
    individual.add_student("Alice");
    batch.set_score(1, 1, 5);
    individual.set_score(1, 1, 5);
    FinalStrategy strategy;
    FinalScoreService calculator(strategy);
    check(std::abs(calculator.calculate(batch, 1) - 2.5) < 1e-9);
    check(calculator.calculate(batch, 1) ==
          calculator.calculate(individual, 1));
}

void calculation_strategies() {
    Container container;
    auto app = container.create_application();
    app.add_student("Alice");
    app.add_criterion("Exam", 20, 0.6);
    app.add_criterion("Homework", 10, 0.4);
    app.set_score(1, 1, 10);
    check(std::abs(app.final_score(1) - 3) < 1e-9);
    app.set_strategy(container.get_preliminary_strategy());
    check(app.mode_name() == "Preliminary");
    check(std::abs(app.final_score(1) - 5) < 1e-9);
    app.set_score(1, 2, 0);
    check(std::abs(app.final_score(1) - 3) < 1e-9);
    app.set_score(1, 2, 10);
    check(std::abs(app.final_score(1) - 7) < 1e-9);
    app.set_strategy(container.get_final_strategy());
    check(std::abs(app.final_score(1) - 7) < 1e-9);
    app.remove_score(1, 1);
    app.remove_score(1, 2);
    app.set_strategy(container.get_preliminary_strategy());
    check(app.final_score(1) == 0);
}

int main() {
    try {
        rubric_builder();
        std::cout << "PASS rubric builder\n";
        calculation_strategies();
        std::cout << "PASS calculation strategies\n";
        invalid_values();
        std::cout << "PASS invalid values\n";
        evaluation_rules();
        std::cout << "PASS evaluation rules\n";
        weighted_results();
        std::cout << "PASS weighted results\n";
        dependency_injection();
        std::cout << "PASS dependency injection\n";
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
