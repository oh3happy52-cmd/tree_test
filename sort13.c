/*
 * 과제 13: 병합 정렬과 퀵 정렬의 성능 비교
 *  - 병합 정렬 1가지, 퀵 정렬 3가지(First / Random / Median-of-Three)
 *  - 측정 항목: 숫자 비교 횟수, 실행시간(ms), 최대 추가 메모리 사용량(Byte)
 *
 * 메모리 측정 방법
 *  - 힙 메모리: 정렬 중 malloc으로 할당한 크기를 직접 더함 (병합 정렬의 임시 배열)
 *  - 스택 메모리: 정렬 함수를 부르기 직전의 스택 주소와,
 *                정렬 중 가장 깊이 내려간 스택 주소의 차이를 측정
 *                (Mac/Linux/Windows 모두 스택은 높은 주소 -> 낮은 주소로 자람)
 *  - 최대 추가 메모리 = 힙 메모리 + 최대 스택 사용량
 *  - 원본 데이터 배열은 공통 비용이므로 제외
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#define PIVOT_FIRST  0
#define PIVOT_RANDOM 1
#define PIVOT_MEDIAN 2

#define ALG_MERGE        0
#define ALG_QUICK_FIRST  1
#define ALG_QUICK_RANDOM 2
#define ALG_QUICK_MEDIAN 3
#define NUM_ALG          4

#define REPEAT 10   /* 실행시간은 10번 정렬한 평균 사용 */

/* ---------------- 측정용 전역 변수 ---------------- */
static long long cmp_count = 0;      /* 데이터 값 비교 횟수 */

static int probe_on = 0;             /* 1일 때만 스택 주소를 기록 */
static uintptr_t stack_base = 0;     /* 정렬 함수 호출 직전의 스택 주소 */
static uintptr_t stack_min = 0;      /* 정렬 중 가장 낮은(깊은) 스택 주소 */
static long long heap_bytes = 0;     /* 정렬 중 추가로 할당한 힙 메모리 */
static int cur_depth = 0;            /* 현재 재귀 깊이 */
static int max_depth = 0;            /* 최대 재귀 깊이 */

/* 현재 함수의 스택 위치를 기록 */
#define PROBE()                                         \
    do {                                                \
        if (probe_on) {                                 \
            volatile char marker = 0;                   \
            uintptr_t addr = (uintptr_t)&marker;        \
            if (addr < stack_min) stack_min = addr;     \
        }                                               \
    } while (0)

/* ---------------- 공통 함수 ---------------- */
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

static int LESS_EQUAL(int a, int b)
{
    cmp_count++;
    return a <= b;
}

/* ---------------- 병합 정렬 ---------------- */
/* list[left..mid]와 list[mid+1..right]를 병합 (sorted는 임시 배열) */
static void merge(int list[], int sorted[], int left, int mid, int right)
{
    int i = left, j = mid + 1, k = left, l;

    PROBE();

    /* 두 부분의 맨 앞 값끼리 비교해서 작은 값을 임시 배열에 넣음 */
    while (i <= mid && j <= right) {
        if (LESS_EQUAL(list[i], list[j]))   /* 같으면 왼쪽을 먼저 넣어 안정 정렬 유지 */
            sorted[k++] = list[i++];
        else
            sorted[k++] = list[j++];
    }

    /* 남은 값은 비교 없이 그대로 복사 */
    while (i <= mid)
        sorted[k++] = list[i++];
    while (j <= right)
        sorted[k++] = list[j++];

    /* 임시 배열의 결과를 원래 배열로 다시 복사 */
    for (l = left; l <= right; l++)
        list[l] = sorted[l];
}

static void merge_sort_rec(int list[], int sorted[], int left, int right)
{
    PROBE();
    cur_depth++;
    if (cur_depth > max_depth)
        max_depth = cur_depth;

    if (left < right) {
        int mid = (left + right) / 2;
        merge_sort_rec(list, sorted, left, mid);        /* 왼쪽 절반 정렬 */
        merge_sort_rec(list, sorted, mid + 1, right);   /* 오른쪽 절반 정렬 */
        merge(list, sorted, left, mid, right);          /* 두 부분 병합 */
    }

    cur_depth--;
}

static void merge_sort(int list[], int n)
{
    int *sorted = (int *)malloc(sizeof(int) * n);   /* 임시 배열 (추가 메모리) */
    if (sorted == NULL) {
        fprintf(stderr, "메모리 할당 실패\n");
        exit(1);
    }
    heap_bytes += (long long)sizeof(int) * n;

    merge_sort_rec(list, sorted, 0, n - 1);

    free(sorted);
}

