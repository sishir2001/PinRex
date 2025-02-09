#include <iostream>
#include <string>
#include <vector>
#include <cstdlib> // * for stoi
#include <stdexcept> // * for exception
#include <fstream> // * for ifstream, ofstream
#include <nlohmann/json.hpp>
#include <array>
#include <memory>
#include <algorithm>
#include <numeric> // * for accumulate function
#include <utility>
#include <set>
#include <regex>
#include "utils.hpp"

using namespace std;
using json = nlohmann::json;
using namespace pinrex;

const string APP_VERSION = "1.2.2";
const string JSON_EXTENSION = ".json";
const size_t JSON_EXTENSION_LENGTH = JSON_EXTENSION.length();
const string ANY_DIGIT_REGEX = "[0-9]";


class Node{
    public:
        int height;
        int childCount;
        array<unique_ptr<Node>,10> children;

        Node(int h) : height(h), childCount(0), children{} {}

        void addChild(int index, unique_ptr<Node> child){
            if(index >= 0 && index < 10){
                children[index] = std::move(child);
                childCount++;
            }
            else{
                throw invalid_argument("Index out of bounds");
            }
        }
};

// build a tree from a list of postal codes
Node* buildTreeFromPostalCodes(const vector<string>& postalCodes){
    Logger::log("Starting tree construction from postal codes", "buildTreeFromPostalCodes", LogLevel::DEBUG);
    Node* root = new Node(0);
    ProgressBar progress(postalCodes.size());
    
    for(size_t i = 0; i < postalCodes.size(); ++i) {
        const auto& postalCode = postalCodes[i];
        try {
            Node* currentNode = root;
            for(const char& digit : postalCode) {
                int index = digit - '0';
                if(index >= 0 && index < 10) {
                    if(!currentNode->children[index]) {
                        Node* child = new Node(currentNode->height + 1);
                        currentNode->addChild(index, unique_ptr<Node>(child));
                        currentNode = child;
                    } else {
                        currentNode = currentNode->children[index].get();
                    }
                } else {
                    string error = "Invalid postal code digit: " + string(1,digit);
                    Logger::log(error, "buildTreeFromPostalCodes", LogLevel::ERROR);
                    throw runtime_error(error);
                }
            }
        } catch (const exception& e) {
            Logger::log("Error processing postal code: " + postalCode + " - " + e.what(), "buildTreeFromPostalCodes", LogLevel::ERROR);
            throw;
        }
        progress.update(i + 1);
    }
    progress.finish();
    Logger::log("Tree construction completed successfully", "buildTreeFromPostalCodes", LogLevel::DEBUG);
    return root;
}

vector<string> getMissingDigits(const vector<string>& vec) {
    Logger::log("Calculating missing digits", "getMissingDigits", LogLevel::DEBUG);
    vector<string> result;
    bool digitsPresent[10] = {false}; // Array to track presence of digits 0-9

    // Mark digits that are present
    for (const string& digitStr : vec) {
        int digit = stoi(digitStr); // Convert string to integer
        if (digit >= 0 && digit <= 9) {
            digitsPresent[digit] = true;
        }
    }

    for (int i = 0; i < 10; ++i) {
        if (!digitsPresent[i]) {
            result.push_back(to_string(i));
        }
    }

    return result;
}


bool areElementsContinuous(const vector<string>& vec) {
    Logger::log("Checking if elements are continuous", "areElementsContinuous", LogLevel::DEBUG);
    if (vec.size() < 2) return true; // A single element or empty vector is trivially continuous

    try {
        int previous = stoi(vec[0]); // Convert the first element to an integer

        for (size_t i = 1; i < vec.size(); ++i) {
            int current = stoi(vec[i]); // Convert current element to integer
            if (current != previous + 1) {
                return false; // If current is not the previous + 1, they are not continuous
            }
            previous = current; // Update previous to current
        }
    } catch (const invalid_argument& e) {
        cerr << "Error: Non-numeric string encountered." << endl;
        return false;
    } catch (const out_of_range& e) {
        cerr << "Error: String conversion led to out of range error." << endl;
        return false;
    }

    return true;
}

