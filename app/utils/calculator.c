#include "include/calculator.h"
#include "include/memcore.h"
#include "include/colors.h"

// Function to determine the precedence of an operator
int precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0;
}

// Function to apply an operator to two operands
int apply_op(int a, int b, char op) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return a / b;
    }
    return 0;
}

// Function to evaluate a postfix expression
int evaluate_postfix(char* postfix) {
    int stack[32];
    int top = -1;
    for (int i = 0; postfix[i]; i++) {
        if (postfix[i] == ' ') {
            continue;
        }

        if (postfix[i] >= '0' && postfix[i] <= '9') {
            int num = 0;
            while (postfix[i] >= '0' && postfix[i] <= '9') {
                num = num * 10 + (postfix[i] - '0');
                i++;
            }
            i--; // To offset the for-loop's increment
            stack[++top] = num;
        } else {
            if (top < 1) {
                // Error: not enough operands for operator
                return 0; // Or some error code
            }
            int val2 = stack[top--];
            int val1 = stack[top--];
            stack[++top] = apply_op(val1, val2, postfix[i]);
        }
    }
    if (top != 0) {
        // Error: malformed expression
        return 0;
    }
    return stack[top];
}

// Shunting-yard algorithm to convert infix to postfix
void infix_to_postfix(const char* infix, char* postfix) {
    char stack[32];
    int top = -1;
    int j = 0;
    for (int i = 0; infix[i]; i++) {
        if (infix[i] == ' ') continue;

        if (infix[i] >= '0' && infix[i] <= '9') {
            while (infix[i] >= '0' && infix[i] <= '9') {
                postfix[j++] = infix[i++];
            }
            postfix[j++] = ' ';
            i--;
        } else if (infix[i] == '(') {
            stack[++top] = infix[i];
        } else if (infix[i] == ')') {
            while (top > -1 && stack[top] != '(') {
                postfix[j++] = stack[top--];
                postfix[j++] = ' ';
            }
            if (top > -1) top--; // Pop '('
        } else { // Operator
            while (top > -1 && stack[top] != '(' && precedence(stack[top]) >= precedence(infix[i])) {
                postfix[j++] = stack[top--];
                postfix[j++] = ' ';
            }
            stack[++top] = infix[i];
        }
    }

    while (top > -1) {
        if (stack[top] == '(') {
            top--;
            continue;
        }
        postfix[j++] = stack[top--];
        postfix[j++] = ' ';
    }
    if (j > 0) {
        postfix[j-1] = '\0';
    } else {
        postfix[j] = '\0';
    }
}

void calculator_main(const char* expression) {
    char postfix[256];
    infix_to_postfix(expression, postfix);
    int result = evaluate_postfix(postfix);
    kprintf("Result: %d\n", result);
}