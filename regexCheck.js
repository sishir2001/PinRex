function validateRegexMatches() {
  // Read the regex patterns and postal codes
  try {
    const regexPatterns = require('./static/output.json').regexes;
    const validPostalCodes = new Set(require('./static/input.json').postalCodes);
    const regexes = regexPatterns.map((pattern) => new RegExp(pattern));
    // Track matches that aren't in the original postal codes
    const invalidMatches = new Set();

    // Test each number from 100000 to 999999
    for (let num = 100000; num <= 999999; num++) {
      const numStr = num.toString();

      // Check if the number matches any regex
      const matchesRegex = regexes.some((regex) => regex.test(numStr));

      if (matchesRegex && !validPostalCodes.has(num)) {
        invalidMatches.add(num);
      }
    }

    if (invalidMatches.size > 0) {
      console.log("Found numbers that match regex but aren't in postal codes:");
      console.log([...invalidMatches].slice(0, 10)); // Show first 10 invalid matches
      console.log(`Total invalid matches: ${invalidMatches.size}`);
    } else {
      console.log("All regex matches are valid postal codes!");
    }
  } catch (e) {
    console.error(e.message);
  }

  // Convert regex strings to RegExp objects
}

validateRegexMatches();
