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

using namespace std;
using json = nlohmann::json;

const string APP_VERSION = "1.1.1";
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
    Node* root = new Node(0);
    for(const auto& postalCode : postalCodes){
        Node* currentNode = root;
        for(const char& digit : postalCode){
            int index = digit - '0';
            if(index >= 0 && index < 10){
                if(!currentNode->children[index]){
                    Node* child = new Node(currentNode->height + 1);
                    currentNode->addChild(index, unique_ptr<Node>(child));
                    currentNode = child;
                }
                else{
                    currentNode = currentNode->children[index].get();
                }
            }
            else{
                throw runtime_error("Invalid postal code digit: " + string(1,digit));
            }
        }
        currentNode = nullptr;
        delete currentNode;
    }
    return root;
}

vector<string> getMissingDigits(const vector<string>& vec) {
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

// parse the tree to build a regex
string buildRegexFromTree(Node* root){
    vector<string> subRegexes;
    if(root->childCount == 0){
        return "";
    }
    for(int i = 0;i < 10;i++){
        if(root->children[i]){ // check if unique_ptr is not null
            Node* childNode = root->children[i].get();
            string subRegex = to_string(i) + buildRegexFromTree(childNode);
            if(root->height == 0){
                subRegex = "^" + subRegex;
            }
            subRegexes.push_back(subRegex);
        }
    }
    // truncating the regexes 
    return truncateRegex(subRegexes,root->height);
}

bool isJsonFile(const string& filePath){
    const size_t filePathLength = filePath.length();
    if(filePathLength >= JSON_EXTENSION_LENGTH){
        return (0 == filePath.compare(filePathLength - JSON_EXTENSION_LENGTH, JSON_EXTENSION_LENGTH, JSON_EXTENSION));
    }
    else{
        return false;
    }
}

json createJSONRegex(const string& regex){
    json result;
    result["regex"] = regex;
    return result;
}

int main(int argc, char* argv[]) {
    string inputFilePath, outputFilePath;
    int regexLengthLimit = 0;

    // Argument parsing
    try{
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "--version") {
            cout << "PinRex Version " << APP_VERSION << endl;
            return 0;
        } else if (arg == "-i" && i + 1 < argc) {
            inputFilePath = argv[++i];
        } else if (arg == "-o" && i + 1 < argc) {
            outputFilePath = argv[++i];
        } else if (arg == "-l" && i + 1 < argc) {
            regexLengthLimit = stoi(argv[++i]);
        } else {
            cerr << "Usage: " << argv[0] << " -i <input-file-path> -o <output-file-path> -l <regex-limit>" << "\n";
            return 1;
        }
    }
    }
    catch(const exception& e){
        cerr << "Error occurred :.\n" << e.what() << "\n";
        return 1;
    }

    // read the input json file 
    ifstream inputFile(inputFilePath);

    if(!isJsonFile(inputFilePath)){
        cerr << "Error: Invalid input file format. It must be a JSON file." << "\n";
        return 1;
    }

    if(!inputFile.is_open()){
        cerr << "Error: Failed to open input file." << "\n";
        return 1;
    }

    json postalCodesJson = json::parse(inputFile);
    // validate the JSON structure
    if(!postalCodesJson.is_object() || !postalCodesJson.contains("postalCodes") || !postalCodesJson["postalCodes"].is_array()){
        cerr << "Error: Invalid JSON structure. It must be an object with a postalCodes array." << "\n";
        return 1;
    }

    // Extract postalCodes as an array of strings
    vector<string> postalCodes;
    for (const auto& code : postalCodesJson["postalCodes"]) {
        postalCodes.push_back(to_string(code.get<int>()));
    }

    Node* root = buildTreeFromPostalCodes(postalCodes);
    string regex = buildRegexFromTree(root);

    // write the regex to the output file
    ofstream outputFile(outputFilePath);
    json result = createJSONRegex(regex);
    if(outputFile.is_open()){
        outputFile << result.dump(4);
        outputFile.close();
        cout << "Regex saved to " << outputFilePath << endl;
    }
    else{
        cerr << "Error: Unable to open output file for writing." << endl;
        return 1;
    }
    return 0;
}