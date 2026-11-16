#include "adapters.hpp"

#include <fstream>
#include <iomanip>
#include <locale>
#include <stdexcept>

Gradebook& MemoryRepository::get() { return book; }
void CsvExporter::write(const Gradebook& book,
                        const FinalScoreService& calculator,
                        const std::string& path) const {
    if (book.get_criteria().empty())
        throw std::invalid_argument("Add at least one criterion first.");
    if (path.size() < 4 || path.substr(path.size() - 4) != ".csv")
        throw std::invalid_argument("Export filename must end with .csv.");
    std::ofstream output(path);
    if (!output) throw std::runtime_error("Cannot open export file.");
    output.imbue(std::locale::classic());
    output << "student_id,name,score,mode\n"
           << std::fixed << std::setprecision(2);
    for (const auto& student : book.get_students()) {
        output << student.get_id() << ",\"";
        for (char ch : student.get_name()) {
            if (ch == '"') output << '"';
            output << ch;
        }
        output << "\"," << calculator.calculate(book, student.get_id()) << ','
               << calculator.mode_name() << '\n';
    }
    output.close();
    if (!output) throw std::runtime_error("Cannot finish writing export file.");
}
Application Container::create_application() {
    return Application(repository, calculator, exporter);
}

const ScoreStrategy& Container::get_final_strategy() const {
    return final_strategy;
}
const ScoreStrategy& Container::get_preliminary_strategy() const {
    return preliminary_strategy;
}
