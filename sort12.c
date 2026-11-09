/*
 * 과제 12: 퀵 정렬의 피벗 선택에 따른 성능 비교
 *  - 피벗 선택 방법 3가지: First / Random / Median-of-Three
 *  - 분할(partition) 알고리즘은 세 방법 모두 동일
 *  - 데이터 값 사이의 비교 횟수만 센다 (인덱스 비교, 난수 생성, 이동은 제외)
 *  - [보너스] 재귀적 퀵 정렬 vs 스택을 이용한 반복적 퀵 정렬 실행시간 비교
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PIVOT_FIRST  0
#define PIVOT_RANDOM 1
#define PIVOT_MEDIAN 2

static long long cmp_count = 0;   /* 데이터 값 비교 횟수 */

/*
 * 0 ~ 약 10억 범위의 난수
 * Windows는 RAND_MAX가 32767이라 하위 15비트 두 개를 이어 붙여 사용
 * (& 0x7FFF로 15비트만 사용해야 Linux/Mac에서도 음수가 나오지 않음)
 */
static int big_rand(void)
{
    return (int)(((unsigned int)(rand() & 0x7FFF) << 15) | (unsigned int)(rand() & 0x7FFF));
}

static void swap(int *a, int *b)
{
    int t = *a;
    *a = *b;
    *b = t;
}

/* 데이터 값 비교: 호출될 때마다 비교 횟수 1 증가 */
static int LESS(int a, int b)
{
    cmp_count++;
    return a < b;
}

static int GREATER(int a, int b)
{
    cmp_count++;
    return a > b;
}

/* 첫 번째, 가운데, 마지막 원소 중 중앙값의 인덱스를 반환 (값 비교 2~3회, 모두 카운트) */
static int median_of_three(int list[], int left, int right)
{
    int mid = (left + right) / 2;

    if (LESS(list[left], list[mid])) {          /* left < mid */
        if (LESS(list[mid], list[right]))       /* left < mid < right */
            return mid;
        else if (LESS(list[left], list[right])) /* left < right <= mid */
            return right;
        else                                    /* right <= left < mid */
            return left;
    } else {                                    /* mid <= left */
        if (LESS(list[left], list[right]))      /* mid <= left < right */
            return left;
        else if (LESS(list[mid], list[right]))  /* mid < right <= left */
            return right;
        else                                    /* right <= mid <= left */
            return mid;
    }
}

/* 피벗 선택: 선택한 피벗을 구간의 첫 번째 칸(left)으로 옮겨 둔다 */
static void choose_pivot(int list[], int left, int right, int method)
{
    int p = left;

    if (method == PIVOT_RANDOM)
        p = left + big_rand() % (right - left + 1);
    else if (method == PIVOT_MEDIAN)
        p = median_of_three(list, left, right);

    if (p != left)
        swap(&list[left], &list[p]);
}

/*
 * 분할 (세 방법 공통)
 *  - 피벗은 list[left]에 있음
 *  - low는 왼쪽에서 오른쪽으로 가며 피벗보다 크거나 같은 값을 찾고
 *  - high는 오른쪽에서 왼쪽으로 가며 피벗보다 작거나 같은 값을 찾아 서로 교환
 *  - low와 high가 엇갈리면 피벗을 high 위치로 옮기고 그 위치를 반환
 */
static int partition(int list[], int left, int right)
{
    int pivot = list[left];
    int low = left;
    int high = right + 1;

    do {
        do {
            low++;
        } while (low <= right && LESS(list[low], pivot));

        do {
            high--;
        } while (high >= left && GREATER(list[high], pivot));

        if (low < high)
            swap(&list[low], &list[high]);
    } while (low < high);

    swap(&list[left], &list[high]);
    return high;
}

/* 재귀적 퀵 정렬 */
static void quick_sort_recursive(int list[], int left, int right, int method)
{
    if (left < right) {
        int q;
        choose_pivot(list, left, right, method);
        q = partition(list, left, right);
        quick_sort_recursive(list, left, q - 1, method);
        quick_sort_recursive(list, q + 1, right, method);
    }
}

