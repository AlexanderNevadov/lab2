#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_INPUT 512
#define MAX_WORDS 8
#define MAX_LEN 64

typedef struct {
    char words[MAX_WORDS][MAX_LEN];
    int count;
    char letters[10];
    int letter_count;
    int digit[26];
    unsigned char leading[26];
    char input[MAX_INPUT];
} Puzzle;

int parse(Puzzle *p, const char *input) {
    memset(p, 0, sizeof(*p));
    for (int i = 0; i < 26; i++) p->digit[i] = -1;
    strncpy(p->input, input, MAX_INPUT - 1);
    int side = 0, pos = 0, equals = 0;
    for (const char *s = input; ; s++) {
        char c = *s;
        if (c >= 'A' && c <= 'Z') {
            if (p->count >= MAX_WORDS || pos >= MAX_LEN - 1) return 0;
            p->words[p->count][pos++] = c;
            continue;
        }
        if (pos > 0) {
            p->words[p->count][pos] = '\0';
            if (pos > 1) p->leading[p->words[p->count][0] - 'A'] = 1;
            for (int j = 0; j < pos; j++) {
                char ch = p->words[p->count][j];
                int found = 0;
                for (int k = 0; k < p->letter_count; k++)
                    if (p->letters[k] == ch) found = 1;
                if (!found) {
                    if (p->letter_count == 10) return 0;
                    p->letters[p->letter_count++] = ch;
                }
            }
            p->count++;
            pos = 0;
        }
        if (c == '\0' || c == '\n') break;
        if (c == ' ' || c == '\t' || c == '\r') continue;
        if (c == '+') {
            if (side != 0 || p->count == 0) return 0;
        } else if (c == '=') {
            if (equals || p->count < 2) return 0;
            equals = 1;
            side = 1;
        } else return 0;
    }
    return equals && p->count >= 3 && p->count <= MAX_WORDS;
}

long long word_value(const Puzzle *p, const char *word) {
    long long value = 0;
    for (int i = 0; word[i]; i++) value = value * 10 + p->digit[word[i] - 'A'];
    return value;
}

int is_solution(const Puzzle *p) {
    int used[10] = {0};
    for (int i = 0; i < p->letter_count; i++) {
        int d = p->digit[p->letters[i] - 'A'];
        if (used[d]) return 0;
        used[d] = 1;
    }
    for (int i = 0; i < p->letter_count; i++) {
        int ch = p->letters[i] - 'A';
        if (p->leading[ch] && p->digit[ch] == 0) return 0;
    }
    long long sum = 0;
    for (int i = 0; i < p->count - 1; i++) sum += word_value(p, p->words[i]);
    return sum == word_value(p, p->words[p->count - 1]);
}

void print_solution(const Puzzle *p) {
    for (int i = 0; i < p->count; i++) {
        if (i) printf(i == p->count - 1 ? " = " : " + ");
        printf("%lld", word_value(p, p->words[i]));
    }
    putchar('\n');
}

int brute_force(Puzzle *p, int index) {
    if (index == p->letter_count) {
        if (is_solution(p)) {
            print_solution(p);
            return 1;
        }
        return 0;
    }
    int ch = p->letters[index] - 'A';
    for (int d = 0; d <= 9; d++) {
        p->digit[ch] = d;
        if (brute_force(p, index + 1)) return 1;
    }
    p->digit[ch] = -1;
    return 0;
}

int main(void) {
    char input[MAX_INPUT];
    Puzzle puzzle;
    printf("Enter puzzle: ");
    if (!fgets(input, sizeof(input), stdin)) return 1;
    if (!parse(&puzzle, input)) {
        puts("Invalid puzzle (use 2 to 7 addends, uppercase letters, + and =).");
        return 1;
    }
    if (!brute_force(&puzzle, 0)) puts("No solution found.");
    return 0;
}
