# PinRex - Pincode Regex Generator

This command line tool takes in a list of pincodes and generates a regular expression (regex) based on the provided list. The generated regex can be used to match and validate pincodes in various applications.

## Installation

1. Clone the repository:
    
        git clone https://github.com/sishir2001/PinRex

2. Navigate to the project directory:

        cd PinRex

3. Create and activate a virtual environment (optional but recommended):

        python -m venv PinRex
        source PinRex/bin/activate

4. Install the dependencies:

        pip install -r requirements.txt

## Usage

To generate a regex based on a list of pincodes, run the following command:

    python ./src/main.py -i <input_file_path>.csv -o <output_file_path>.csv

- `-i` or `--input` options for specifying the path of input csv file
- `-o` or `--output` options for specifying the path of output csv file

## Input File Format

The input file should be a CSV file with a single column containing the list of pincodes. Each pincode should be on a separate line.

Example input file:

    Pin,State
    123456,Telangana
    123478,Telangana
    345678,Karnataka
    349823,Karnataka


## Output

The generated regex will match the provided pincodes. You can use this regex in your applications to validate and match pincodes.

Example output:

    State,Regex
    Telangana,"^1234(56|78)"
    Karnataka,"^34(5678|9823)"


# Contributing

Contributions are welcome! If you find any issues or have suggestions for improvements, please open an issue or submit a pull request.

# License

This project is licensed under the [MIT License](LICENSE).


