#pragma once
#include <string>
#include <vector>

class Score {
    double value;

   public:
    explicit Score(double value);
    double get_value() const;
};

class Student {
    int id;
    std::string name;

   public:
    Student(int id, const std::string& name);
    int get_id() const;
    const std::string& get_name() const;
};

class Criterion {
    int id;
    std::string name;
    double maximum;
    double weight;

   public:
    Criterion(int id, const std::string& name, double maximum, double weight);
    int get_id() const;
    const std::string& get_name() const;
    double get_maximum() const;
    double get_weight() const;
};

class RubricBuilder {
    std::vector<Criterion> criteria;

   public:
    void add(const std::string& name, double maximum, double weight);
    std::vector<Criterion> build() const;
};

class Evaluation {
    int student_id;
    int criterion_id;
    Score score;

   public:
    Evaluation(const Student& student, const Criterion& criterion, Score score);
    int get_student_id() const;
    int get_criterion_id() const;
    double get_score() const;
};

class Gradebook {
    std::vector<Student> students;
    std::vector<Criterion> criteria;
    std::vector<Evaluation> evaluations;

   public:
    void set_rubric(const RubricBuilder& builder);
    void add_student(const std::string& name);
    void add_criterion(const std::string& name, double maximum, double weight);
    void set_score(int student_id, int criterion_id, double score);
    void remove_score(int student_id, int criterion_id);
    const Student& find_student(int id) const;
    const Criterion& find_criterion(int id) const;
    const std::vector<Student>& get_students() const;
    const std::vector<Criterion>& get_criteria() const;
    const std::vector<Evaluation>& get_evaluations() const;
};

class ScoreStrategy {
   public:
    virtual double counted_weight(double weight, bool has_score) const = 0;
    virtual std::string name() const = 0;
    virtual ~ScoreStrategy() = default;
};

class FinalStrategy : public ScoreStrategy {
   public:
    double counted_weight(double weight, bool has_score) const override;
    std::string name() const override;
};

class PreliminaryStrategy : public ScoreStrategy {
   public:
    double counted_weight(double weight, bool has_score) const override;
    std::string name() const override;
};

class FinalScoreService {
    const ScoreStrategy* strategy;

   public:
    explicit FinalScoreService(const ScoreStrategy& strategy);
    void set_strategy(const ScoreStrategy& value);
    std::string mode_name() const;
    double calculate(const Gradebook& book, int student_id) const;
};

class GradebookRepository {
   public:
    virtual Gradebook& get() = 0;
    virtual ~GradebookRepository() = default;
};
