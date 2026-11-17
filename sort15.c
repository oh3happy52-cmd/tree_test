/*
 * 과제 15: 퀵 정렬과 AVL 트리 정렬의 성능 비교
 *  - 퀵 정렬(Median-of-Three)로 배열을 정렬한 뒤, 정렬된 배열에서 삽입/삭제
 *  - 같은 데이터를 AVL 트리에 삽입해 정렬 상태를 유지한 뒤, AVL 트리에서 삽입/삭제
 *  - 측정: 총 수행시간, 데이터 값 비교 횟수, 데이터 이동 횟수, 최대 메모리 사용량
 *
 * 측정 기준
 *  - 비교 횟수: 데이터 값끼리 비교할 때만 셈 (LESS, GREATER, LESS_EQUAL, EQUAL 함수)
 *              AVL의 높이/균형 인수 비교, 인덱스 비교, NULL 검사는 제외
 *  - 이동 횟수: 데이터 값을 복사/대입할 때만 셈
 *              (swap 1번 = 3회, 피벗 값 저장 = 1회, 배열 칸 이동 = 1회, AVL 노드에 값 저장 = 1회)
 *              AVL의 포인터 연결 변경(회전), 중복 횟수(count) 변경은 제외
 *  - 메모리: 자료구조 크기 + 최대 스택 사용량
 *           배열 = 정렬/삽입/삭제 중 최대 원소 개수 x sizeof(int)
 *           AVL  = 정렬/삽입/삭제 중 최대 노드 개수 x sizeof(AVLNode)
 *           스택 = 함수를 부르기 직전 스택 주소와 가장 깊이 내려간 스택 주소의 차이
 *  - 시간: 정렬 시간 + 삽입/삭제 시간 (난수 생성, 배열 복사, 결과 확인, 출력, 트리 해제는 제외)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#define REPEAT 10            /* 시간은 같은 과정을 10번 반복한 평균 */
#define VALUE_RANGE 100000   /* 데이터 값 범위: 0 ~ 99999 */

typedef struct AVLNode {
    int key;                 /* 데이터 값 */
    int count;               /* 같은 값의 개수 (중복 허용) */
    int height;              /* 노드 높이 */
    struct AVLNode *left;
    struct AVLNode *right;
} AVLNode;

typedef struct {
    int is_insert;           /* 1이면 삽입, 0이면 삭제 */
    int value;               /* 삽입 또는 삭제할 값 */
} Operation;

typedef struct {
    double time_ms;
    long long comparisons;
    long long movements;
    long long memory;
    int final_count;
    /* README 분석용 세부 정보 */
    long long struct_bytes;  /* 자료구조 크기 */
    long long stack_bytes;   /* 최대 스택 사용량 */
    long long max_units;     /* 배열: 최대 원소 수, AVL: 최대 노드 수 */
    long long rotations;     /* AVL 회전 횟수 */
    int tree_height;         /* 마지막 AVL 트리 높이 */
    long long sort_cmp, sort_move;   /* 정렬 단계만의 비교/이동 */
    int inserted, deleted, missed;   /* 삽입 수, 삭제 성공 수, 삭제 실패 수 */
} Result;

/* ---------------- 측정용 전역 변수 ---------------- */
static long long cmp_count = 0;
static long long move_count = 0;
static long long rotation_count = 0;

static int probe_on = 0;
static uintptr_t stack_base = 0;
static uintptr_t stack_min = 0;

static long long cur_units = 0;   /* 현재 배열 원소 수 또는 AVL 노드 수 */
static long long max_units = 0;

#define PROBE()                                         \
    do {                                                \
        if (probe_on) {                                 \
            volatile char marker = 0;                   \
            uintptr_t addr = (uintptr_t)&marker;        \
            if (addr < stack_min) stack_min = addr;     \
        }                                               \
    } while (0)

static void units_changed(long long diff)
{
    cur_units += diff;
    if (cur_units > max_units)
        max_units = cur_units;
}

/* ---------------- 공통 함수 ---------------- */
static int big_rand(void)
{
    return (int)(((unsigned int)(rand() & 0x7FFF) << 15) | (unsigned int)(rand() & 0x7FFF));
}

/* 데이터 값 비교: 호출될 때마다 비교 횟수 1 증가 */
static int LESS(int a, int b)       { cmp_count++; return a < b; }
static int GREATER(int a, int b)    { cmp_count++; return a > b; }
static int LESS_EQUAL(int a, int b) { cmp_count++; return a <= b; }
static int EQUAL(int a, int b)      { cmp_count++; return a == b; }

