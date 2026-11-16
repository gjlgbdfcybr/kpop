#pragma once
#include "application.hpp"

class MemoryRepository : public GradebookRepository {
    Gradebook book;

   public:
    Gradebook& get() override;
};

class CsvExporter : public ResultsExporter {
   public:
    void write(const Gradebook& book, const FinalScoreService& calculator,
               const std::string& path) const override;
};

class Container {
    MemoryRepository repository;
    FinalStrategy final_strategy;
    PreliminaryStrategy preliminary_strategy;
    FinalScoreService calculator{final_strategy};
    CsvExporter exporter;

   public:
    Container() = default;
    Container(const Container&) = delete;
    Container& operator=(const Container&) = delete;
    Application create_application();
    const ScoreStrategy& get_final_strategy() const;
    const ScoreStrategy& get_preliminary_strategy() const;
};
