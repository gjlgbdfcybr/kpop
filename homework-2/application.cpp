#include "application.hpp"

Application::Application(GradebookRepository& repository,
                         FinalScoreService& calculator,
                         const ResultsExporter& exporter)
    : repository(repository), calculator(calculator), exporter(exporter) {}
void Application::add_student(const std::string& name) {
    repository.get().add_student(name);
}
void Application::add_criterion(const std::string& name, double maximum,
                                double weight) {
    repository.get().add_criterion(name, maximum, weight);
}
void Application::set_score(int student_id, int criterion_id, double score) {
    repository.get().set_score(student_id, criterion_id, score);
}
void Application::remove_score(int student_id, int criterion_id) {
    repository.get().remove_score(student_id, criterion_id);
}
double Application::final_score(int student_id) const {
    return calculator.calculate(repository.get(), student_id);
}
void Application::export_results(const std::string& path) const {
    exporter.write(repository.get(), calculator, path);
}
const Gradebook& Application::get_book() const { return repository.get(); }

void Application::set_rubric(const RubricBuilder& builder) {
    repository.get().set_rubric(builder);
}
void Application::set_strategy(const ScoreStrategy& strategy) {
    calculator.set_strategy(strategy);
}
std::string Application::mode_name() const { return calculator.mode_name(); }