/* 두 값 교환: 데이터 대입 3회 */
static void swap(int *a, int *b)
{
    int t = *a;
    *a = *b;
    *b = t;
    move_count += 3;
}

static double now_ms(void)
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

/* ================= 1. 배열 + 퀵 정렬 ================= */

/* 첫 번째, 가운데, 마지막 원소 중 중앙값의 인덱스 (과제 12~14와 동일) */
static int median_of_three(int list[], int left, int right)
{
    int mid = (left + right) / 2;

    PROBE();

    if (LESS(list[left], list[mid])) {
        if (LESS(list[mid], list[right]))
            return mid;
        else if (LESS(list[left], list[right]))
            return right;
        else
            return left;
    } else {
        if (LESS(list[left], list[right]))
            return left;
        else if (LESS(list[mid], list[right]))
            return right;
        else
            return mid;
    }
}

static int partition(int list[], int left, int right)
{
    int pivot, low = left, high = right + 1;

    PROBE();

    pivot = list[left];      /* 피벗 값 저장: 이동 1회 */
    move_count++;

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

static void quick_sort(int list[], int left, int right)
{
    PROBE();

    if (left < right) {
        int p = median_of_three(list, left, right);
        int q;
        if (p != left)
            swap(&list[left], &list[p]);
        q = partition(list, left, right);
        quick_sort(list, left, q - 1);
        quick_sort(list, q + 1, right);
    }
}

/* 정렬된 배열에 x 삽입: 앞에서부터 선형 탐색 후 뒤의 데이터를 한 칸씩 뒤로 이동 */
static void array_insert(int list[], int *count, int x)
{
    int i = 0, j;

    PROBE();

    /* x보다 큰 값이 처음 나오는 위치 찾기 (같은 값 뒤에 삽입) */
    while (i < *count && LESS_EQUAL(list[i], x))
        i++;

    /* i번 칸부터 끝까지 한 칸씩 뒤로 이동 (뒤에서부터) */
    for (j = *count; j > i; j--) {
        list[j] = list[j - 1];
        move_count++;
    }

    list[i] = x;              /* 새 값 저장 */
    move_count++;
    (*count)++;
    units_changed(1);
}

/* 정렬된 배열에서 x 하나 삭제: 선형 탐색 후 뒤의 데이터를 한 칸씩 앞으로 이동 */
static int array_delete(int list[], int *count, int x)
{
    int i = 0, j;

    PROBE();

    /* x보다 작은 값은 건너뜀 (정렬되어 있으므로 x보다 큰 값이 나오면 x는 없음) */
    while (i < *count && LESS(list[i], x))
        i++;

    if (i >= *count || !EQUAL(list[i], x))
        return 0;             /* 삭제할 값 없음 */

    for (j = i; j < *count - 1; j++) {
        list[j] = list[j + 1];
        move_count++;
    }

    (*count)--;
    units_changed(-1);
    return 1;
}

/* ================= 2. AVL 트리 ================= */

static int height(AVLNode *node)
{
    return node ? node->height : 0;
}

static void update_height(AVLNode *node)
{
    int hl = height(node->left), hr = height(node->right);
    node->height = (hl > hr ? hl : hr) + 1;
}

static int balance_factor(AVLNode *node)
{
    return height(node->left) - height(node->right);
}

/* 오른쪽 회전 (포인터 연결만 변경, 데이터 이동 없음) */
static AVLNode *rotate_right(AVLNode *y)
{
    AVLNode *x = y->left;
    PROBE();
    y->left = x->right;
    x->right = y;
    update_height(y);
    update_height(x);
    rotation_count++;
    return x;
}

/* 왼쪽 회전 */
static AVLNode *rotate_left(AVLNode *x)
{
    AVLNode *y = x->right;
    PROBE();
    x->right = y->left;
    y->left = x;
    update_height(x);
    update_height(y);
    rotation_count++;
    return y;
}

/* 높이를 갱신하고, 균형이 깨졌으면 회전으로 복구 (LL, LR, RR, RL) */
static AVLNode *rebalance(AVLNode *node)
{
    int bf;

    update_height(node);
    bf = balance_factor(node);

    if (bf > 1) {                                  /* 왼쪽이 높음 */
        if (balance_factor(node->left) < 0)        /* LR */
            node->left = rotate_left(node->left);
        return rotate_right(node);                 /* LL */
    }
    if (bf < -1) {                                 /* 오른쪽이 높음 */
        if (balance_factor(node->right) > 0)       /* RL */
            node->right = rotate_right(node->right);
        return rotate_left(node);                  /* RR */
    }
    return node;
}

static AVLNode *new_node(int key)
{
    AVLNode *node = (AVLNode *)malloc(sizeof(AVLNode));
    if (node == NULL) {
        fprintf(stderr, "메모리 할당 실패\n");
        exit(1);
    }
    node->key = key;          /* 데이터 값 저장: 이동 1회 */
    move_count++;
    node->count = 1;
    node->height = 1;
    node->left = node->right = NULL;
    units_changed(1);
    return node;
}

/* AVL 삽입: 같은 값이 있으면 count만 1 증가 */
static AVLNode *avl_insert(AVLNode *node, int key)
{
    PROBE();

    if (node == NULL)
        return new_node(key);

    if (LESS(key, node->key))
        node->left = avl_insert(node->left, key);
    else if (GREATER(key, node->key))
        node->right = avl_insert(node->right, key);
    else {
        node->count++;        /* 중복 값: 노드를 새로 만들지 않음 */
        return node;
    }

    return rebalance(node);
}

/* 오른쪽 서브트리의 가장 작은 노드를 떼어냄 (왼쪽 포인터만 따라가므로 값 비교 없음) */
static AVLNode *remove_min(AVLNode *node)
{
    PROBE();

    if (node->left == NULL) {
        AVLNode *right = node->right;
        free(node);
        units_changed(-1);
        return right;
    }
    node->left = remove_min(node->left);
    return rebalance(node);
}

/* AVL 삭제: 값 하나만 제거. 삭제했으면 *deleted = 1 */
static AVLNode *avl_delete(AVLNode *node, int key, int *deleted)
{
    PROBE();

    if (node == NULL)
        return NULL;          /* 삭제할 값 없음 */

    if (LESS(key, node->key))
        node->left = avl_delete(node->left, key, deleted);
    else if (GREATER(key, node->key))
        node->right = avl_delete(node->right, key, deleted);
    else {
        *deleted = 1;

        if (node->count > 1) {          /* 중복 값: count만 1 감소 */
            node->count--;
            return node;
        }

        if (node->left == NULL || node->right == NULL) {   /* 자식이 0개 또는 1개 */
            AVLNode *child = node->left ? node->left : node->right;
            free(node);
            units_changed(-1);
            return child;
        }

        /* 자식이 2개: 오른쪽 서브트리의 최솟값(후계자)을 이 노드로 복사 */
        {
            AVLNode *succ = node->right;
            while (succ->left != NULL)
                succ = succ->left;
            node->key = succ->key;      /* 데이터 값 복사: 이동 1회 */
            move_count++;
            node->count = succ->count;
            node->right = remove_min(node->right);
        }
    }

    return rebalance(node);
}

/* 중위 순회로 오름차순 결과를 배열에 저장 (결과 확인용, 측정에서 제외) */
static void inorder(AVLNode *node, int out[], int *idx)
{
    int c;
    if (node == NULL)
        return;
    inorder(node->left, out, idx);
    for (c = 0; c < node->count; c++)
        out[(*idx)++] = node->key;
    inorder(node->right, out, idx);
}

static void free_tree(AVLNode *node)
{
    if (node == NULL)
        return;
    free_tree(node->left);
    free_tree(node->right);
    free(node);
}

/* ================= 실험 ================= */

static void reset_counters(void)
{
    cmp_count = 0;
    move_count = 0;
    rotation_count = 0;
    cur_units = 0;
    max_units = 0;
}

/* 배열 + 퀵 정렬 과정 1번 실행. final_out에 최종 배열을 복사 */
static double run_array(const int original[], int n, const Operation ops[], int num_ops,
                        Result *res, int *final_out, int *sorted_out)
{
    volatile char base_marker = 0;
    int *list = (int *)malloc(sizeof(int) * (n + num_ops));  /* 삽입에 대비해 여유 공간 확보 */
    int count = n, i;
    double t0, t1, t2, t3;

    if (list == NULL) {
        fprintf(stderr, "메모리 할당 실패\n");
        exit(1);
    }
    memcpy(list, original, sizeof(int) * n);   /* 복사는 측정에서 제외 */

    reset_counters();
    cur_units = n;
    max_units = n;
    stack_base = (uintptr_t)&base_marker;
    stack_min = stack_base;
    res->inserted = res->deleted = res->missed = 0;

    /* 1) 정렬 */
    t0 = now_ms();
    quick_sort(list, 0, n - 1);
    t1 = now_ms();

    res->sort_cmp = cmp_count;
    res->sort_move = move_count;

    /* 정렬 확인 (측정 제외) */
    for (i = 1; i < n; i++) {
        if (list[i - 1] > list[i]) {
            fprintf(stderr, "퀵 정렬 오류 (n = %d)\n", n);
            exit(1);
        }
    }
    if (sorted_out != NULL)
        memcpy(sorted_out, list, sizeof(int) * n);

    /* 2) 삽입/삭제 */
    t2 = now_ms();
    for (i = 0; i < num_ops; i++) {
        if (ops[i].is_insert) {
            array_insert(list, &count, ops[i].value);
            res->inserted++;
        } else if (array_delete(list, &count, ops[i].value)) {
            res->deleted++;
        } else {
            res->missed++;
        }
    }
    t3 = now_ms();

    res->time_ms = (t1 - t0) + (t3 - t2);
    res->comparisons = cmp_count;
    res->movements = move_count;
    res->final_count = count;
    res->max_units = max_units;
    res->struct_bytes = max_units * (long long)sizeof(int);
    res->stack_bytes = (long long)(stack_base - stack_min);
    res->memory = res->struct_bytes + res->stack_bytes;
    res->rotations = 0;
    res->tree_height = 0;

    if (final_out != NULL)
        memcpy(final_out, list, sizeof(int) * count);

    free(list);
    return res->time_ms;
}

/* AVL 트리 과정 1번 실행. 정렬 결과와 최종 결과를 배열 결과와 비교 */
static double run_avl(const int original[], int n, const Operation ops[], int num_ops,
                      Result *res, const int *sorted_ref, const int *final_ref, int final_ref_count)
{
    volatile char base_marker = 0;
    AVLNode *root = NULL;
    int total = 0, i, idx;
    double t0, t1, t2, t3;
    int *check = (int *)malloc(sizeof(int) * (n + num_ops));

    if (check == NULL) {
        fprintf(stderr, "메모리 할당 실패\n");
        exit(1);
    }

    reset_counters();
    stack_base = (uintptr_t)&base_marker;
    stack_min = stack_base;
    res->inserted = res->deleted = res->missed = 0;

    /* 1) 정렬: 모든 데이터를 AVL 트리에 삽입 */
    t0 = now_ms();
    for (i = 0; i < n; i++)
        root = avl_insert(root, original[i]);
    t1 = now_ms();
    total = n;

    res->sort_cmp = cmp_count;
    res->sort_move = move_count;

    /* 중위 순회로 오름차순 확인 (측정 제외) */
    if (sorted_ref != NULL) {
        idx = 0;
        inorder(root, check, &idx);
        if (idx != n || memcmp(check, sorted_ref, sizeof(int) * n) != 0) {
            fprintf(stderr, "AVL 정렬 결과가 퀵 정렬 결과와 다름 (n = %d)\n", n);
            exit(1);
        }
    }

    /* 2) 삽입/삭제 */
    t2 = now_ms();
    for (i = 0; i < num_ops; i++) {
        if (ops[i].is_insert) {
            root = avl_insert(root, ops[i].value);
            total++;
            res->inserted++;
        } else {
            int deleted = 0;
            root = avl_delete(root, ops[i].value, &deleted);
            if (deleted) {
                total--;
                res->deleted++;
            } else {
                res->missed++;
            }
        }
    }
    t3 = now_ms();

    res->time_ms = (t1 - t0) + (t3 - t2);
    res->comparisons = cmp_count;
    res->movements = move_count;
    res->final_count = total;
    res->max_units = max_units;
    res->struct_bytes = max_units * (long long)sizeof(AVLNode);
    res->stack_bytes = (long long)(stack_base - stack_min);
    res->memory = res->struct_bytes + res->stack_bytes;
    res->rotations = rotation_count;
    res->tree_height = height(root);

    /* 최종 데이터가 배열과 같은지 확인 (측정 제외) */
    if (final_ref != NULL) {
        idx = 0;
        inorder(root, check, &idx);
        if (idx != final_ref_count || memcmp(check, final_ref, sizeof(int) * idx) != 0) {
            fprintf(stderr, "삽입/삭제 후 AVL 트리와 배열의 데이터가 다름 (n = %d)\n", n);
            exit(1);
        }
    }

    free(check);
    free_tree(root);     /* 트리 해제는 측정에서 제외 */
    return res->time_ms;
}

int main(void)
{
    int sizes[] = { 1000, 2000, 4000, 8000, 16000, 20000 };
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    Result qr_log[6], ar_log[6];
    int s, i, r;

    srand((unsigned int)time(NULL));

    for (s = 0; s < num_sizes; s++) {
        int n = sizes[s];
        int num_ops = n / 5;                       /* n의 20% */
        int *original = (int *)malloc(sizeof(int) * n);
        Operation *ops = (Operation *)malloc(sizeof(Operation) * num_ops);
        int *sorted_ref = (int *)malloc(sizeof(int) * n);
        int *final_ref = (int *)malloc(sizeof(int) * (n + num_ops));
        Result qr, ar, tmp;
        double q_total = 0.0, a_total = 0.0;
        char buf1[64], buf2[64];

        if (original == NULL || ops == NULL || sorted_ref == NULL || final_ref == NULL) {
            fprintf(stderr, "메모리 할당 실패\n");
            exit(1);
        }

        /* 데이터와 연산 순서를 미리 만들어 두고 두 자료구조에 똑같이 적용 (난수 생성 시간 제외) */
        for (i = 0; i < n; i++)
            original[i] = big_rand() % VALUE_RANGE;
        for (i = 0; i < num_ops; i++) {
            ops[i].is_insert = (big_rand() % 2 == 0);       /* 짝수면 삽입, 홀수면 삭제 */
            ops[i].value = big_rand() % VALUE_RANGE;        /* 실제 값은 새 난수 */
        }

        /* 1) 비교/이동/메모리 측정 + 결과 확인 (스택 기록 켬) */
        probe_on = 1;
        run_array(original, n, ops, num_ops, &qr, final_ref, sorted_ref);
        run_avl(original, n, ops, num_ops, &ar, sorted_ref, final_ref, qr.final_count);
        probe_on = 0;

        /* 2) 시간 측정: 같은 과정을 REPEAT번 반복한 평균 (스택 기록 끔) */
        for (r = 0; r < REPEAT; r++) {
            q_total += run_array(original, n, ops, num_ops, &tmp, NULL, NULL);
            a_total += run_avl(original, n, ops, num_ops, &tmp, NULL, NULL, 0);
        }
        qr.time_ms = q_total / REPEAT;
        ar.time_ms = a_total / REPEAT;

        qr_log[s] = qr;
        ar_log[s] = ar;

        printf("Data Size : %d\n", n);
        printf("Operations: %d\n\n", num_ops);
        printf("                  Quick Sort      AVL Tree\n");
        printf("------------------------------------------------\n");
        sprintf(buf1, "%.4f ms", qr.time_ms);
        sprintf(buf2, "%.4f ms", ar.time_ms);
        printf("%-18s%-16s%s\n", "Total Time", buf1, buf2);
        printf("%-18s%-16lld%lld\n", "Comparisons", qr.comparisons, ar.comparisons);
        printf("%-18s%-16lld%lld\n", "Data Movements", qr.movements, ar.movements);
        sprintf(buf1, "%lld Bytes", qr.memory);
        sprintf(buf2, "%lld Bytes", ar.memory);
        printf("%-18s%-16s%s\n", "Memory Usage", buf1, buf2);
        printf("\n");
        printf("%-18s%-16d%d\n", "Final Data Count", qr.final_count, ar.final_count);
        printf("\n");

        free(original);
        free(ops);
        free(sorted_ref);
        free(final_ref);
    }

    /* README 분석용 세부 내역 */
    printf("==============================================================================\n");
    printf("[Detail]\n");
    printf("==============================================================================\n");
    printf("Size   Struct  SortCmp    SortMove   MaxUnits  StructB   StackB  Rotations Height\n");
    printf("------------------------------------------------------------------------------\n");
    for (s = 0; s < num_sizes; s++) {
        Result *q = &qr_log[s], *a = &ar_log[s];
        printf("%-6d %-7s %-10lld %-10lld %-9lld %-9lld %-7lld %-9s %s\n",
               sizes[s], "Array", q->sort_cmp, q->sort_move, q->max_units,
               q->struct_bytes, q->stack_bytes, "-", "-");
        printf("%-6d %-7s %-10lld %-10lld %-9lld %-9lld %-7lld %-9lld %d\n",
               sizes[s], "AVL", a->sort_cmp, a->sort_move, a->max_units,
               a->struct_bytes, a->stack_bytes, a->rotations, a->tree_height);
    }
    printf("\nOperations (Insert / Delete OK / Delete Miss)\n");
    for (s = 0; s < num_sizes; s++)
        printf("%-6d %d / %d / %d\n", sizes[s], qr_log[s].inserted, qr_log[s].deleted, qr_log[s].missed);

    printf("\nsizeof(int) = %d, sizeof(AVLNode) = %d\n", (int)sizeof(int), (int)sizeof(AVLNode));
    printf("(Time: %d회 평균, 나머지: 첫 번째 실행에서 측정)\n", REPEAT);
    printf("(정렬 결과와 삽입/삭제 후 최종 데이터가 두 자료구조에서 모두 같음을 확인함)\n");

    return 0;
}