string truncateRegexForLeafNodes(const vector<string>& leafRegexes){
    Logger::log("Truncating regex for leaf nodes", "truncateRegexForLeafNodes", LogLevel::DEBUG);
    const size_t leafRegexesSize = leafRegexes.size();
    if(leafRegexesSize == 10){
        return ANY_DIGIT_REGEX;
    }
    else if(leafRegexesSize > 3 && areElementsContinuous(leafRegexes)){
        return "[" + leafRegexes[0] + "-" + leafRegexes[leafRegexesSize - 1] + "]"; 
    }
    vector<string> missingDigits = getMissingDigits(leafRegexes);
    const size_t missingDigitsSize = missingDigits.size();
    if(missingDigitsSize < leafRegexesSize){
        if(missingDigitsSize > 3 && areElementsContinuous(missingDigits)){
            return "[^" + missingDigits[0] + "-" + missingDigits[missingDigitsSize-1] +"]";
        }
        else{
            return "[^" + accumulate(missingDigits.begin(),missingDigits.end(),string()) + "]";
        }
    }
    else{
        return "[" + accumulate(leafRegexes.begin(),leafRegexes.end(),string()) + "]";
    }
}


string truncateRegex(vector<string>& regexes,int height){
    // filter out empty strings
    string result = "";
    regexes.erase(remove_if(regexes.begin(), regexes.end(),[](const string& s) { return s.empty(); }),regexes.end());
    size_t regexesSize = regexes.size();
    if(regexesSize == 0){
        return "";
    }
    else if(regexesSize == 1){
        return regexes[0];
    }
    else if(height == 5){
        return truncateRegexForLeafNodes(regexes);
    }
    else{
        string result = accumulate(regexes.begin(),regexes.end(),string(),[](const string& a, const string& b){return a.empty() ? b : a + "|" + b;});
        return "(" + result + ")";
    }
}

vector<string> mergeSortedVectors(const vector<string>& vec1, const vector<string>& vec2){
    vector<string> result;
    size_t vec1Size = vec1.size();
    size_t vec2Size = vec2.size();
    size_t i = 0, j = 0;
    while(i < vec1Size && j < vec2Size){
        if(vec1[i].size() < vec2[j].size()){
            result.push_back(vec1[i++]);
        }
        else{
            result.push_back(vec2[j++]);
        }
    }
    while(i < vec1Size){
        result.push_back(vec1[i++]);
    }

    while(j < vec2Size){
        result.push_back(vec2[j++]);
    }
    return result;
}

vector<string> truncateAndSort(vector<vector<string>>& groups,int height){
    vector<string> result;
    for(auto& group : groups){
        result.push_back(truncateRegex(group,height));
    }
    sort(result.begin(),result.end(),[](const string& a, const string& b){return a.size() < b.size();});
    return result;
}

vector<string> groupAndTruncateRegexes(vector<string>& regexes,int height,int limit){
    vector<vector<string>> groups;
    regexes.erase(remove_if(regexes.begin(), regexes.end(),[](const string& s) { return s.empty(); }),regexes.end());
    if(regexes.size() == 0 || regexes.size() == 1){
        return regexes;
    }
    // group the regexes by the regex lengths
    int size = 0;
    vector<string> group;
    for(int i = 0;i < regexes.size();i++){
        size += regexes[i].size();
        int partitions = group.size();
        if(height + 3 + size + partitions < limit){
            group.push_back(regexes[i]);
        }
        else{
            // need to break the group and start a new one
            groups.push_back(group);
            group.clear();
            size = regexes[i].size();
            group.push_back(regexes[i]);
        }
    }
    if(group.size() > 0){
        groups.push_back(group);
        group.clear();
    }

    // truncate the group of regexes and sort them by the regex lengths
    return truncateAndSort(groups,height);
}

// parse the tree to build a regex
// should return an array of regexes
vector<string> buildRegexFromTree(Node* root,int limit){
    Logger::log("Building regex patterns from tree with limit: " + to_string(limit), "buildRegexFromTree", LogLevel::INFO);
    vector<string> result;
    if(root->childCount == 0){
        return {};
    }
    for(int i = 0;i < 10;i++){
        if(root->children[i]){ // check if unique_ptr is not null
            vector<string> aux;
            Node* childNode = root->children[i].get();
            vector<string> subRegexes = buildRegexFromTree(childNode,limit);
            // before prefixing the regexes with the digit, we need to group
            if(subRegexes.size() == 0){
                // leaf nodes handling
                aux.push_back(to_string(i));
            }
            else{
                for(const auto& subRegex : subRegexes){
                    string auxRegex = to_string(i) + subRegex;
                    if(root->height == 0){
                        auxRegex = "^" + auxRegex;
                    }
                    aux.push_back(auxRegex);
                }
            }

            // merge the aux array with the result array in a sorted manner
            result = mergeSortedVectors(result,aux);
        }
    }
    // need to trucate the regex
    vector<string> groupedRegexes = groupAndTruncateRegexes(result,root->height,limit);
    Logger::log("Completed building regex patterns", "buildRegexFromTree", LogLevel::INFO);
    return groupedRegexes;
}


