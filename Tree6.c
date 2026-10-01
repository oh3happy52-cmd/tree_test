/*과제 5번*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DATA_COUNT 100
#define SEARCH_COUNT 50
#define MAX_VALUE 1000

typedef struct Node {
    int value;
    struct Node *left, *right;
} Node;

int dataArray[DATA_COUNT];
int searchKeys[SEARCH_COUNT];

/* ---------------- 중복 없는 랜덤 정수 생성 ---------------- */
int isDuplicate(int *arr, int count, int value) {
    for (int i = 0; i < count; i++) {
        if (arr[i] == value) return 1;
    }
    return 0;
}

void generateUniqueRandoms(int *arr, int count) {
    int generated = 0;
    while (generated < count) {
        int value = rand() % (MAX_VALUE + 1);
        if (isDuplicate(arr, generated, value)) continue;
        arr[generated++] = value;
    }
}

/* ---------------- BST 탐색 (트리 구조 활용) ---------------- */
long bstBuildComparisons = 0;

Node* newNode(int value) {
    Node *n = (Node *)malloc(sizeof(Node));
    n->value = value;
    n->left = n->right = NULL;
    return n;
}

Node* bstInsert(Node *root, int value) {
    if (root == NULL) {
        return newNode(value);
    }

    Node *current = root;
    while (1) {
        bstBuildComparisons++;
        if (value < current->value) {
            if (current->left == NULL) {
                current->left = newNode(value);
                break;
            }
            current = current->left;
        } else {
            if (current->right == NULL) {
                current->right = newNode(value);
                break;
            }
            current = current->right;
        }
    }
    return root;
}

int bstSearch(Node *root, int key, long *comparisons) {
    Node *current = root;
    *comparisons = 0;
    while (current != NULL) {
        (*comparisons)++;
        if (key == current->value) return 1;
        else if (key < current->value) current = current->left;
        else current = current->right;
    }
    return 0;
}

void freeTree(Node *n) {
    if (n == NULL) return;
    freeTree(n->left);
    freeTree(n->right);
    free(n);
}

/* ---------------- 순차 탐색 (트리 구조 활용 안 함) ---------------- */
int sequentialSearch(int *arr, int count, int key, long *comparisons) {
    *comparisons = 0;
    for (int i = 0; i < count; i++) {
        (*comparisons)++;
        if (arr[i] == key) return 1;
    }
    return 0;
}

int main(void) {
    srand((unsigned int)time(NULL));

    /* 1. 100개의 서로 다른 정수 생성 및 배열 저장 */
    generateUniqueRandoms(dataArray, DATA_COUNT);

    printf("===== 생성된 100개의 정수 (배열, 발생 순서) =====\n");
    for (int i = 0; i < DATA_COUNT; i++) {
        printf("%d ", dataArray[i]);
        if ((i + 1) % 10 == 0) printf("\n");
    }
    printf("\n");

    /* 2. 동일한 100개를 BST에 삽입 (비교 횟수 누적) */
    Node *root = NULL;
    bstBuildComparisons = 0;
    for (int i = 0; i < DATA_COUNT; i++) {
        root = bstInsert(root, dataArray[i]);
    }

    printf("BST 생성 과정에서 발생한 총 비교 횟수: %ld\n\n", bstBuildComparisons);

    /* 3. 탐색 대상 50개 생성 */
    generateUniqueRandoms(searchKeys, SEARCH_COUNT);

    printf("===== 생성된 50개의 탐색 대상 =====\n");
    for (int i = 0; i < SEARCH_COUNT; i++) {
        printf("%d ", searchKeys[i]);
        if ((i + 1) % 10 == 0) printf("\n");
    }
    printf("\n");

    /* 4. 각 탐색 대상에 대해 순차 탐색 + BST 탐색 수행  */
    long totalSeqComparisons = 0;
    long totalBstComparisons = 0;

    printf("===== 개별 탐색 결과 (50건) =====\n\n");

    for (int i = 0; i < SEARCH_COUNT; i++) {
        int key = searchKeys[i];
        long seqCmp, bstCmp;

        int seqFound = sequentialSearch(dataArray, DATA_COUNT, key, &seqCmp);
        int bstFound = bstSearch(root, key, &bstCmp);

        totalSeqComparisons += seqCmp;
        totalBstComparisons += bstCmp;

        printf("Search Key : %d\n\n", key);
        printf("Sequential Search\n");
        printf("Result      : %s\n", seqFound ? "Found" : "Not Found");
        printf("Comparisons : %ld\n\n", seqCmp);
        printf("BST Search\n");
        printf("Result      : %s\n", bstFound ? "Found" : "Not Found");
        printf("Comparisons : %ld\n\n", bstCmp);
        printf("--------------------------------------------------\n\n");
    }

    /* 5. 통계 출력 */
    double seqAvg = (double)totalSeqComparisons / SEARCH_COUNT;
    double bstAvg = (double)totalBstComparisons / SEARCH_COUNT;

    printf("===== 통계 =====\n");
    printf("Number of searches: %d\n\n", SEARCH_COUNT);
    printf("Sequential Search\n");
    printf("  Total comparisons   : %ld\n", totalSeqComparisons);
    printf("  Average comparisons : %.2f\n\n", seqAvg);
    printf("BST Search\n");
    printf("  Total comparisons   : %ld\n", totalBstComparisons);
    printf("  Average comparisons : %.2f\n\n", bstAvg);

    printf("BST 생성 시 발생한 총 비교 횟수 : %ld\n", bstBuildComparisons);
    printf("BST 생성 비용 + 탐색 비용 합계   : %ld\n", bstBuildComparisons + totalBstComparisons);
    printf("순차 탐색만의 총 비용            : %ld\n", totalSeqComparisons);

    freeTree(root);
    return 0;
}