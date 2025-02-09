# PinRex - Pincode Regex Generator

This command line tool takes in a list of postal codes in JSON format and generates optimized regular expressions that match the provided codes. The generated regex patterns can be used to validate postal codes in various applications.

## Features

- Generates optimized regex patterns for postal code validation
- Handles large sets of postal codes efficiently
- Supports verification of generated patterns
- Configurable regex length limits
- JSON input/output format

## Prerequisites

- C++ compiler with C++11 support
- Make build system
- nlohmann/json library for C++

## Installation

### Using Homebrew (macOS and Linux)

You can install PinRex using Homebrew:

        brew tap sishir2001/pinrex
        brew install pinrex

### Building from Source

If you prefer to build from source:

1.  Clone the repository:

        git clone https://github.com/sishir2001/PinRex

2.  Navigate to the project directory:

        cd PinRex

3.  Build the project:

        mkdir build
        cd build
        cmake --build .

## Usage

To generate regex patterns based on a list of postal codes, run the following command:

    ./pinrex -i <input_file_path>.json -o <output_file_path>.json [-l <regex_length_limit>] [--verify]

Options:

- `-i`: Input JSON file path containing postal codes
- `-o`: Output JSON file path for generated regex patterns
- `-l`: Optional regex length limit (default: 1000)
- `--verify`: Optional flag to verify generated regex patterns
- `--version`: Display version information
- `--help`: Display help message

## Input File Format

The input file should be a JSON file containing an array of postal codes under the "postalCodes" key.

Example input file (input.json):

    {
        "postalCodes": [
            110001,
            110002,
            110003,
            110004,
            110005
        ]
    }

## Output Format

The output will be a JSON file containing the generated regex patterns that match the input postal codes.

Example output (output.json):

    {
        "regexes": [
            "^11000[1-5]$"
        ]
    }

The generated regex patterns:

- Are anchored with `^` and `$` to ensure exact matches
- Use character classes `[]` for ranges of digits
- Use grouping `()` to capture common prefixes
- Use alternation `|` to match different possibilities

## Verification

You can verify the generated regex patterns using the `--verify` flag:

    ./pinrex -i input.json -o output.json --verify

This will check if:

1. The regex patterns only match valid postal codes
2. All valid postal codes are matched by the patterns
3. No invalid postal codes are matched

## Error Handling

The program includes error checking for:

- Invalid input file format
- Missing input/output files
- Invalid JSON structure
- Invalid command-line arguments
- Regex generation failures

## Performance

The tool is optimized to:

- Handle large sets of postal codes
- Generate compact regex patterns
- Minimize regex backtracking
- Maintain reasonable memory usage

## Contributing

Contributions are welcome! If you find any issues or have suggestions for improvements, please:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

Please ensure your code follows the existing style and includes appropriate tests.

## License

This project is licensed under the [MIT License](LICENSE).
