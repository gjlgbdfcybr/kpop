#pragma once
#include "domain.hpp"

class ResultsExporter {
   public:
    virtual void write(const Gradebook& book,
                       const FinalScoreService& calculator,
                       const std::string& path) const = 0;
    virtual ~ResultsExporter() = default;
};

class Application {
    GradebookRepository& repository;
    FinalScoreService& calculator;
    const ResultsExporter& exporter;

   public:
    Application(GradebookRepository& repository, FinalScoreService& calculator,
                const ResultsExporter& exporter);
    void set_rubric(const RubricBuilder& builder);
    void set_strategy(const ScoreStrategy& strategy);
    std::string mode_name() const;
    void add_student(const std::string& name);
    void add_criterion(const std::string& name, double maximum, double weight);
    void set_score(int student_id, int criterion_id, double score);
    void remove_score(int student_id, int criterion_id);
    double final_score(int student_id) const;
    void export_results(const std::string& path) const;
    const Gradebook& get_book() const;
};
