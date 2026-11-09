int heap_size = 0;

void insert_value(int value) {

    int i = ++heap_size;

    while(i != 1 && value > heap[i/2]) {
        heap[i] = heap[i/2];
        i = i/2;

    }
    heap[i] = value;
    
}


int delete_value() {

    int max = heap[1];
    int temp = heap[heap_size--];
    int parent = 1;
    int child = 2;

    while(child <= heap_size) {

        if(child<heap_size && heap[child] <= heap[child+1])
            child++;
        if(temp >= heap[child])
            break;

        heap[parent] = heap[child];
        parent = child;
        child = child*2;
    }
    heap[parent] = temp;
    return max;





}