#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cctype>
#include <algorithm>
#include <limits>
#include <cstdlib>

#define MAX_NAME_LEN 50
#define MAX_QUESTIONS 100
#define MAX_TEXT_LEN 200
#define HISTORY_FILE "history.txt"
#define QUESTIONS_FILE "questions.txt"
#define MAX_HISTORY 100

#define GREEN "\x1B[32m"
#define RED "\x1B[31m"
#define BLUE "\x1B[34m"
#define YELLOW "\x1B[33m"
#define RESET "\x1B[0m"

class Question {
public:
    std::string question;
    std::string options[4];
    char correct;
};

class HistoryEntry {
public:
    std::string name;
    float score;
};

class QuizGameManager {
private:
    std::vector<Question> questions;
    std::vector<HistoryEntry> history;
    const std::string historyFile = "history.txt";
    const std::string questionsFile = "questions.txt";

    void clearScreen() const {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
    }

    void pauseAndClear() const {
        std::cout << "\nPress ENTER to continue...";
        getchar();
        clearScreen();
    }

    void saveHistory(const std::string& name, float score) {
        std::ofstream file(historyFile, std::ios::app);
        if (!file) {
            std::cout << "Error opening history file!\n";
            return;
        }
        file << name << " " << score << "\n";
    }

    void loadHistory() {
        history.clear();
        std::ifstream file(historyFile);
        if (!file) return;
        std::string name;
        float score;
        while (file >> name >> score) {
            history.push_back({name, score});
            if (history.size() >= MAX_HISTORY) break;
        }
    }

    void viewHistory() {
        clearScreen();
        std::cout << "=== Game History ===\n";
        loadHistory();
        if (history.empty()) {
            std::cout << "No history found.\n";
        } else {
            for (const auto& entry : history) {
                std::cout << "Player: " << entry.name << ", Score: " << entry.score << "\n";
            }
        }
        pauseAndClear();
    }

    void viewLeaderboard() {
        clearScreen();
        std::cout << "=== Leaderboard ===\n";
        loadHistory();
        if (history.empty()) {
            std::cout << "No players yet.\n";
        } else {
            std::sort(history.begin(), history.end(), [](const HistoryEntry& a, const HistoryEntry& b) {
                return b.score > a.score;
            });
            size_t top = history.size() < 5 ? history.size() : 5;
            for (size_t i = 0; i < top; i++) {
                std::cout << i + 1 << ". " BLUE << history[i].name << RESET " - " << history[i].score << " points\n";
            }
        }
        pauseAndClear();
    }

    void help() const {
        clearScreen();
        std::cout << "=== Help ===\n";
        std::cout << "1. Play Game: Answer quiz questions from file. One wrong answer = game over.\n";
        std::cout << "2. Game History: See all past player scores.\n";
        std::cout << "3. Leaderboard: View top 5 players with highest scores.\n";
        std::cout << "4. Help: Shows this help menu.\n";
        std::cout << "5. Exit: Quit the application.\n";
        pauseAndClear();
    }

    int loadQuestions() {
        std::ifstream file(questionsFile);
        if (!file) {
            std::cout << "Error opening questions file!\n";
            return 0;
        }
        questions.clear();
        std::string questionText;
        while (std::getline(file, questionText)) {
            Question q;
            q.question = questionText;
            bool valid = true;
            for (int i = 0; i < 4; i++) {
                if (!std::getline(file, q.options[i])) {
                    valid = false;
                    break;
                }
            }
            std::string correctLine;
            if (!std::getline(file, correctLine) || correctLine.empty()) valid = false;
            if (valid) {
                q.correct = toupper(correctLine[0]);
                questions.push_back(q);
                if (questions.size() >= MAX_QUESTIONS) break;
            }
        }
        return questions.size();
    }

