#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>
using namespace std;
int main(int argc, char* argv[]) {
    std::ifstream infile("text_dataset/processed_text_dataset.txt");
    if (!infile.is_open()) {
        std::cerr << "Failed to open file.\n";
        return 1;
    }

    int doc_id = 1;
    const int limit = stoi(argv[1]);
    int querysize=1000;
    std::unordered_map<std::string, int> word_idf;
    std::vector<std::unordered_map<std::string, int>> doc_word_cnt;
    doc_word_cnt.push_back({});  // Initialize with an empty map for doc_id 0
    std::vector<int> total_cnt;
    total_cnt.push_back(0);  // Initialize with 0 for doc_id 0
    std::string line;
    while (std::getline(infile, line)) {
        std::unordered_set<std::string> vis;
        std::unordered_map<std::string, int> word_cnt;
        int total = 0;
        std::istringstream iss(line);
        std::string word;
        while (iss >> word) {
            total++;
            if (vis.find(word) == vis.end()) {
                vis.insert(word);
                word_cnt[word] = 1;
                word_idf[word]++;
            } else {
                word_cnt[word]++;
            }
        }
        total_cnt.push_back(total);
        doc_word_cnt.push_back(std::move(word_cnt));
        std::cout << doc_id << " done\n";
        if (doc_id == limit) break;
        doc_id++;
    }

    infile.close();
    string filename="text_dataset/text_actual_sim_" + std::to_string(limit) + ".txt";
    std::ofstream result(filename);
    if (!result.is_open()) {
        std::cerr << "Failed to open output file.\n";
        return 1;
    }

    std::unordered_map<long long, double> doc_sim;

    for (int i = 1; i <= querysize; ++i) {
        for (int j = i + 1; j <= querysize; ++j) {
            long long code = static_cast<long long>(i) * (100001) + j;
            double min_weight_sum = 0.0;
            double max_weight_sum = 0.0;

            // 先遍历 doc i 中的词
            for (const auto& [word, cnt] : doc_word_cnt[i]) {
                double tf1=(double)cnt/total_cnt[i];
                int df = word_idf[word];
                double idf = std::log2(static_cast<double>(doc_id) / (df + 1));
                double w1 = tf1 * idf;
                double tf2 = doc_word_cnt[j].count(word) ? doc_word_cnt[j].at(word) : 0;
                tf2= (double)tf2 / total_cnt[j];
                double w2 = tf2 * idf;

                min_weight_sum += std::min(w1, w2);
                max_weight_sum += std::max(w1, w2);
            }

            // 再补充 doc j 中 doc i 没有的词
            for (const auto& [word, cnt] : doc_word_cnt[j]) {
                if (doc_word_cnt[i].count(word)) continue;
                double tf2 = (double)cnt / total_cnt[j];
                int df = word_idf[word];
                double idf = std::log2(static_cast<double>(doc_id) / (df + 1));
                double w1 = 0.0;
                double w2 = tf2 * idf;

                min_weight_sum += std::min(w1, w2);  // 即 0
                max_weight_sum += std::max(w1, w2);  // 即 w2
            }

            double sim = (max_weight_sum > 0) ? (min_weight_sum / max_weight_sum) : 0.0;
            doc_sim[code] = sim;

            result << code << " " << std::fixed << std::setprecision(6) << sim << "\n";
        }
    }

    result.close();
    std::cout << "All similarities computed and saved.\n";
    return 0;
}
