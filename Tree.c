/*
 * 학번: 5906914
 * 이름: 문정욱
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_LENGTH 2000
#define MAX_CHILDREN 100

char treeString[MAX_LENGTH];
int treeLength;

int totalNodeCount = 0;
int leafNodeCount = 0;
int nonLeafNodeCount = 0;
int treeHeight = 0;
int treeDegree = 0;

int isCFound = 0;        // 트리 내에 노드 'C'가 존재하는가?
char cParentNode = 0;    // 'C'의 부모 노드 문자 (0이면 부모 없음/루트)

char childrenOfCStack[MAX_CHILDREN];
int childrenOfCTop = -1;

char parentStack[MAX_LENGTH];
int childCountStack[MAX_LENGTH];
int stackTop = -1;

// ---- 부모 스택 push/pop ----
void pushParent(char parentChar) {
    stackTop++;
    parentStack[stackTop] = parentChar;
    childCountStack[stackTop] = 0;
}

void popParent() {
    stackTop--;
}

// ---- C의 자식을 저장하는 스택 push ----
void pushChildOfC(char childChar) {
    if (childrenOfCTop < MAX_CHILDREN - 1) {
        childrenOfCTop++;
        childrenOfCStack[childrenOfCTop] = childChar;
    }
}

int checkTreeAndComputeStats() {
    int index = 0;
    int currentLevel = 0;
    int expectingNode = 1;   // 1: 노드가 와야 함, 0: ',' 또는 ')' 가 와야 함

    while (index < treeLength) {
        char currentChar = treeString[index];

        if (expectingNode) {
            if (currentChar < 'A' || currentChar > 'Z') {
                printf("오류: 잘못된 입력입니다\n");
                return 0;
            }

            totalNodeCount++;
            if (totalNodeCount == 1 && currentChar != 'A') {
                printf("오류: 루트 노드는 A여야 합니다\n");
                return 0;
            }

            int currentDepth = currentLevel + 1;
            if (currentDepth > treeHeight) {
                treeHeight = currentDepth;
            }

            char parentChar = (stackTop >= 0) ? parentStack[stackTop] : 0;
            if (stackTop >= 0) {
                childCountStack[stackTop]++;
            }

            if (currentChar == 'C') {
                isCFound = 1;
                cParentNode = parentChar;
            }
            if (parentChar == 'C') {
                pushChildOfC(currentChar);
            }

            index++;
            if (index < treeLength && treeString[index] == '(') {
                nonLeafNodeCount++;
                pushParent(currentChar);
                currentLevel++;
                index++;
                expectingNode = 1;
            } else {
                leafNodeCount++;
                expectingNode = 0;
            }
        } else {
            if (index >= treeLength) break;
            currentChar = treeString[index];

            if (currentChar == ',') {
                if (stackTop < 0) {
                    printf("오류: 구조 오류입니다\n");
                    return 0;
                }
                index++;
                expectingNode = 1;
            } else if (currentChar == ')') {
                if (stackTop < 0) {
                    printf("오류: 괄호가 맞지 않습니다\n");
                    return 0;
                }
                if (childCountStack[stackTop] > treeDegree) {
                    treeDegree = childCountStack[stackTop];
                }
                popParent();
                currentLevel--;
                index++;
                expectingNode = 0;
            } else {
                printf("오류: 잘못된 입력입니다\n");
                return 0;
            }
        }
    }

    if (stackTop != -1) {
        printf("오류: 괄호가 맞지 않습니다\n");
        return 0;
    }
    if (totalNodeCount == 0) {
        printf("오류: 트리가 비어 있습니다\n");
        return 0;
    }
    return 1;
}

int isLastSibling(int position) {
    position++;
    if (treeString[position] == '(') {
        int depth = 1;
        position++;
        while (depth > 0) {
            if (treeString[position] == '(') depth++;
            else if (treeString[position] == ')') depth--;
            position++;
        }
    }
    return treeString[position] != ',';
}

void printTree(int *position, char *prefix, int isLast, int isRoot) {
    char nodeChar = treeString[*position];
    (*position)++;

    if (isRoot) {
        printf("%c\n", nodeChar);
    } else {
        printf("%s+---%c\n", prefix, nodeChar);
    }

    if (treeString[*position] == '(') {
        (*position)++;
        char newPrefix[MAX_LENGTH];
        snprintf(newPrefix, MAX_LENGTH, "%s%s",
                 prefix, isRoot ? "" : (isLast ? "    " : "|   "));

        while (1) {
            int childIsLast = isLastSibling(*position);
            printTree(position, newPrefix, childIsLast, 0);
            if (treeString[*position] == ',') {
                (*position)++;
            } else {
                (*position)++;
                break;
            }
        }
    }
}

int main(void) {
    char inputBuffer[MAX_LENGTH];
    printf("괄호형 트리를 입력하세요: ");
    if (fgets(inputBuffer, MAX_LENGTH, stdin) == NULL) {
        printf("오류: 입력이 없습니다\n");
        return 1;
    }

    int writeIndex = 0;
    for (int readIndex = 0; inputBuffer[readIndex]; readIndex++) {
        if (!isspace((unsigned char)inputBuffer[readIndex])) {
            treeString[writeIndex++] = inputBuffer[readIndex];
        }
    }
    treeString[writeIndex] = '\0';
    treeLength = writeIndex;

    if (!checkTreeAndComputeStats()) {
        return 1;
    }

    printf("전체 노드 수: %d\n", totalNodeCount);
    printf("단말 노드 수: %d\n", leafNodeCount);
    printf("비단말 노드 수: %d\n", nonLeafNodeCount);
    printf("트리의 높이: %d\n", treeHeight);
    printf("트리의 차수: %d\n", treeDegree);

    printf("C의 부모: ");
    if (!isCFound) {
        printf("C 없음\n");
    } else if (cParentNode == 0) {
        printf("없음(루트)\n");
    } else {
        printf("%c\n", cParentNode);
    }

    printf("C의 자식: ");
    if (!isCFound) {
        printf("C 없음\n");
    } else if (childrenOfCTop == -1) {
        printf("없음(단말노드)\n");
    } else {
        for (int i = 0; i <= childrenOfCTop; i++) {
            printf("%c ", childrenOfCStack[i]);
        }
        printf("\n");
    }

    int startPosition = 0;
    printTree(&startPosition, "", 1, 1);

    return 0;
}