// check if the file is a json file by checking the extension
bool isJsonFile(const string& filePath){
    Logger::log("Checking if file is JSON: " + filePath, "isJsonFile", LogLevel::DEBUG);
    const size_t filePathLength = filePath.length();
    if(filePathLength >= JSON_EXTENSION_LENGTH){
        return (0 == filePath.compare(filePathLength - JSON_EXTENSION_LENGTH, JSON_EXTENSION_LENGTH, JSON_EXTENSION));
    }
    else{
        return false;
    }
}

json createJSONRegex(const vector<string>& regexes){
    json result;
    result["regexes"] = regexes;
    return result;
}

bool validateRegexMatches(const vector<string>& regexes, const vector<int>& postalCodes) {
    LOG("Starting regex validation", LogLevel::INFO);
    try {
        set<int> validPostalCodes(postalCodes.begin(), postalCodes.end());
        LOG("Total valid postal codes: " + to_string(validPostalCodes.size()), LogLevel::DEBUG);

        vector<regex> regexObjects;
        for (const auto& pattern : regexes) {
            try {
                regexObjects.push_back(regex(pattern));
            } catch (const regex_error& e) {
                LOG("Invalid regex pattern: " + pattern + " Error: " + e.what(), LogLevel::ERROR);
                return false;
            }
        }

        set<int> invalidMatches;
        size_t totalMatches = 0;
        
        LOG("Starting validation of numbers from 100000 to 999999", LogLevel::INFO);
        ProgressBar progress(899999);
        
        for (int num = 100000; num <= 999999; num++) {
            const string numStr = to_string(num);
            bool matchesRegex = false;

            for (const auto& regex : regexObjects) {
                try {
                    if (regex_match(numStr, regex)) {
                        matchesRegex = true;
                        break;
                    }
                } catch (const regex_error& e) {
                    continue;
                }
            }

            if (matchesRegex) {
                totalMatches++;
                if (validPostalCodes.find(num) == validPostalCodes.end()) {
                    invalidMatches.insert(num);
                }
            }
            progress.update(num - 100000);
        }
        progress.finish();

        LOG("Validation complete. Total matches: " + to_string(totalMatches), LogLevel::INFO);
        LOG("Invalid matches: " + to_string(invalidMatches.size()), LogLevel::INFO);

        if (!invalidMatches.empty()) {
            string invalidSamples;
            int count = 0;
            for (int num : invalidMatches) {
                if (count++ < 10) {
                    invalidSamples += to_string(num) + " ";
                }
            }
            LOG("Sample invalid matches: " + invalidSamples, LogLevel::WARNING);
            return false;
        }

        return totalMatches == validPostalCodes.size();
    } catch (const exception& e) {
        LOG("Validation error: " + string(e.what()), LogLevel::ERROR);
        return false;
    }
}

