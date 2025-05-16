# Q1
# It is poisson binomial distribution. Each trial has a probability and is independent to other trials.
# We can calculate by dynamic programming.
def prob_rain_more_than_n(p: list[float], n: int) -> float:
    total_days = len(p)
    dp = [0] * (total_days + 1)  # dp[k] will hold the probability of exactly k rainy days
    dp[0] = 1  # Probability of having 0 rainy days initially is 1

    for prob in p:
        # We need to update the dp array in reverse order to avoid overwriting the current day's results
        for k in range(total_days, 0, -1):
            dp[k] = dp[k] * (1 - prob) + dp[k - 1] * prob
        dp[0] *= (1 - prob)  # The probability of not raining at all is the product of (1 - prob).
    
    # Calculate the probability of having at least n rainy days
    return sum(dp[k] for k in range(n, total_days + 1))


# Q2
# dictionary
word_dictionary = {"ABACUS": ["AE", "B", "AH", "K", "AH", "S"],
    "BOOK": ["B", "UH", "K"],
    "THEIR": ["DH", "EH", "R"],
    "THERE": ["DH", "EH", "R"],
    "TOMATO": ["T", "AH", "M", "AA", "T", "OW"],
    "TOMATO": ["T", "AH", "M", "EY", "T", "OW"]
}

# I used depth first search. 
def find_word_combos_with_pronunciation(phonemes: list[str]) -> list[list[str]]:
    # Base case: if no phonemes are left, return an empty list
    if not phonemes:
        return []

    possible_phonemes = []

    for key, values in word_dictionary.items():
        # Check if the current word's pronunciation matches the start of the phonemes
        if phonemes[:len(values)] == values:
            # Recursively find combinations for the remaining phonemes
            if len(phonemes[len(values):]) != 0:
                # first word + list[rest of possible words]
                next_all_possible_phonemes = find_word_combos_with_pronunciation(phonemes[len(values):])
                for j in next_all_possible_phonemes:
                    possible_phonemes.append([key] + j)
            else:
                possible_phonemes.append([key])

    return possible_phonemes

