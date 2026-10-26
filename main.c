#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#define MAX_INPUT 512
#define MAX_WORDS 8
#define MAX_LEN 64

typedef struct {
    char words[MAX_WORDS][MAX_LEN];
    char operations[MAX_WORDS - 2];  /* operators between left-hand words */
    int count;
    char letters[10];
    int letter_count;
    int digit[26];
    unsigned char leading[26];
} Puzzle;

int parse(Puzzle* p, const char* input) {
    memset(p, 0, sizeof(*p));
    for (int i = 0; i < 26; ++i) p->digit[i] = -1;

    const char* s = input;
    int seen_equal = 0;
    int need_word = 1;
    while (1) {
        while (*s == ' ' || *s == '\t' || *s == '\r') ++s;
        if (*s == '\n' || *s == '\0') break;

        if (need_word) {
            if (*s < 'A' || *s > 'Z' || p->count >= MAX_WORDS) return 0;
            int len = 0;
            while (*s >= 'A' && *s <= 'Z') {
                if (len >= MAX_LEN - 1) return 0;
                char c = *s++;
                p->words[p->count][len++] = c;
                int found = 0;
                for (int i = 0; i < p->letter_count; ++i)
                    if (p->letters[i] == c) found = 1;
                if (!found) {
                    if (p->letter_count == 10) return 0;
                    p->letters[p->letter_count++] = c;
                }
            }
            p->words[p->count][len] = '\0';
            if (len > 1) p->leading[p->words[p->count][0] - 'A'] = 1;
            ++p->count;
            need_word = 0;
        }
        else {
            char op = *s++;
            if (op == '=') {
                if (seen_equal || p->count < 2) return 0;
                seen_equal = 1;
            }
            else if (op == '+' || op == '-' || op == '*' || op == '/') {
                if (seen_equal || p->count >= MAX_WORDS - 1) return 0;
                p->operations[p->count - 1] = op;
            }
            else return 0;
            need_word = 1;
        }
    }
    return seen_equal && !need_word && p->count >= 3 && p->count <= MAX_WORDS;
}

/* Return 0 for values larger than signed 64-bit integer. */
int word_value(const Puzzle* p, const char* word, long long* out) {
    long long value = 0;
    for (int i = 0; word[i]; ++i) {
        int digit = p->digit[word[i] - 'A'];
        if (digit < 0 || value >(LLONG_MAX - digit) / 10) return 0;
        value = value * 10 + digit;
    }
    *out = value;
    return 1;
}

int safe_add(long long a, long long b, long long* out) {
    if ((b > 0 && a > LLONG_MAX - b) ||
        (b < 0 && a < LLONG_MIN - b)) return 0;
    *out = a + b;
    return 1;
}

/* Operators * and / apply before + and -, as in normal arithmetic.
   Division by zero and non-integer division are rejected. */
int calculate_left(const Puzzle* p, long long* out) {
    long long sum = 0;
    long long term;
    if (!word_value(p, p->words[0], &term)) return 0;
    int sign = 1;
    for (int i = 1; i < p->count - 1; ++i) {
        long long value;
        if (!word_value(p, p->words[i], &value)) return 0;
        char op = p->operations[i - 1];
        if (op == '*') {
            if (value != 0 && term > LLONG_MAX / value) return 0;
            term *= value;
        }
        else if (op == '/') {
            if (value == 0 || term % value != 0) return 0;
            term /= value;
        }
        else {
            if (!safe_add(sum, sign == 1 ? term : -term, &sum)) return 0;
            term = value;
            sign = (op == '-') ? -1 : 1;
        }
    }
    return safe_add(sum, sign == 1 ? term : -term, out);
}

int is_solution(const Puzzle* p) {
    long long left, right;
    if (!calculate_left(p, &left)) return 0;
    if (!word_value(p, p->words[p->count - 1], &right)) return 0;
    return left == right;
}

void print_solution(const Puzzle* p) {
    for (int i = 0; i < p->count; ++i) {
        long long value = 0;
        word_value(p, p->words[i], &value);
        if (i > 0) {
            if (i == p->count - 1) printf(" = ");
            else printf(" %c ", p->operations[i - 1]);
        }
        printf("%lld", value);
    }
    putchar('\n');
}

int backtracking(Puzzle* p, int index, int used[10]) {
    if (index == p->letter_count) return is_solution(p);
    int ch = p->letters[index] - 'A';
    for (int d = 0; d <= 9; ++d) {
        if (used[d]) continue;
        if (d == 0 && p->leading[ch]) continue;
        p->digit[ch] = d;
        used[d] = 1;
        if (backtracking(p, index + 1, used)) return 1;
        used[d] = 0;
        p->digit[ch] = -1;
    }
    return 0;
}

int main(void) {
    char input[MAX_INPUT];
    Puzzle puzzle;
    printf("Enter puzzle: ");
    if (!fgets(input, sizeof(input), stdin)) return 1;
    if (!parse(&puzzle, input)) {
        puts("Invalid puzzle (use 2 to 7 operands, uppercase letters, + - * / and =).");
        return 1;
    }
    int used[10] = { 0 };
    clock_t start = clock();
    int found = backtracking(&puzzle, 0, used);
    clock_t end = clock();
    if (found) print_solution(&puzzle);
    else puts("No solution found.");
    printf("Execution time: %.6f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    return 0;
}