    void playGame() {
        clearScreen();
        std::cout << "=== Play Game ===\n";

        if (questions.empty()) {
            std::cout << RED "Error: No questions loaded. Cannot start the game.\n" RESET;
            pauseAndClear();
            return;
        }

        std::string name;
        int valid = 0;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        while (!valid) {
            std::cout << "Enter your name (letters only): ";
            std::getline(std::cin, name);
            valid = 1;
            for (char c : name) {
                if (!isalpha(c)) {
                    valid = 0;
                    break;
                }
            }
            if (!valid) {
                std::cout << RED "Invalid input. Please use letters only.\n" RESET;
            }
        }
        float score = 0;
        for (size_t i = 0; i < questions.size(); i++) {
            std::cout << "\nQ" << i + 1 << ": " YELLOW << questions[i].question << RESET "\n";
            for (int j = 0; j < 4; j++) {
                std::cout << j + 1 << ". " BLUE << questions[i].options[j] << RESET "\n";
            }
            std::cout << "Your answer (A/B/C/D, or H for 50/50 help): ";
            char answer;
            std::cin >> answer;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            if (answer == 'H' || answer == 'h') {
                std::cout << "50/50 Help: The correct answer is " << questions[i].correct << ".\n";
                std::cout << "You now get 0.5 points if correct.\n";
                std::cout << "Your answer (A/B/C/D): ";
                std::cin >> answer;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                 if (toupper(answer) == questions[i].correct) {
                    std::cout << GREEN "CORRECT!" RESET "\n";
                    std::cout << "+0.5 points.\n";
                    score += 0.5f;
                } else {
                    std::cout << RED "Wrong! The correct answer was " << questions[i].correct << ".\n" RESET;
                    std::cout << "\nGame over! You scored " << score << " points.\n";
                    saveHistory(name, score);
                    pauseAndClear();
                    return;
                }
            } else {
                if (toupper(answer) == questions[i].correct) {
                    std::cout << GREEN "CORRECT!" RESET "\n";
                    std::cout << "+1 point.\n";
                    score += 1.0f;
                } else {
                    std::cout << RED "Wrong! The correct answer was " << questions[i].correct << ".\n" RESET;
                    std::cout << "\nGame over! You scored " << score << " points.\n";
                    saveHistory(name, score);
                    pauseAndClear();
                    return;
                }
            }
        }
        std::cout << "\nCongratulations! You finished all questions. Total score: " << score << " points.\n";
        saveHistory(name, score);
        pauseAndClear();
    }


public:
    void run(int argc, char* argv[]) {
        clearScreen();

        if (argc > 1) {
            std::string arg = argv[1];
            if (arg == "--history") {
                loadHistory();
                viewHistory();
                return;
            } else if (arg == "--leaderboard") {
                loadHistory();
                viewLeaderboard();
                return;
            } else if (arg == "--help") {
                 help();
                 return;
            } else {
                 std::cout << "Unknown argument: " << arg << "\n";
                 std::cout << "Usage: " << argv[0] << " [OPTION]\n";
                 std::cout << "Options:\n";
                 std::cout << "  --history    View game history\n";
                 std::cout << "  --leaderboard  View leaderboard\n";
                 std::cout << "  --help       Show help menu\n";
                 pauseAndClear();
            }
        }


        if (loadQuestions() == 0) {
            std::cout << "No questions loaded. Please check '" << questionsFile << "' file.\n";
            return;
        }

        int choice;
        do {
            std::cout << "=== Main Menu ===\n";
            std::cout << "1. " BLUE "Play Game" RESET "\n";
            std::cout << "2. " BLUE "Game History" RESET "\n";
            std::cout << "3. " BLUE "Leaderboard" RESET "\n";
            std::cout << "4. " BLUE "Help" RESET "\n";
            std::cout << "5. " BLUE "Exit" RESET "\n";
            std::cout << "Choose an option: ";
            if (!(std::cin >> choice)) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Invalid input. Redirecting to Help...\n";
                help();
                continue;
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');


            switch (choice) {
                case 1:
                    playGame();
                    break;
                case 2:
                    viewHistory();
                    break;
                case 3:
                    viewLeaderboard();
                    break;
                case 4:
                    help();
                    break;
                case 5:
                    std::cout << "Exiting... Goodbye!\n";
                    break;
                default:
                    std::cout << "Invalid option. Redirecting to Help...\n";
                    help();
            }
        } while (choice != 5);
    }
};

int main(int argc, char* argv[]) {
    QuizGameManager gameManager;
    gameManager.run(argc, argv);
    return 0;
}