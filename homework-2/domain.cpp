#include "domain.hpp"

#include <cmath>
#include <stdexcept>

namespace {
void check_name(const std::string& name) {
    if (name.find_first_not_of(" \t\r\n") == std::string::npos)
        throw std::invalid_argument("Name must not be empty.");
}
}  // namespace

Score::Score(double value) : value(value) {
    if (!std::isfinite(value) || value < 0)
        throw std::invalid_argument("Score must be finite and nonnegative.");
}
double Score::get_value() const { return value; }

Student::Student(int id, const std::string& name) : id(id), name(name) {
    if (id <= 0) throw std::invalid_argument("Invalid student ID.");
    check_name(name);
}
int Student::get_id() const { return id; }
const std::string& Student::get_name() const { return name; }

Criterion::Criterion(int id, const std::string& name, double maximum,
                     double weight)
    : id(id), name(name), maximum(maximum), weight(weight) {
    if (id <= 0) throw std::invalid_argument("Invalid criterion ID.");
    check_name(name);
    if (!std::isfinite(maximum) || maximum <= 0)
        throw std::invalid_argument("Maximum must be finite and positive.");
    if (!std::isfinite(weight) || weight <= 0 || weight > 1)
        throw std::invalid_argument("Weight must be in (0, 1].");
}
int Criterion::get_id() const { return id; }
const std::string& Criterion::get_name() const { return name; }
double Criterion::get_maximum() const { return maximum; }
double Criterion::get_weight() const { return weight; }

Evaluation::Evaluation(const Student& student, const Criterion& criterion,
                       Score score)
    : student_id(student.get_id()),
      criterion_id(criterion.get_id()),
      score(score) {
    if (score.get_value() > criterion.get_maximum())
        throw std::invalid_argument("Score exceeds criterion maximum.");
}
int Evaluation::get_student_id() const { return student_id; }
int Evaluation::get_criterion_id() const { return criterion_id; }
double Evaluation::get_score() const { return score.get_value(); }

void Gradebook::add_student(const std::string& name) {
    students.emplace_back(static_cast<int>(students.size()) + 1, name);
}
void Gradebook::add_criterion(const std::string& name, double maximum,
                              double weight) {
    criteria.emplace_back(static_cast<int>(criteria.size()) + 1, name, maximum,
                          weight);
}
const Student& Gradebook::find_student(int id) const {
    for (const auto& student : students)
        if (student.get_id() == id) return student;
    throw std::invalid_argument("Student not found.");
}
const Criterion& Gradebook::find_criterion(int id) const {
    for (const auto& criterion : criteria)
        if (criterion.get_id() == id) return criterion;
    throw std::invalid_argument("Criterion not found.");
}
void Gradebook::set_score(int student_id, int criterion_id, double score) {
    Evaluation replacement(find_student(student_id),
                           find_criterion(criterion_id), Score(score));
    for (auto& evaluation : evaluations) {
        if (evaluation.get_student_id() == student_id &&
            evaluation.get_criterion_id() == criterion_id) {
            evaluation = replacement;
            return;
        }
    }
    evaluations.push_back(replacement);
}
void Gradebook::remove_score(int student_id, int criterion_id) {
    find_student(student_id);
    find_criterion(criterion_id);
    for (auto it = evaluations.begin(); it != evaluations.end(); ++it) {
        if (it->get_student_id() == student_id &&
            it->get_criterion_id() == criterion_id) {
            evaluations.erase(it);
            return;
        }
    }
    throw std::invalid_argument("Evaluation not found.");
}
const std::vector<Student>& Gradebook::get_students() const { return students; }
const std::vector<Criterion>& Gradebook::get_criteria() const {
    return criteria;
}
const std::vector<Evaluation>& Gradebook::get_evaluations() const {
    return evaluations;
}

double FinalScoreService::calculate(const Gradebook& book,
                                    int student_id) const {
    book.find_student(student_id);
    if (book.get_criteria().empty())
        throw std::invalid_argument("Add at least one criterion first.");
    double total_weight = 0;
    double result = 0;
    for (const auto& criterion : book.get_criteria()) {
        bool found = false;
        double score = 0;
        for (const auto& evaluation : book.get_evaluations()) {
            if (evaluation.get_student_id() == student_id &&
                evaluation.get_criterion_id() == criterion.get_id()) {
                found = true;
                score = evaluation.get_score();
                break;
            }
        }
        double weight = strategy->counted_weight(criterion.get_weight(), found);
        total_weight += weight;
        result += score / criterion.get_maximum() * weight;
    }
    if (total_weight == 0) return 0;
    return result / total_weight * 10;
}

void RubricBuilder::add(const std::string& name, double maximum,
                        double weight) {
    criteria.emplace_back(static_cast<int>(criteria.size()) + 1, name, maximum,
                          weight);
}
std::vector<Criterion> RubricBuilder::build() const {
    if (criteria.empty())
        throw std::invalid_argument("Rubric must not be empty.");
    return criteria;
}
void Gradebook::set_rubric(const RubricBuilder& builder) {
    if (!criteria.empty())
        throw std::invalid_argument(
            "Batch rubric can only be installed in an empty rubric.");
    criteria = builder.build();
}
double FinalStrategy::counted_weight(double weight, bool) const {
    return weight;
}
std::string FinalStrategy::name() const { return "Final"; }
double PreliminaryStrategy::counted_weight(double weight,
                                           bool has_score) const {
    return has_score ? weight : 0;
}
std::string PreliminaryStrategy::name() const { return "Preliminary"; }
FinalScoreService::FinalScoreService(const ScoreStrategy& strategy)
    : strategy(&strategy) {}
void FinalScoreService::set_strategy(const ScoreStrategy& value) {
    strategy = &value;
}
std::string FinalScoreService::mode_name() const { return strategy->name(); }