/* ---------------- 퀵 정렬 (과제 12와 동일) ---------------- */
static int median_of_three(int list[], int left, int right)
{
    int mid = (left + right) / 2;

    PROBE();

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

static int partition(int list[], int left, int right)
{
    int pivot = list[left];
    int low = left;
    int high = right + 1;

    PROBE();

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

static void quick_sort_rec(int list[], int left, int right, int method)
{
    PROBE();
    cur_depth++;
    if (cur_depth > max_depth)
        max_depth = cur_depth;

    if (left < right) {
        int q;
        choose_pivot(list, left, right, method);
        q = partition(list, left, right);
        quick_sort_rec(list, left, q - 1, method);
        quick_sort_rec(list, q + 1, right, method);
    }

    cur_depth--;
}

/* ---------------- 실험 ---------------- */
static int is_sorted(int list[], int n)
{
    int i;
    for (i = 1; i < n; i++)
        if (list[i - 1] > list[i])
            return 0;
    return 1;
}

static void make_data(int list[], int n)
{
    int i;
    for (i = 0; i < n; i++)
        list[i] = big_rand() % 100000;
}

static double now_ms(void)
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

static void run_sort(int alg, int list[], int n)
{
    switch (alg) {
    case ALG_MERGE:        merge_sort(list, n); break;
    case ALG_QUICK_FIRST:  quick_sort_rec(list, 0, n - 1, PIVOT_FIRST); break;
    case ALG_QUICK_RANDOM: quick_sort_rec(list, 0, n - 1, PIVOT_RANDOM); break;
    case ALG_QUICK_MEDIAN: quick_sort_rec(list, 0, n - 1, PIVOT_MEDIAN); break;
    }
}

/* 비교 횟수와 메모리 측정 (스택 기록 켜고 1번 정렬) */
static void measure_count_memory(int alg, int list[], int n,
                                 long long *cmp, long long *heap, long long *stack, int *depth)
{
    volatile char base_marker = 0;

    cmp_count = 0;
    heap_bytes = 0;
    cur_depth = 0;
    max_depth = 0;
    stack_base = (uintptr_t)&base_marker;   /* 정렬 함수를 부르기 직전의 스택 위치 */
    stack_min = stack_base;
    probe_on = 1;

    run_sort(alg, list, n);

    probe_on = 0;
    *cmp = cmp_count;
    *heap = heap_bytes;
    *stack = (long long)(stack_base - stack_min);
    *depth = max_depth;
}

int main(void)
{
    int sizes[] = { 1000, 2000, 4000, 8000, 16000, 20000 };
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    const char *names[NUM_ALG] = { "Merge Sort", "Quick (First)", "Quick (Random)", "Quick (Median3)" };

    /* 메모리 세부 내역 (마지막에 따로 출력) */
    long long heap_log[6][NUM_ALG], stack_log[6][NUM_ALG];
    int depth_log[6][NUM_ALG];
    int s, a, r;

    srand((unsigned int)time(NULL));

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
        printf("Algorithm       Comparisons    Time(ms)    Memory(Bytes)\n");
        printf("--------------------------------------------------------\n");

        for (a = 0; a < NUM_ALG; a++) {
            long long cmp, heap, stack;
            int depth;
            double total = 0.0, start;

            /* 1) 비교 횟수, 메모리 측정 */
            memcpy(copy, original, sizeof(int) * n);   /* 모든 알고리즘이 같은 원본의 복사본 사용 */
            measure_count_memory(a, copy, n, &cmp, &heap, &stack, &depth);
            if (!is_sorted(copy, n)) {
                fprintf(stderr, "정렬 오류: %s, n = %d\n", names[a], n);
                exit(1);
            }

            /* 2) 실행시간 측정 (스택 기록 끄고 REPEAT번 평균, 복사 시간은 제외) */
            for (r = 0; r < REPEAT; r++) {
                memcpy(copy, original, sizeof(int) * n);
                start = now_ms();
                run_sort(a, copy, n);
                total += now_ms() - start;
                if (!is_sorted(copy, n)) {
                    fprintf(stderr, "정렬 오류: %s, n = %d\n", names[a], n);
                    exit(1);
                }
            }

            heap_log[s][a] = heap;
            stack_log[s][a] = stack;
            depth_log[s][a] = depth;

            printf("%-15s %-14lld %-11.4f %lld\n", names[a], cmp, total / REPEAT, heap + stack);
        }
        printf("\n");

        free(original);
        free(copy);
    }

    /* 메모리 세부 내역: README 분석용 */
    printf("========================================================\n");
    printf("[Memory Detail] Memory = Heap + Stack\n");
    printf("========================================================\n");
    printf("Size   Algorithm        Heap(B)   Stack(B)  MaxDepth\n");
    printf("--------------------------------------------------------\n");
    for (s = 0; s < num_sizes; s++) {
        for (a = 0; a < NUM_ALG; a++) {
            printf("%-6d %-16s %-9lld %-9lld %d\n",
                   sizes[s], names[a], heap_log[s][a], stack_log[s][a], depth_log[s][a]);
        }
    }
    printf("\n(Time: %d회 평균, Comparisons/Memory: 첫 번째 정렬에서 측정)\n", REPEAT);

    return 0;
}