/*
argc -> argument count
argv -> argument vector
*/
int main(int argc, char* argv[]) {
    string inputFilePath, outputFilePath;
    int regexLengthLimit = 1000;
    bool verifyMode = false;
    bool verboseMode = false;

    // Initialize logger
    Logger::init("pinrex.log");

    // Argument parsing
    try {
        LOG("Parsing command line arguments", LogLevel::DEBUG);
        for (int i = 1; i < argc; ++i) {
            string arg = argv[i];
            if (arg == "--version") {
                LOG("Version request received", LogLevel::DEBUG);
                cout << "PinRex Version " << APP_VERSION << endl;
                return 0;
            } else if (arg == "--help" || arg == "-h") {
                LOG("Help request received", LogLevel::DEBUG);
                cout << "Usage: " << argv[0] << " -i <input-file-path> -o <output-file-path> -l <regex-limit>" << "\n";
                cout << "Options:" << "\n";
                cout << "  -i, --input <input-file-path>    Path to the input JSON file containing postal codes" << "\n";
                cout << "  -o, --output <output-file-path>  Path to the output JSON file for generated regex patterns" << "\n";
                cout << "  -l, --limit <regex-limit>        Maximum length of generated regex patterns (default: 1000)" << "\n";
                cout << "  --verify                        Verify generated regex patterns against input postal codes" << "\n";
                cout << "  --verbose                       Enable verbose output" << "\n";
                cout << "  --version                       Display the version of PinRex" << "\n";
                cout << "  --help                          Display this help message" << "\n";
                return 0;
            } else if (arg == "-i" && i + 1 < argc) {
                inputFilePath = argv[++i];
                LOG("Input file set to: " + inputFilePath, LogLevel::DEBUG);
            } else if (arg == "-o" && i + 1 < argc) {
                outputFilePath = argv[++i];
                LOG("Output file set to: " + outputFilePath, LogLevel::DEBUG);
            } else if (arg == "-l" && i + 1 < argc) {
                regexLengthLimit = stoi(argv[++i]);
                LOG("Regex length limit set to: " + to_string(regexLengthLimit), LogLevel::DEBUG);
            } else if (arg == "--verify") {
                verifyMode = true;
                LOG("Verify mode enabled", LogLevel::DEBUG);
            } else if (arg == "--verbose") {
                verboseMode = true;
                LOG("Verbose mode enabled", LogLevel::DEBUG);
            } else {
                LOG("Invalid argument: " + arg, LogLevel::ERROR);
                cerr << "Usage: " << argv[0] << " -i <input-file-path> -o <output-file-path> -l <regex-limit>" << "\n";
                return 1;
            }
        }

        // Set verbose mode after parsing arguments
        Logger::setVerbose(verboseMode);
        
        if (verboseMode) {
            LOG("Starting PinRex v" + APP_VERSION, LogLevel::INFO);
        }

        // Input file validation and reading
        LOG("Validating input file: " + inputFilePath, LogLevel::INFO);
        if(!isJsonFile(inputFilePath)) {
            LOG("Invalid input file format", LogLevel::ERROR);
            return 1;
        }

        ifstream inputFile(inputFilePath);
        if(!inputFile.is_open()) {
            LOG("Failed to open input file: " + inputFilePath, LogLevel::ERROR);
            return 1;
        }

        // Parse JSON
        try {
            LOG("Parsing input JSON file", LogLevel::INFO);
            json postalCodesJson = json::parse(inputFile);
            
            if(!postalCodesJson.is_object() || !postalCodesJson.contains("postalCodes") || 
               !postalCodesJson["postalCodes"].is_array()) {
                LOG("Invalid JSON structure", LogLevel::ERROR);
                return 1;
            }

            vector<string> postalCodes;
            for (const auto& code : postalCodesJson["postalCodes"]) {
                postalCodes.push_back(to_string(code.get<int>()));
            }
            LOG("Successfully parsed " + to_string(postalCodes.size()) + " postal codes", LogLevel::INFO);

            if (verifyMode) {
                LOG("Starting verification mode", LogLevel::INFO);
                try {
                    ifstream outputFile(outputFilePath);
                    if (!outputFile.is_open()) {
                        LOG("Failed to open output file for verification: " + outputFilePath, LogLevel::ERROR);
                        return 1;
                    }
                    json outputJson = json::parse(outputFile);
                    vector<string> regexes = outputJson["regexes"];
                    bool isValid = validateRegexMatches(regexes, postalCodesJson["postalCodes"]);
                    LOG("Verification completed. Result: " + string(isValid ? "valid" : "invalid"), LogLevel::INFO);
                    return isValid ? 0 : 1;
                } catch (const exception& e) {
                    LOG("Verification error: " + string(e.what()), LogLevel::ERROR);
                    return 1;
                }
            }

            // Build regex tree and patterns
            LOG("Building regex tree", LogLevel::INFO);
            Node* root = buildTreeFromPostalCodes(postalCodes);
            vector<string> regexes = buildRegexFromTree(root, regexLengthLimit);
            
            // Write output
            LOG("Writing regex patterns to output file", LogLevel::INFO);
            ofstream outputFile(outputFilePath);
            if(outputFile.is_open()) {
                json result = createJSONRegex(regexes);
                outputFile << result.dump(4);
                outputFile.close();
                LOG("Successfully wrote regex patterns to: " + outputFilePath, LogLevel::INFO);
            } else {
                LOG("Failed to open output file for writing: " + outputFilePath, LogLevel::ERROR);
                return 1;
            }

        } catch (const exception& e) {
            LOG("Fatal error: " + string(e.what()), LogLevel::ERROR);
            return 1;
        }

    } catch(const exception& e) {
        LOG("Error parsing arguments: " + string(e.what()), LogLevel::ERROR);
        return 1;
    }

    LOG("PinRex completed successfully", LogLevel::INFO);
    return 0;
}