/* 반복적 퀵 정렬: 정렬할 구간 (left, right)를 직접 만든 스택에 저장 */
static void quick_sort_iterative(int list[], int n, int method)
{
    int *stack;
    int top = -1;

    if (n < 2)
        return;

    stack = (int *)malloc(sizeof(int) * 2 * n);
    if (stack == NULL) {
        fprintf(stderr, "메모리 할당 실패\n");
        exit(1);
    }

    stack[++top] = 0;
    stack[++top] = n - 1;

    while (top >= 0) {
        int right = stack[top--];
        int left = stack[top--];
        int q;

        if (left >= right)
            continue;

        choose_pivot(list, left, right, method);
        q = partition(list, left, right);

        /* 큰 구간을 먼저 넣고 작은 구간을 나중에 넣어, 작은 구간부터 처리 (스택 깊이 감소) */
        if (q - left > right - q) {
            stack[++top] = left;   stack[++top] = q - 1;
            stack[++top] = q + 1;  stack[++top] = right;
        } else {
            stack[++top] = q + 1;  stack[++top] = right;
            stack[++top] = left;   stack[++top] = q - 1;
        }
    }

    free(stack);
}

/* 오름차순 정렬 확인 (비교 횟수에 포함하지 않음) */
static int is_sorted(int list[], int n)
{
    int i;
    for (i = 1; i < n; i++)
        if (list[i - 1] > list[i])
            return 0;
    return 1;
}

/* 무작위 정수 데이터 생성 (0 ~ 99999, 중복 허용) */
static void make_data(int list[], int n)
{
    int i;
    for (i = 0; i < n; i++)
        list[i] = big_rand() % 100000;
}

/* 현재 시각 (ms 단위, 나노초 해상도) */
static double now_ms(void)
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

static void run_pivot_experiment(void)
{
    int sizes[] = { 1000, 2000, 4000, 8000, 16000, 20000 };
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    const char *names[] = { "Quick Sort (First)   ", "Quick Sort (Random)  ", "Quick Sort (Median-3)" };
    int s, m;

    for (s = 0; s < num_sizes; s++) {
        int n = sizes[s];
        int *original = (int *)malloc(sizeof(int) * n);
        int *copy = (int *)malloc(sizeof(int) * n);

        if (original == NULL || copy == NULL) {
            fprintf(stderr, "메모리 할당 실패\n");
            exit(1);
        }

        make_data(original, n);

        printf("Data Size : %d\n\n", n);
        for (m = PIVOT_FIRST; m <= PIVOT_MEDIAN; m++) {
            memcpy(copy, original, sizeof(int) * n);  /* 세 방법 모두 같은 원본의 복사본 사용 */
            cmp_count = 0;
            quick_sort_recursive(copy, 0, n - 1, m);

            if (!is_sorted(copy, n)) {
                fprintf(stderr, "정렬 오류: %s, n = %d\n", names[m], n);
                exit(1);
            }
            printf("%s : %lld comparisons\n", names[m], cmp_count);
        }
        printf("\n");

        free(original);
        free(copy);
    }
}

/* [보너스] 재귀 vs 반복 실행시간 비교 */
static void run_bonus_experiment(void)
{
    const int n = 10000;
    const int repeat = 100;  /* 실행시간이 매우 짧아서 100번 측정한 평균 사용 */
    int *original = (int *)malloc(sizeof(int) * n);
    int *copy = (int *)malloc(sizeof(int) * n);
    double total_rec = 0.0, total_itr = 0.0, start;
    int r;

    if (original == NULL || copy == NULL) {
        fprintf(stderr, "메모리 할당 실패\n");
        exit(1);
    }

    make_data(original, n);

    for (r = 0; r < repeat; r++) {
        /* 재귀: 배열 복사는 측정 구간 밖에서 */
        memcpy(copy, original, sizeof(int) * n);
        start = now_ms();
        quick_sort_recursive(copy, 0, n - 1, PIVOT_MEDIAN);
        total_rec += now_ms() - start;
        if (!is_sorted(copy, n)) {
            fprintf(stderr, "정렬 오류: Recursive\n");
            exit(1);
        }

        /* 반복 */
        memcpy(copy, original, sizeof(int) * n);
        start = now_ms();
        quick_sort_iterative(copy, n, PIVOT_MEDIAN);
        total_itr += now_ms() - start;
        if (!is_sorted(copy, n)) {
            fprintf(stderr, "정렬 오류: Iterative\n");
            exit(1);
        }
    }

    printf("Data Size : %d\n", n);
    printf("Pivot     : Median-of-Three\n\n");
    printf("Recursive Quick Sort : %.4f ms\n", total_rec / repeat);
    printf("Iterative Quick Sort : %.4f ms\n", total_itr / repeat);
    printf("(%d회 측정 평균)\n", repeat);

    free(original);
    free(copy);
}

int main(void)
{
    srand((unsigned int)time(NULL));

    run_pivot_experiment();

    printf("========================================\n");
    printf("[Bonus] Recursive vs Iterative\n");
    printf("========================================\n\n");
    run_bonus_experiment();

    return 0;
}